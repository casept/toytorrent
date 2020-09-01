#ifndef TOYTORRENT_PEER_H_
#define TOYTORRENT_PEER_H_
#include <cstdint>
#include <array>
#include <string>

constexpr std::size_t peer_id_length {20};
typedef std::array<char, peer_id_length> PeerID;

class Peer {
    public:
    std::string m_ip;
    std::uint32_t m_port;
    PeerID m_id;

    Peer() = delete;
    Peer(PeerID id, std::string const &ip, std::uint32_t port);
};
#endif