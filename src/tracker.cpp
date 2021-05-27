#include "tracker.h"

#include <cpr/cpr.h>
#include <fmt/core.h>

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "bencode_parser.h"
#include "peer.h"

TrackerCommunicator::TrackerCommunicator(std::string announce_url, std::uint32_t our_port, PeerID our_peer_id,
                                         std::string trunc_infohash) {
    m_data_downloaded = {0};
    m_data_uploaded = {0};
    m_data_left = {0};
    m_announce_url = {announce_url};
    m_peer_id = {our_peer_id};
    m_port = {our_port};
    m_info_hash = {trunc_infohash};
}

BEncodeObject get_object_from_dict_or_throw(std::string key, std::map<std::string, BEncodeObject> dict,
                                            std::string throw_msg) {
    auto dict_entry = dict.find(key);
    if (dict_entry == dict.end()) {
        // Key doesn't exist
        throw std::runtime_error(throw_msg);
    }
    auto obj = dict_entry->second;
    return obj;
}

std::vector<Peer> bencode_peer_list_to_peers(std::vector<BEncodeObject> list) {
    std::vector<Peer> peers{};
    for (BEncodeObject const& peer_entry : list) {
        if (!peer_entry.dict.has_value()) {
            auto err = "Tracker violated protocol: peer entry is not a bencoded dictionary";
            throw std::runtime_error(err);
        }
        auto peer_dict = peer_entry.dict.value();
        auto peer_id_obj = get_object_from_dict_or_throw("peer id", peer_dict,
                                                         "Tracker violated protocol: all peers must have peer ID");
        auto peer_ip_obj =
            get_object_from_dict_or_throw("ip", peer_dict, "Tracker violated protocol: all peers must have IP address");
        auto peer_port_obj =
            get_object_from_dict_or_throw("port", peer_dict, "Tracker violated protocol: all peers must have port");
        std::uint32_t peer_port = static_cast<std::int64_t>(peer_port_obj.integer.value());
        PeerID peer_id;
        std::copy_n(peer_id_obj.str.value().data(), peer_id_length, peer_id.begin());
        peers.push_back(Peer(peer_id, peer_ip_obj.str.value(), peer_port));
    }
    return peers;
}

// TODO: Quite a bit of error handling is missing
std::tuple<std::vector<Peer>, std::int64_t> TrackerCommunicator::send_to_tracker(const std::string& event) {
    // Perform the request
    // TODO: Implement support for compact representation, because many trackers mandate it
    cpr::Parameters p = cpr::Parameters{{"info_hash", m_info_hash},
                                        {"peer_id", std::string(m_peer_id.data())},
                                        {"port", std::to_string(m_port)},
                                        {"uploaded", std::to_string(m_data_uploaded)},
                                        {"downloaded", std::to_string(m_data_downloaded)},
                                        {"left", std::to_string(m_data_left)},
                                        {"event", event},
                                        {"compact", "0"}};
    cpr::Response r = cpr::Get(cpr::Url{m_announce_url}, p);
    auto queue = std::deque<char>(r.text.begin(), r.text.end());
    auto parser = BEncodeParser(queue);
    auto resp_object_maybe_none = parser.next();

    // Sanity check
    if (!resp_object_maybe_none.has_value()) {
        auto err = std::string("Tracker response must be a bencoded dictionary");
        throw std::runtime_error(err);
    }
    auto resp_object = resp_object_maybe_none.value();
    if (resp_object.type != BEncodeObjectType::Dict) {
        auto err = std::string("Tracker response must be a bencoded dictionary");
        throw std::runtime_error(err);
    }

    // Check whether the tracker returned a failure
    auto resp_dict = resp_object.dict.value();
    auto failure_reason = resp_dict.find("failure reason");
    if (failure_reason != resp_dict.end()) {
        // The key exists, meaning we have a reason
        auto err = fmt::format("Tracker indicated failure: {}", failure_reason->second.str.value());
        throw std::runtime_error(err);
    }

    // Check when we're supposed to contact the tracker next
    auto interval_mapret = resp_dict.find("interval");
    if (interval_mapret == resp_dict.end()) {
        // The key doesn't exist, meaning the tracker violated the protocol
        auto err = std::string("Tracker violated protocol: expected a key 'interval' in response, but it was absent");
        throw std::runtime_error(err);
    }
    auto interval = interval_mapret->second.integer.value();

    // Finally, parse out the peers
    auto peers_mapret = resp_dict.find("peers");
    if (peers_mapret == resp_dict.end()) {
        // The key doesn't exist, meaning the tracker violated the protocol
        auto err = std::string("Tracker violated protocol: expected a key 'peers' in response, but it was absent");
        throw std::runtime_error(err);
    }
    auto peers_list = peers_mapret->second.list.value();
    auto peers = bencode_peer_list_to_peers(peers_list);

    return std::tuple<std::vector<Peer>, std::int64_t>(peers, interval);
}

std::tuple<std::vector<Peer>, std::int64_t> TrackerCommunicator::send_completed() {
    return send_to_tracker("completed");
}

std::tuple<std::vector<Peer>, std::int64_t> TrackerCommunicator::send_started() { return send_to_tracker("started"); }

std::tuple<std::vector<Peer>, std::int64_t> TrackerCommunicator::send_stopped() { return send_to_tracker("stopped"); }

std::tuple<std::vector<Peer>, std::int64_t> TrackerCommunicator::send_update() { return send_to_tracker("empty"); }