#include "peer.h"

Peer::Peer(PeerID id, std::string const &ip, std::uint32_t port) {
    m_id = id;
    m_ip = ip;
    m_port = port;
}