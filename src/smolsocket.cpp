#include "smolsocket.hpp"

extern "C" {
#include <arpa/inet.h>
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

Exception::Exception(const std::string_view& msg) : m_msg(msg) {}

const char* Exception::what() const throw() { return this->m_msg.c_str(); };

Sock::Sock(const std::string_view& addr, const Proto proto) {
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
        default:
            std::cerr << "smolsocket::Sock::Sock(): Unknown protocol. This is a bug." << std::endl;
            std::exit(EXIT_FAILURE);
            break;
    }

    // Lookup
    int err;
    struct addrinfo* res;
    err = getaddrinfo(addr.data(), NULL, &hints, &res);
    if (err != 0) {
        throw Exception("smolsocket::Sock::Sock(): getaddrinfo() failed");
    }

    // Only return 1st addr for now
    // TODO: Deal with >1
    if (res == NULL) {
        throw Exception("smolsocket::Sock::Sock(): Lookup result contains no IP addresses");
    }
    switch (res->ai_family) {
        case AF_INET:
            this->m_addr_kind = AddrKind::V4;
            break;
        case AF_INET6:
            this->m_addr_kind = AddrKind::V6;
            break;
        default:
            throw Exception("smolsocket::Sock::Sock(): Unknown address family");
            break;
    }
    freeaddrinfo(res);

    // Actually open the socket
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        throw Exception("smolsocket::Sock::Sock(): Failed to create socket");
    }

    // And connect
    // TODO: Also support bind()-ing
    err = connect(sock, res->ai_addr, res->ai_addrlen);
    if (err == -1) {
        throw Exception("smolsocket::Sock::Sock(): Failed to connect() socket");
    }
}

void Sock::send(const std::vector<uint8_t>& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const int ret = ::send(this->m_sockfd, data.data() + sent, data.size() - sent, 0);
        if (ret == -1) {
            throw Exception("smolsocket::Sock::send(): Failed to send()");
        }
        sent += (size_t)ret;
    }
}

Sock::~Sock() { close(this->m_sockfd); }

}  // namespace smolsocket
