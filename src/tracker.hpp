#pragma once

#include <array>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "bencode.hpp"
#include "peer.hpp"
namespace tt::tracker {
// Thrown on tracker-related failures.
class Exception : public std::exception {
   public:
    std::string m_msg{};
    Exception(const std::string_view&);
    const char* what() const throw();
};

// This class abstracts away communication with a particular tracker.
class TrackerCommunicator {
   private:
    std::string m_announce_url;
    std::vector<std::uint8_t> m_info_hash;
    std::string m_peer_id;
    std::uint64_t m_data_downloaded;
    std::uint64_t m_data_uploaded;
    std::uint64_t m_data_left;
    std::uint32_t m_port;
    // Send the given event to the tracker with the data in this class
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<peer::Peer>, std::int64_t> send_to_tracker(const std::string& event);

   public:
    // Create a TrackerCommunicator to talk with the given tracker.
    TrackerCommunicator(const std::string_view& announce_url, const std::uint16_t our_port, const peer::ID& our_peer_id,
                        const std::vector<std::uint8_t>& trunc_infohash_binary, const std::size_t data_left);
    // Update our statistics about uploaded and downloaded data
    // which are sent to the tracker.
    void set_data_downloaded(std::uint64_t num_bytes);
    void set_data_uploaded(std::uint64_t num_bytes);
    void set_data_left(std::uint64_t num_bytes);

    // Tell the tracker that our download is finished.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<peer::Peer>, std::int64_t> send_completed();

    // Tell the tracker that we've just started our download.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<peer::Peer>, std::int64_t> send_started();

    // Tell the tracker that we've stopped downloading.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<peer::Peer>, std::int64_t> send_stopped();

    // Tell the tracker that we just want more peers.
    // Returns a vector of peers the tracker gave us if successful.
    // Causes an exception if not.
    std::tuple<std::vector<peer::Peer>, std::int64_t> send_update();
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
    bencode::Object peers;
};
}  // namespace tt::tracker
