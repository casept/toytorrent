#include "smolsocket.hpp"

extern "C" {
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

namespace smolsocket {

Exception::Exception(const std::string_view& msg, const std::optional<int> errno_val, const std::optional<int> gai_err)
    : m_msg(msg) {
    if (errno_val.has_value()) {
        this->m_msg.append(strerror(errno_val.value()));
    }
    if (gai_err.has_value()) {
        this->m_msg.append(gai_strerror(gai_err.value()));
    }
}

const char* Exception::what() const throw() { return this->m_msg.c_str(); }

Sock::Sock(const std::string_view& addr, const uint32_t port, const Proto proto) {
    this->m_proto = proto;

    // Give hints
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    switch (proto) {
        case Proto::TCP:
            hints.ai_socktype = SOCK_STREAM;
            break;
        case Proto::UDP:
            hints.ai_socktype = SOCK_DGRAM;
            break;
        default:
            std::cerr << "smolsocket::Sock::Sock(): Unknown protocol. This is a bug." << std::endl;
            std::exit(EXIT_FAILURE);
            break;
    }

    // Lookup
    int err;
    struct addrinfo* res;
    err = getaddrinfo(addr.data(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        throw Exception("smolsocket::Sock::Sock(): getaddrinfo() failed: ", {}, {err});
    }

    // Only return 1st addr for now
    // TODO: Deal with >1
    if (res == NULL) {
        throw Exception("smolsocket::Sock::Sock(): Lookup result contains no IP addresses", {}, {});
    }
    switch (res->ai_family) {
        case AF_INET:
            this->m_addr_kind = AddrKind::V4;
            break;
        case AF_INET6:
            this->m_addr_kind = AddrKind::V6;
            break;
        default:
            freeaddrinfo(res);
            throw Exception("smolsocket::Sock::Sock(): Unknown address family", {}, {});
            break;
    }

    // Actually open the socket
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        freeaddrinfo(res);
        throw Exception("smolsocket::Sock::Sock(): Failed to create socket: ", {errno}, {});
    }
    this->m_sockfd = sock;

    // And connect
    // TODO: Also support bind()-ing
    err = connect(sock, res->ai_addr, res->ai_addrlen);
    if (err == -1) {
        freeaddrinfo(res);
        throw Exception("smolsocket::Sock::Sock(): Failed to connect() socket: ", {errno}, {});
    }
    freeaddrinfo(res);
}

void Sock::send(const std::vector<uint8_t>& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const int ret = ::send(this->m_sockfd, data.data() + sent, data.size() - sent, 0);
        if (ret == -1) {
            throw Exception("smolsocket::Sock::send(): Failed to send(): ", {errno}, {});
        }
        sent += (size_t)ret;
    }
}

Sock::~Sock() { close(this->m_sockfd); }

}  // namespace smolsocket
