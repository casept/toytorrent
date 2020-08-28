#ifndef TOYTORRENT_TRACKER_H_
#define TOYTORRENT_TRACKER_H_

#include "bencode_parser.h"

#include <string>
#include <array>
#include <cstdint>

constexpr std::size_t peer_id_length {20};

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
    void send_to_tracker(const std::string &event);
    
    public:
    TrackerCommunicator() = delete;
    // Create a TrackerCommunicator to talk with the given tracker.
    explicit TrackerCommunicator(std::string announce_url, std::uint32_t our_port, std::array<char, peer_id_length> our_peer_id, std::string infohash);
    // Update our statistics about uploaded and downloaded data
    // which are sent to the tracker.
    void set_data_downloaded(std::uint64_t num_bytes);
    void set_data_uploaded(std::uint64_t num_bytes);
    void set_data_left(std::uint64_t num_bytes);
    
    // Tell the tracker that our download is finished.
    void send_completed();
    // Tell the tracker that we've just started our download.
    void send_started();
    // Tell the tracker that we've stopped downloading.
    void send_stopped();
    // Tell the tracker that we just want more peers.
    void send_update();
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
#endif