#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

// An active connection to a peer.
class Conn {
   public:
    bool m_we_choked = false;
    bool m_we_interested = false;
    smolsocket::Sock m_sock;
    Conn(smolsocket::Sock m_sock);
};

class Peer {
   private:
    std::optional<Conn> m_conn{};

   public:
    std::string m_ip;
    std::uint16_t m_port;
    ID m_id;

    Peer(const ID& id, std::string const& ip, const std::uint16_t port);
    // Establish a connection to this peer.
    void handshake(const std::vector<std::uint8_t>& truncated_infohash, const ID& our_id);
    // Check whether a connection is established.
    bool is_connected() const;
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