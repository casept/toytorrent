#include "peer.hpp"

#include <botan-2/botan/auto_rng.h>

#include <algorithm>
#include <cstdint>
#include <string>

namespace tt {
PeerID::PeerID() {
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

PeerID::PeerID(const std::array<char, Peer_ID_Length>& str) { std::copy(str.begin(), str.end(), this->m_id.begin()); }

std::string PeerID::as_string() const { return std::string(this->m_id.data()); }

Peer::Peer(const PeerID& id, std::string const& ip, const std::uint32_t port) {
    m_id = id;
    m_ip = ip;
    m_port = port;
}
}  // namespace tt