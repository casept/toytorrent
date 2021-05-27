#pragma once

#include <array>
#include <cstdint>
#include <string>

constexpr std::size_t peer_id_length{21};
typedef std::array<char, peer_id_length> PeerID;

class Peer {
   public:
    std::string m_ip;
    std::uint32_t m_port;
    PeerID m_id;

    Peer() = delete;
    Peer(PeerID id, std::string const &ip, std::uint32_t port);
};
