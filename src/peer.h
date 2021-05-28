#pragma once

#include <array>
#include <cstdint>
#include <string>

constexpr std::size_t peer_id_length{21};

class PeerID {
   public:
    // Generate a new ID at random.
    PeerID();
    // Generate a new ID based on given 20-byte (plus \0-terminator) string.
    PeerID(const std::array<char, peer_id_length> str);
    // Get string representation.
    std::string as_string() const;

   private:
    std::array<char, peer_id_length> m_id;
};

class Peer {
   public:
    std::string m_ip;
    std::uint32_t m_port;
    PeerID m_id;

    Peer() = delete;
    Peer(const PeerID id, std::string const &ip, const std::uint32_t port);
};
