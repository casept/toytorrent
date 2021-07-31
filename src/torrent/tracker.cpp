#include "tracker.hpp"

#include <cpr/cpr.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "../log.hpp"
#include "../reusable/byteorder.hpp"
#include "../torrent/bencode.hpp"
#include "../torrent/peer.hpp"

namespace tt::tracker {
Exception::Exception(const std::string_view& msg) : m_msg(msg) {}

const char* Exception::what() const noexcept { return this->m_msg.c_str(); }

static bencode::Object get_object_from_dict_or_throw(std::string key, std::map<std::string, bencode::Object> dict,
                                                     std::string throw_msg) {
    auto dict_entry = dict.find(key);
    if (dict_entry == dict.end()) {
        // Key doesn't exist
        throw Exception(throw_msg);
    }
    auto obj = dict_entry->second;
    return obj;
}

static std::vector<peer::Peer> bencode_peer_list_to_peers(std::vector<bencode::Object> list) {
    std::vector<peer::Peer> peers{};
    peers.reserve(list.size());
    for (bencode::Object const& peer_entry : list) {
        if (!peer_entry.dict.has_value()) {
            throw Exception("Tracker violated protocol: peer entry is not a bencoded dictionary");
        }
        auto peer_dict = peer_entry.dict.value();
        auto peer_id_obj = get_object_from_dict_or_throw("peer id", peer_dict,
                                                         "Tracker violated protocol: all peers must have peer ID");
        auto peer_ip_obj =
            get_object_from_dict_or_throw("ip", peer_dict, "Tracker violated protocol: all peers must have IP address");
        const std::string_view peer_ip_view = std::string_view(peer_ip_obj.str.value());
        auto peer_port_obj =
            get_object_from_dict_or_throw("port", peer_dict, "Tracker violated protocol: all peers must have port");
        const std::uint16_t peer_port = static_cast<std::uint16_t>(peer_port_obj.integer.value());
        const peer::ID peer_id{peer_id_obj.str.value()};
        peers.emplace_back(peer_id, peer_ip_view, peer_port);
    }
    return peers;
}

static std::vector<peer::Peer> bep52_peer_str_to_peers(const std::string_view& str) {
    std::vector<peer::Peer> peers{};
    size_t i = 0;
    // v4 addresses are coded as 4 bytes for address, 2 bytes for port
    if (str.size() % 6 != 0) {
        throw Exception("Tracker violated protocol: Compact peer representation must be multiple of 6 bytes long");
    }
    while (i < str.size()) {
        // Extract from string
        std::array<uint8_t, smolsocket::V6_Len_Bytes> ip{};
        const std::string_view ip_substr = str.substr(i, 4);
        std::transform(ip_substr.begin(), ip_substr.end(), ip.begin(), [](char x) { return static_cast<uint8_t>(x); });
        std::array<char, 2> port;
        std::copy_n(str.substr(i + 4, 2).begin(), 2, port.begin());
        // Convert to usable repr
        std::string ip_str = smolsocket::ip_to_str(ip, smolsocket::AddrKind::V4);
        uint16_t port_native_endian = bo::ntoh(port);
        // Compact format has no ID, generate at random
        peers.emplace_back(peer::ID(), ip_str, port_native_endian);
        i += 6;
    }
    return peers;
}

// Convert request kind to string
static std::string req_kind_to_str(const RequestKind r) {
    std::string event_str{};
    switch (r) {
        case RequestKind::STARTED:
            event_str = "started";
            break;
        case RequestKind::COMPLETED:
            event_str = "completed";
            break;
        case RequestKind::STOPPED:
            event_str = "stopped";
            break;
        case RequestKind::UPDATE:
            event_str = "";
            break;
        default:
            break;
    }
    return event_str;
}

static cpr::Response query_tracker(const std::string_view& announce_url, const Request& r) {
    std::string info_hash_str{};
    for (const auto b : r.trunc_infohash_binary) {
        info_hash_str.push_back(static_cast<char>(b));
    }
    auto event_str = req_kind_to_str(r.kind);
    cpr::Parameters p = cpr::Parameters{{"info_hash", info_hash_str},
                                        {"peer_id", r.our_id.as_string()},
                                        {"port", std::to_string(r.our_port)},
                                        {"uploaded", std::to_string(r.stats.bytes_uploaded)},
                                        {"downloaded", std::to_string(r.stats.bytes_downloaded)},
                                        {"left", std::to_string(r.stats.bytes_left)},
                                        {"event", event_str},
                                        // Some trackers (like opentracker) hate when this is set to 0
                                        {"compact", "1"}};
    cpr::Response resp = cpr::Get(cpr::Url{std::string(announce_url)}, p);

    tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker,
                 fmt::format("Tracker request URL: {}", resp.url.str()));
    tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker,
                 fmt::format("Raw tracker response: {}", resp.text));
    if (resp.error.code != cpr::ErrorCode::OK) {
        if (resp.status_code != cpr::status::HTTP_OK) {
            throw Exception(fmt::format("tracker::send_request(): Got status code {} from tracker", resp.status_code));
        } else {
            throw Exception(fmt::format("tracker::send_request(): Got error \"{}\" from curl", resp.error.message));
        }
    }
    return resp;
}

static std::vector<peer::Peer> parse_peers_from_tracker_resp(const std::map<std::string, bencode::Object>& resp_dict) {
    auto peers = resp_dict.find("peers");
    if (peers == resp_dict.end()) {
        // The key doesn't exist, meaning the tracker violated the protocol
        throw Exception(
            "tracker::send_request(): Tracker violated protocol: expected a key 'peers' in response, but it was "
            "absent");
    }
    // Trackers may return BEP52-style compact peer lists unprompted, so we have to always be ready to parse both
    if (peers->second.list.has_value()) {
        // Classic peer list
        auto peers_bencoded = peers->second.list.value();
        return bencode_peer_list_to_peers(peers_bencoded);
    } else if (peers->second.str.has_value()) {
        // Compact peer list
        auto peers_bencoded = peers->second.str.value();
        return bep52_peer_str_to_peers(peers_bencoded);
    } else {
        // Garbage
        throw Exception("tracker::send_request(): Tracker violated protocol: peers weren't list or string");
    }
}

static std::int64_t parse_checkin_interval_from_tracker_resp(const std::map<std::string, bencode::Object>& resp_dict) {
    // Check when we're supposed to contact the tracker next
    auto interval_mapret = resp_dict.find("interval");
    if (interval_mapret == resp_dict.end()) {
        // The key doesn't exist, meaning the tracker violated the protocol
        throw Exception(
            "tracker::send_request(): Tracker violated protocol: expected a key 'interval' in response, but it was "
            "absent");
    }
    std::int64_t interval = interval_mapret->second.integer.value();
    tt::log::log(tt::log::Level::Debug, tt::log::Subsystem::Tracker,
                 fmt::format("tracker::send_request(): Tracker told us to check in again in {} seconds\n", interval));
    return interval;
}

std::tuple<std::vector<peer::Peer>, std::int64_t> send_request(const std::string_view& announce_url, const Request& r) {
    const auto resp = query_tracker(announce_url, r);
    auto queue = std::deque<char>(resp.text.begin(), resp.text.end());
    auto parser = bencode::Parser(queue);
    auto resp_object_maybe_none = parser.next();

    // Sanity check
    if (!resp_object_maybe_none.has_value()) {
        throw Exception("tracker::send_request(): Tracker violated protocol: response must be a bencoded dictionary");
    }
    auto resp_object = resp_object_maybe_none.value();
    if (resp_object.type != bencode::ObjectType::Dict) {
        throw Exception("tracker::send_request(): Tracker violated protcol: response must be a bencoded dictionary");
    }

    // Check whether the tracker returned a failure
    auto resp_dict = resp_object.dict.value();
    auto failure_reason = resp_dict.find("failure reason");
    if (failure_reason != resp_dict.end()) {
        // The key exists, meaning we have a reason
        throw Exception(fmt::format("tracker::send_request(): Tracker indicated failure with reason: {}",
                                    failure_reason->second.str.value()));
    }

    auto interval = parse_checkin_interval_from_tracker_resp(resp_dict);
    auto peers_vec = parse_peers_from_tracker_resp(resp_dict);
    return std::tuple<std::vector<peer::Peer>, std::int64_t>(std::move(peers_vec), interval);
}
}  // namespace tt::tracker
