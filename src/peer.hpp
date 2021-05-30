#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "smolsocket.hpp"

namespace tt::peer {
constexpr std::size_t ID_Length{21};

// A per-torrent peer identifier
class ID {
   public:
    // Generate a new ID at random.
    ID();
    // Generate a new ID based on given 20-byte (plus \0-terminator) string.
    ID(const std::array<char, ID_Length>& str);
    // Equal if the internal ID field is the same.
    inline bool operator==(const ID& rhs) { return this->m_id == rhs.m_id; };
    // Get string representation.
    std::string as_string() const;

   private:
    std::array<char, ID_Length> m_id;
};

// An active connection to a peer.
class Conn {
   public:
    bool m_we_choked = false;
    bool m_we_interested = false;
    smolsocket::Sock m_sock;
    Conn(smolsocket::Sock m_sock);
};

class Peer {
   private:
    std::optional<Conn> m_conn{};

   public:
    std::string m_ip;
    std::uint32_t m_port;
    ID m_id;

    Peer(const ID& id, std::string const& ip, const std::uint32_t port);
    // Establish a connection to this peer.
    void connect(const std::string_view& truncated_infohash);
};
}  // namespace tt::peer
