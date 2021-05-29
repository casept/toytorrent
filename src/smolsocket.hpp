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

#include <cstdint>
#include <exception>
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
    Exception(const std::string_view& msg);
    const char* what() const throw();
};

// A socket.
class Sock {
   private:
    int m_sockfd;

   public:
    AddrKind m_addr_kind;
    Proto m_proto;

    /*
     * Takes an ip:port or domain:port string and a protocol,
     * resolves the address and opens a socket.
     *
     * For now, only connect()-ing to socket is supported.
     */
    Sock(const std::string_view& addr, const Proto proto);
    /*
     * Send the given data, retrying until it gets through.
     * TODO: Timeout
     */
    void send(const std::vector<uint8_t>& data);

    ~Sock();
};

}  // namespace smolsocket
