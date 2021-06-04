#include "peer.hpp"

#include <bits/stdint-uintn.h>
#include <botan-2/botan/auto_rng.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "log.hpp"
#include "smolsocket.hpp"

namespace tt::peer {

const std::vector<std::uint8_t> Peer_Handshake_Magic = {19,  'B', 'i', 't', 'T', 'o', 'r', 'r', 'e', 'n',
                                                        't', ' ', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l'};

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

std::vector<std::uint8_t> ID::as_byte_vec() const {
    auto char_to_bytes = [](char c) { return static_cast<uint8_t>(c); };
    std::vector<std::uint8_t> result{};
    result.resize(this->m_id.size());
    std::transform(this->m_id.begin(), this->m_id.end(), result.begin(), char_to_bytes);
    return result;
};

Conn::Conn(smolsocket::Sock sock) : m_sock(sock) {}

Peer::Peer(const ID& id, std::string const& ip, const std::uint16_t port) : m_ip(ip), m_port(port), m_id(id) {}

void Peer::handshake(const std::vector<std::uint8_t>& truncated_infohash, const ID& our_id) {
    // Create connection
    try {
        log::log(log::Level::Debug, log::Subsystem::Peer, fmt::format("Peer::connect(): Trying {}", *this));
        this->m_conn = {{smolsocket::Sock(this->m_ip, this->m_port, smolsocket::Proto::TCP, 2000)}};
    } catch (const smolsocket::Exception& e) {
        auto msg = fmt::format("Peer::connect(): Failed to connect: {}", e.what());
        log::log(log::Level::Warning, log::Subsystem::Peer, msg);
        throw Exception(msg);
    }

    // Handshake
    auto& sock = this->m_conn.value().m_sock;
    try {
        // Fixed handshake bytes
        sock.send(Peer_Handshake_Magic, 2000);
        // Supported protocol extensions
        sock.send({0, 0, 0, 0, 0, 0, 0, 0}, 2000);
        // Infohash
        sock.send(truncated_infohash, 2000);
        // Our peer ID
        sock.send(our_id.as_byte_vec(), 2000);
        // We're technically supposed to verify the ID of the other peer here,
        // but that's impossible because compact format tracker messages contain no ID.
        // Therefore, just accept anything here.

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
