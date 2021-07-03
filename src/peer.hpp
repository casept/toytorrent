#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "peer_message.hpp"
#include "smolsocket.hpp"

namespace tt::peer {
// Thrown on peer-related failures.
class Exception : public std::exception {
   public:
    std::string m_msg{};
    Exception(const std::string_view&);
    [[nodiscard]] const char* what() const noexcept;
};

const std::size_t ID_Length = 20;

// A per-torrent peer identifier
class ID {
   public:
    // Generate a new ID at random.
    ID();
    // Generate a new ID based on given 20-byte (plus \0-terminator) string.
    ID(const std::array<char, ID_Length>& str);
    // Generate a new ID based on given string.
    ID(const std::string_view& str);
    // Equal if the internal ID field is the same.
    inline bool operator==(const ID& rhs) { return this->m_id == rhs.m_id; };
    inline bool operator!=(const ID& rhs) { return this->m_id != rhs.m_id; };
    // Get string representation.
    std::string as_string() const;
    // Get byte representation.
    std::vector<std::uint8_t> as_byte_vec() const;

   private:
    std::string m_id;
};

class Peer {
   private:
    std::optional<smolsocket::Sock> m_sock;
    bool m_we_choked = false;
    bool m_we_interested = false;

   public:
    std::string m_ip;
    std::uint16_t m_port;
    ID m_id;

    Peer(const ID& id, const std::string_view& ip, const std::uint16_t port);
    Peer() = delete;
    // Moving will invalidate the old peer.
    Peer(Peer&& src);
    // The underlying socket should only ever be touched by one instance.
    Peer(const Peer&) = delete;
    Peer& operator=(const Peer&) = delete;
    Peer& operator=(Peer&&) = delete;
    // Establish a connection to this peer.
    void handshake(const std::vector<std::uint8_t>& truncated_infohash, const ID& our_id);
    // Send a message to this peer (after handshaking).
    void send_message(const peer::IMessage& msg);
    /*
     * Send a keepalive to this peer (after handshaking).
     * The protocol requires this to happen at least once every 2 minutes.
     */
    void send_keepalive();
    // Block until this peer has sent us a message.
    std::unique_ptr<IMessage> wait_for_message();
};
}  // namespace tt::peer

namespace fmt {
template <>
struct formatter<tt::peer::Peer> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const tt::peer::Peer& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{{ID: \"{}\", IP: \"{}\", Port: \"{}\"}}", p.m_id.as_string(), p.m_ip.c_str(),
                         p.m_port);
    }
};
}  // namespace fmt
