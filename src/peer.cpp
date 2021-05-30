#include "peer.hpp"

#include <botan-2/botan/auto_rng.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "log.hpp"
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

Conn::Conn(smolsocket::Sock sock) : m_sock(sock) {}

Peer::Peer(const ID& id, std::string const& ip, const std::uint32_t port) {
    m_id = id;
    m_ip = ip;
    m_port = port;
}

void Peer::connect(const std::string_view& truncated_infohash) {
    // Create connection
    try {
        const std::string addr = fmt::format("{}:{}", this->m_ip, this->m_port);
        log::log(log::Level::Debug, log::Subsystem::Peer, fmt::format("Connecting to peer {}", addr));
        this->m_conn = {{smolsocket::Sock(addr, smolsocket::Proto::TCP)}};
    } catch (const smolsocket::Exception& e) {
        log::log(log::Level::Fatal, log::Subsystem::Peer, fmt::format("Failed to connect: {}", e.what()));
        exit(EXIT_FAILURE);
    }

    // Handshake
    auto& sock = this->m_conn.value().m_sock;
    auto char_to_bytes = [](char c) { return (uint8_t)c; };
    std::string bt_proto = "19BitTorrent protocol";
    std::vector<uint8_t> bt_proto_bytes{};
    std::transform(bt_proto.begin(), bt_proto.end(), std::back_inserter(bt_proto_bytes), char_to_bytes);
    try {
        sock.send(bt_proto_bytes);
        sock.send({0, 0, 0, 0, 0, 0, 0, 0});  // No extensions
        std::vector<uint8_t> hash_bytes{};
        std::transform(truncated_infohash.begin(), truncated_infohash.end(), std::back_inserter(hash_bytes),
                       char_to_bytes);
        sock.send(hash_bytes);
    } catch (const smolsocket::Exception& e) {
        log::log(log::Level::Warning, log::Subsystem::Peer, fmt::format("Failed to handshake: {}", e.what()));
    }
}
}  // namespace tt::peer
