#include "tracker.hpp"

#include <cpr/cpr.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "bencode.hpp"
#include "log.hpp"
#include "peer.hpp"

namespace tt::tracker {
Exception::Exception(const std::string_view& msg) : m_msg(msg) {}

const char* Exception::what() const throw() { return this->m_msg.c_str(); }

TrackerCommunicator::TrackerCommunicator(const std::string_view& announce_url, const std::uint16_t our_port,
                                         const peer::ID& our_peer_id,
                                         const std::vector<std::uint8_t>& trunc_infohash_binary,
                                         const std::size_t data_left)
    : m_data_downloaded(0),
      m_data_uploaded(0),
      m_data_left(data_left),
      m_announce_url(announce_url),
      m_peer_id(our_peer_id.as_string()),
      m_port(our_port) {
    m_info_hash = {};
    m_info_hash.resize(trunc_infohash_binary.size());
    std::copy(trunc_infohash_binary.begin(), trunc_infohash_binary.end(), m_info_hash.begin());
}

bencode::Object get_object_from_dict_or_throw(std::string key, std::map<std::string, bencode::Object> dict,
                                              std::string throw_msg) {
    auto dict_entry = dict.find(key);
    if (dict_entry == dict.end()) {
        // Key doesn't exist
        throw Exception(throw_msg);
    }
    auto obj = dict_entry->second;
    return obj;
}

std::vector<peer::Peer> bencode_peer_list_to_peers(std::vector<bencode::Object> list) {
    std::vector<peer::Peer> peers{};
    for (bencode::Object const& peer_entry : list) {
        if (!peer_entry.dict.has_value()) {
            throw Exception("Tracker violated protocol: peer entry is not a bencoded dictionary");
        }
        auto peer_dict = peer_entry.dict.value();
        auto peer_id_obj = get_object_from_dict_or_throw("peer id", peer_dict,
                                                         "Tracker violated protocol: all peers must have peer ID");
        auto peer_ip_obj =
            get_object_from_dict_or_throw("ip", peer_dict, "Tracker violated protocol: all peers must have IP address");
        auto peer_port_obj =
            get_object_from_dict_or_throw("port", peer_dict, "Tracker violated protocol: all peers must have port");
        std::uint32_t peer_port = static_cast<std::int64_t>(peer_port_obj.integer.value());
        peer::ID peer_id;
        std::copy_n(peer_id_obj.str.value().data(), peer::ID_Length, peer_id.as_string().begin());
        peers.push_back(peer::Peer(peer_id, peer_ip_obj.str.value(), peer_port));
    }
    return peers;
}

std::vector<peer::Peer> bep52_peer_str_to_peers(const std::string_view& str) {
    std::vector<peer::Peer> peers{};
    size_t i = 0;
    // v4 addresses are coded as 4 bytes for address, 2 bytes for port
    if (str.size() % 6 != 0) {
        throw Exception("Tracker violated protocol: Compact peer representation must be multiple of 6 bytes long");
    }
    while (i < str.size()) {
        // Extract from string
        std::array<uint8_t, smolsocket::util::V6_Len_Bytes> ip{};
        std::transform(str.substr(i, i + 4).begin(), str.substr(i, i + 4).end(), ip.begin(),
                       [](char x) { return static_cast<uint8_t>(x); });
        std::array<char, 2> port;
        std::copy_n(str.substr(i + 4, i + 6).begin(), 2, port.begin());
        // Convert to usable repr
        std::string ip_str = smolsocket::util::ip_to_str(ip, smolsocket::AddrKind::V4);
        uint16_t port_native_endian = smolsocket::util::ntoh(port);
        // Compact format has no ID, generate at random
        peers.push_back(peer::Peer(peer::ID(), ip_str, port_native_endian));
        i += 6;
    }
    return peers;
}

/*
 * Send a peer request to the tracker.
 * Returns peer list and interval in seconds until next checkin.
 * TODO: Support BEP7
 */
std::tuple<std::vector<peer::Peer>, std::int64_t> TrackerCommunicator::send_to_tracker(const std::string& event) {
    // Perform the request
    // TODO: Implement support for compact representation, because many trackers mandate it
    std::string info_hash_str{};
    for (const auto b : this->m_info_hash) {
        info_hash_str.push_back(static_cast<char>(b));
    }
    cpr::Parameters p = cpr::Parameters{{"info_hash", info_hash_str},
                                        {"peer_id", std::string(m_peer_id.data())},
                                        {"port", std::to_string(m_port)},
                                        {"uploaded", std::to_string(m_data_uploaded)},
                                        {"downloaded", std::to_string(m_data_downloaded)},
                                        {"left", std::to_string(m_data_left)},
                                        {"event", event},
                                        // Some trackers (like opentracker) hate when this is set to 0
                                        {"compact", "1"}};
    cpr::Response r = cpr::Get(cpr::Url{m_announce_url}, p);
    tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker,
                 fmt::format("Tracker request URL: {}", r.url.str()));
    tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker, fmt::format("Raw tracker response: {}", r.text));

    auto queue = std::deque<char>(r.text.begin(), r.text.end());
    auto parser = bencode::Parser(queue);
    auto resp_object_maybe_none = parser.next();

    // Sanity check
    if (!resp_object_maybe_none.has_value()) {
        throw Exception("Tracker violated protocol: response must be a bencoded dictionary");
    }
    auto resp_object = resp_object_maybe_none.value();
    if (resp_object.type != bencode::ObjectType::Dict) {
        throw Exception("Tracker violated protcol: response must be a bencoded dictionary");
    }

    // Check whether the tracker returned a failure
    auto resp_dict = resp_object.dict.value();
    auto failure_reason = resp_dict.find("failure reason");
    if (failure_reason != resp_dict.end()) {
        // The key exists, meaning we have a reason
        throw Exception(fmt::format("Tracker indicated failure with reason: {}", failure_reason->second.str.value()));
    }

    // Check when we're supposed to contact the tracker next
    auto interval_mapret = resp_dict.find("interval");
    if (interval_mapret == resp_dict.end()) {
        // The key doesn't exist, meaning the tracker violated the protocol
        throw Exception("Tracker violated protocol: expected a key 'interval' in response, but it was absent");
    }
    auto interval = interval_mapret->second.integer.value();
    tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker,
                 fmt::format("Tracker told us to check in again in {} seconds\n", interval));

    // Finally, parse out the peers
    auto peers = resp_dict.find("peers");
    if (peers == resp_dict.end()) {
        // The key doesn't exist, meaning the tracker violated the protocol
        throw Exception("Tracker violated protocol: expected a key 'peers' in response, but it was absent");
    }
    // Trackers may return BEP52-style compact peer lists unprompted, so we have to always be ready to parse both
    std::vector<peer::Peer> peers_vec{};
    if (peers->second.list.has_value()) {
        // Classic peer list
        auto peers_bencoded = peers->second.list.value();
        peers_vec = bencode_peer_list_to_peers(peers_bencoded);
    } else if (peers->second.str.has_value()) {
        // Compact peer list
        auto peers_bencoded = peers->second.str.value();
        peers_vec = bep52_peer_str_to_peers(peers_bencoded);
    } else {
        // Garbage
        throw Exception("Tracker violated protocol: peers weren't list or string");
    }
    for (const auto& peer : peers_vec) {
        tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker,
                     fmt::format("Got peer from tracker: {}\n", peer));
    }
    return std::tuple<std::vector<peer::Peer>, std::int64_t>(peers_vec, interval);
}

std::tuple<std::vector<peer::Peer>, std::int64_t> TrackerCommunicator::send_completed() {
    log::log(log::Level::Debug, log::Subsystem::Tracker,
             fmt::format("Sending completion notice to tracker {}", this->m_announce_url));
    return send_to_tracker("completed");
}

std::tuple<std::vector<peer::Peer>, std::int64_t> TrackerCommunicator::send_started() {
    log::log(log::Level::Debug, log::Subsystem::Tracker,
             fmt::format("Sending start notice to tracker {}", this->m_announce_url));
    return send_to_tracker("started");
}

std::tuple<std::vector<peer::Peer>, std::int64_t> TrackerCommunicator::send_stopped() {
    log::log(log::Level::Debug, log::Subsystem::Tracker,
             fmt::format("Sending stop notice to tracker {}", this->m_announce_url));
    return send_to_tracker("stopped");
}

std::tuple<std::vector<peer::Peer>, std::int64_t> TrackerCommunicator::send_update() {
    log::log(log::Level::Debug, log::Subsystem::Tracker,
             fmt::format("Sending update request to tracker {}", this->m_announce_url));
    return send_to_tracker("");
}
}  // namespace tt::tracker
