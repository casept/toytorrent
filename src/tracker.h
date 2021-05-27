#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "bencode_parser.h"
#include "peer.h"

// This class abstracts away communication with a particular tracker.
class TrackerCommunicator {
   private:
    std::string m_announce_url;
    std::string m_info_hash;
    std::array<char, peer_id_length> m_peer_id;
    std::uint64_t m_data_downloaded;
    std::uint64_t m_data_uploaded;
    std::uint64_t m_data_left;
    std::uint32_t m_port;
    // Send the given event to the tracker with the data in this class
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<Peer>, std::int64_t> send_to_tracker(const std::string &event);

   public:
    TrackerCommunicator() = delete;
    // Create a TrackerCommunicator to talk with the given tracker.
    explicit TrackerCommunicator(std::string announce_url, std::uint32_t our_port, PeerID our_peer_id,
                                 std::string trunc_infohash);
    // Update our statistics about uploaded and downloaded data
    // which are sent to the tracker.
    void set_data_downloaded(std::uint64_t num_bytes);
    void set_data_uploaded(std::uint64_t num_bytes);
    void set_data_left(std::uint64_t num_bytes);

    // Tell the tracker that our download is finished.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<Peer>, std::int64_t> send_completed();

    // Tell the tracker that we've just started our download.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<Peer>, std::int64_t> send_started();

    // Tell the tracker that we've stopped downloading.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<Peer>, std::int64_t> send_stopped();

    // Tell the tracker that we just want more peers.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<Peer>, std::int64_t> send_update();
};

struct TrackerResponse {
    std::int64_t num_peers_complete;
    // TODO: Figure out what this field is (undocumented?)
    std::int64_t downloaded;
    std::int64_t num_peers_incomplete;
    // How often we should check in
    std::int64_t tracker_request_wait;
    // How often we're allowed to contact the tracker
    std::int64_t minimum_tracker_request_wait;
    // TODO: Represent better
    BEncodeObject peers;
};
