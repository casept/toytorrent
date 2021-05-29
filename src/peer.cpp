#include "peer.hpp"

#include <botan-2/botan/auto_rng.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "smolsocket.hpp"

namespace tt::peer {
ID::ID() {
    // Generate 20 random printable ASCII chars
    this->m_id = {'\0'};
    auto rng = Botan::AutoSeeded_RNG();
    size_t i = 0;
    while (i < this->m_id.size() - 1) {
        const uint8_t byte = rng.next_nonzero_byte();
        if (byte >= 33 && byte <= 126) {
            this->m_id[i] = (char)byte;
            i++;
        }
    }
    this->m_id[20] = '\0';
}

ID::ID(const std::array<char, ID_Length>& str) { std::copy(str.begin(), str.end(), this->m_id.begin()); }

std::string ID::as_string() const { return std::string(this->m_id.data()); }

Conn::Conn(smolsocket::Sock sock) : m_sock(sock){};

Peer::Peer(const ID& id, std::string const& ip, const std::uint32_t port) {
    m_id = id;
    m_ip = ip;
    m_port = port;
}

void Peer::connect() {
    const std::string addr = fmt::format("{}:{}", this->m_ip, this->m_port);
    this->m_conn = {{smolsocket::Sock(addr, smolsocket::Proto::TCP)}};
}
}  // namespace tt::peer
