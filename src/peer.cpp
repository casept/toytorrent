#include "peer.hpp"

#include <botan-2/botan/auto_rng.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "log.hpp"
#include "peer_message.hpp"
#include "shared_constants.hpp"
#include "smolsocket.hpp"

namespace tt::peer {

const std::vector<std::uint8_t> Peer_Handshake_Magic = {19,  'B', 'i', 't', 'T', 'o', 'r', 'r', 'e', 'n',
                                                        't', ' ', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l'};
const std::optional<std::uint64_t> Timeout{2000};

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
ID::ID(const std::string_view& str) { std::copy_n(str.begin(), ID_Length, this->m_id.begin()); }

std::string ID::as_string() const { return std::string(this->m_id.data()); }

std::vector<std::uint8_t> ID::as_byte_vec() const {
    auto char_to_bytes = [](char c) { return static_cast<uint8_t>(c); };
    std::vector<std::uint8_t> result{};
    result.resize(this->m_id.size());
    std::transform(this->m_id.begin(), this->m_id.end(), result.begin(), char_to_bytes);
    return result;
}

Peer::Peer(const ID& id, const std::string_view& ip, const std::uint16_t port)
    : m_sock({}), m_ip(ip), m_port(port), m_id(id) {}

Peer::Peer(Peer&& src)
    : m_sock(std::move(src.m_sock)),
      m_we_choked(src.m_we_choked),
      m_we_interested(src.m_we_interested),
      m_ip(std::move(src.m_ip)),
      m_port(src.m_port),
      m_id(std::move(src.m_id)) {
    // FIXME: We should invalidate the old one's socket,
    // but initialization in this language is so fucked that after 2 hours of trying I can't figure out how to
    // re-initialize a class field without having to copy-construct.
}

Peer& Peer::operator=(Peer&& other) {
    if (this != &other) {
        // Take other's resources
        std::swap(this->m_sock, other.m_sock);
        this->m_we_choked = other.m_we_choked;
        this->m_we_interested = other.m_we_interested;
        this->m_ip = std::move(other.m_ip);
        this->m_port = other.m_port;
        this->m_id = std::move(other.m_id);
        return *this;
    }
    return other;
}

void Peer::handshake(const std::vector<std::uint8_t>& truncated_infohash, const ID& our_id) {
    // Create connection
    try {
        log::log(log::Level::Debug, log::Subsystem::Peer, fmt::format("Peer::connect(): Trying {}", *this));
        this->m_sock.emplace(smolsocket::Sock(m_ip, m_port, smolsocket::Proto::TCP, Timeout));
    } catch (const smolsocket::Exception& e) {
        auto msg = fmt::format("Conn::Conn(): Failed to connect: {}", e.what());
        log::log(log::Level::Warning, log::Subsystem::Peer, msg);
        throw Exception(msg);
    }
    // Handshake
    try {
        // Fixed handshake bytes
        m_sock.value().send(Peer_Handshake_Magic, Timeout);
        // Supported protocol extensions
        m_sock.value().send({0, 0, 0, 0, 0, 0, 0, 0}, Timeout);
        // Infohash
        m_sock.value().send(truncated_infohash, 2000);
        // Our peer ID
        m_sock.value().send(our_id.as_byte_vec(), 2000);
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

void Peer::send_message(const peer::IMessage& msg) {
    try {
        log::log(log::Level::Debug, log::Subsystem::Peer,
                 fmt::format("Peer::send_message(): Sending message {} to {}", *this, msg));
        // Send message type
        this->m_sock.value().send({static_cast<std::uint8_t>(msg.get_type())}, Timeout);
        // Send message payload
        this->m_sock.value().send(msg.serialize(), Timeout);
    } catch (const smolsocket::Exception& e) {
        auto msg = fmt::format("Peer::send_message(): Failed to send message: {}", e.what());
        log::log(log::Level::Warning, log::Subsystem::Peer, msg);
        throw Exception(msg);
    }
}

void Peer::send_keepalive() {
    try {
        // keepalives are empty messages.
        this->m_sock->send({}, Timeout);
    } catch (const smolsocket::Exception& e) {
        auto msg = fmt::format("Peer::send_keepalive(): Failed to send keepalive: {}", e.what());
        log::log(log::Level::Warning, log::Subsystem::Peer, msg);
        throw Exception(msg);
    }
}

std::unique_ptr<IMessage> Peer::wait_for_message() {
    // FIXME: This timeout is wildly inappropriate. It should be decided by the caller.
    return blocking_read_message_from_socket(this->m_sock.value(), {2000}, Request_Subpiece_Size);
}
}  // namespace tt::peer
