#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "bencode.hpp"
#include "peer.hpp"
namespace tt::tracker {
// Thrown on tracker-related failures.
class Exception : public std::exception {
   public:
    std::string m_msg{};
    Exception(const std::string_view&);
    const char* what() const noexcept override;
};

// Statistics the tracker collects from us
struct Stats {
   public:
    std::uint64_t bytes_downloaded;
    std::uint64_t bytes_uploaded;
    std::uint64_t bytes_left;
};

// The types of requests we can send to the tracker.
enum class RequestKind {
    // Tell tracker that we just joined the swarm
    STARTED,
    // Tell tracker we have finished leeching
    COMPLETED,
    // Tell tracker that we've stopped
    STOPPED,
    // Routine update sent on tracker-requested schedule or to replenish peers
    UPDATE
};

// What the tracker needs to know to service a request.
struct Request {
   public:
    // What we want from the tracker
    RequestKind kind;
    // The torrent's 20-byte truncated binary infohash
    std::vector<std::uint8_t> trunc_infohash_binary;
    // Statistics the tracker would like to know from us
    Stats stats;
    // Our ID
    peer::ID our_id;
    // Our IP
    std::string our_ip;
    // Our port
    std::uint16_t our_port;
};

// Sends a request to the tracker.
// See `RequestKind` for semantics of each message type.
// Returns a vector of peers the tracker gave us if successful.
// Causes an exception if not.
std::tuple<std::vector<peer::Peer>, std::int64_t> send_request(const std::string_view& announce_url, const Request& r);
}  // namespace tt::tracker
