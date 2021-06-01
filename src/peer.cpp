#include "peer.hpp"

#include <botan-2/botan/auto_rng.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "log.hpp"
#include "smolsocket.hpp"

namespace tt::peer {
Exception::Exception(const std::string_view& msg) : m_msg(msg) {}

const char* Exception::what() const throw() { return this->m_msg.c_str(); }

ID::ID() {
    // Generate 20 random printable ASCII chars
    this->m_id = std::string("");
    this->m_id.reserve(ID_Length);
    auto rng = Botan::AutoSeeded_RNG();
    while (this->m_id.size() < ID_Length) {
        const uint8_t byte = rng.next_nonzero_byte();
        if (byte >= 33 && byte <= 126) {
            this->m_id.push_back((char)byte);
        }
    }
}

ID::ID(const std::array<char, ID_Length>& str) { std::copy(str.begin(), str.end(), this->m_id.begin()); }

std::string ID::as_string() const { return std::string(this->m_id.data()); }

Conn::Conn(smolsocket::Sock sock) : m_sock(sock) {}

Peer::Peer(const ID& id, std::string const& ip, const std::uint16_t port) : m_ip(ip), m_port(port), m_id(id) {}

void Peer::connect(const std::string_view& truncated_infohash) {
    // Create connection
    try {
        log::log(log::Level::Debug, log::Subsystem::Peer, fmt::format("Peer::connect(): {}", *this));
        this->m_conn = {{smolsocket::Sock(this->m_ip, this->m_port, smolsocket::Proto::TCP)}};
    } catch (const smolsocket::Exception& e) {
        auto msg = fmt::format("Peer::connect(): Failed to connect: {}", e.what());
        log::log(log::Level::Warning, log::Subsystem::Peer, msg);
        throw Exception(msg);
    }

    // Handshake
    auto& sock = this->m_conn.value().m_sock;
    auto char_to_bytes = [](char c) { return static_cast<uint8_t>(c); };
    std::string bt_proto = "19BitTorrent protocol";
    std::vector<uint8_t> bt_proto_bytes{};
    std::transform(bt_proto.begin(), bt_proto.end(), std::back_inserter(bt_proto_bytes), char_to_bytes);
    try {
        sock.send(bt_proto_bytes);
        sock.send({0, 0, 0, 0, 0, 0, 0, 0});  // No protocol extensions (yet)
        std::vector<uint8_t> hash_bytes{};
        std::transform(truncated_infohash.begin(), truncated_infohash.end(), std::back_inserter(hash_bytes),
                       char_to_bytes);
        sock.send(hash_bytes);
    } catch (const smolsocket::Exception& e) {
        auto msg = fmt::format("Peer::connect(): Failed to handshake: {}", e.what());
        log::log(log::Level::Warning, log::Subsystem::Peer, msg);
        throw Exception(msg);
    }
    log::log(log::Level::Debug, log::Subsystem::Peer,
             fmt::format("Peer::connect(): connection established to peer {}", *this));
}

bool Peer::is_connected() const { return this->m_conn.has_value(); }
}  // namespace tt::peer
