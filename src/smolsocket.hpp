/*
 * A lightweight berkeley sockets C++ wrapper, because everyone's either pulling in Boost and the kitchen sink, not
 * maintaining theirs anymore or I'm too incompetent at Googling.
 *
 * This is not designed to support every feature of the API, only the parts *I* consider to be useful (namely a TCP/UDP
 * client/server, commonly-used address notation etc).
 *
 * TODO: Win32 support, shouldn't be hard but I can't be bothered to test it right now.
 * TODO: Better exception messages
 */

#pragma once

#include <bits/stdint-uintn.h>

#include <array>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace smolsocket {

// Kind of IP address.
enum class AddrKind { V4, V6 };

// Kind of IP-based protocol.
enum class Proto { TCP, UDP };

class Exception : public std::exception {
   public:
    std::string m_msg{};
    Exception(const std::string_view& msg, const std::optional<int> errno_val, const std::optional<int> gai_err);
    const char* what() const noexcept;
};

// A socket.
class Sock {
   private:
    std::optional<int> m_sockfd;

   public:
    AddrKind m_addr_kind;
    Proto m_proto;

    /*
     * Takes an IP or domain name string and a protocol,
     * resolves the address and opens a socket.
     *
     * For now, only connect()-ing to socket is supported.
     *
     * Will throw on failure or timeout.
     */
    Sock(const std::string_view& addr, const uint16_t port, const Proto proto,
         std::optional<std::uint64_t> timeout_millis);
    // Moving out replaces the internal socket handle with an invalid one.
    Sock(Sock&& src);
    // Copying is banned, as there should only be a single object managing the socket.
    Sock(const Sock&) = delete;
    Sock& operator=(const Sock&) = delete;
    Sock& operator=(Sock&&) = delete;
    /*
     * Send the given data, retrying until it gets through.
     * If timeout is set, an exception will be raised if the transfer doesn't complete in time.
     */
    void send(const std::vector<uint8_t>& data, std::optional<std::uint64_t> timeout_millis);
    /*
     * Receive the given amount of data, retrying until it gets through.
     * If timeout is set, an exception will be raised if the transfer doesn't complete in time.
     */
    std::vector<std::uint8_t> recv(const std::size_t data_size, const std::optional<std::uint64_t> timeout_millis);

    ~Sock();
};

const size_t V4_Len_Bytes = 4;
const size_t V6_Len_Bytes = 16;

/*
 * Converts numeric repr (in network byte order) of an IP address to a string.
 * For IPv4, everything after first 4 bytes is ignored.
 */
std::string ip_to_str(const std::array<uint8_t, V6_Len_Bytes>& bytes, const AddrKind kind);

}  // namespace smolsocket
