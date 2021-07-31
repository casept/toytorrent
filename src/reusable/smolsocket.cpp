#include "smolsocket.hpp"

#include <cstdint>

extern "C" {
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
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

const char* Exception::what() const noexcept { return this->m_msg.c_str(); }

static void enable_timeout(int sockfd, std::optional<std::uint64_t> timeout_millis) {
    // TODO: This is Linux-only, but select() based impl looks to be a massive pain
    if (timeout_millis.has_value()) {
        struct timeval timeout {};
        timeout.tv_sec = timeout_millis.value() / 1000;
        timeout.tv_usec = (timeout_millis.value() * 1000) % 1000;
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    }
}

static void disable_timeout(int sockfd) {
    struct timeval disable_timeout;
    disable_timeout.tv_sec = 0;
    disable_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &disable_timeout, sizeof(disable_timeout));
}

Sock::Sock(const std::string_view& addr, const uint16_t port, const Proto proto,
           std::optional<std::uint64_t> timeout_millis) {
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
    if (res == nullptr) {
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

    // Set timeout if desired
    enable_timeout(this->m_sockfd.value(), timeout_millis);

    // TODO: Also support bind()-ing
    // And connect
    err = connect(this->m_sockfd.value(), res->ai_addr, res->ai_addrlen);
    if (err == -1) {
        freeaddrinfo(res);
        disable_timeout(this->m_sockfd.value());
        throw Exception("smolsocket::Sock::Sock(): Failed to connect() socket: ", {errno}, {});
    }
    // Re-disable timeout
    disable_timeout(this->m_sockfd.value());
    freeaddrinfo(res);
}

Sock::Sock(Sock&& src) : m_sockfd(src.m_sockfd), m_addr_kind(src.m_addr_kind), m_proto(src.m_proto) {
    src.m_sockfd = -1;
}

Sock& Sock::operator=(Sock&& other) {
    if (this != &other) {
        // Give resources to new instance
        this->m_sockfd = other.m_sockfd;
        this->m_proto = other.m_proto;
        this->m_addr_kind = other.m_addr_kind;

        // Take resources away from old instance
        other.m_sockfd = {};

        return *this;
    }
    return other;
}

void Sock::send(const std::vector<uint8_t>& data, std::optional<std::uint64_t> timeout_millis) {
    enable_timeout(this->m_sockfd.value(), timeout_millis);
    size_t sent = 0;
    while (sent < data.size()) {
        const ssize_t ret = ::send(this->m_sockfd.value(), data.data() + sent, data.size() - sent, 0);
        if (ret == -1) {
            disable_timeout(this->m_sockfd.value());
            throw Exception("smolsocket::Sock::send(): Failed to send(): ", {errno}, {});
        }
        sent += static_cast<std::size_t>(ret);
    }
    disable_timeout(this->m_sockfd.value());
}

std::vector<std::uint8_t> Sock::recv(const std::size_t data_size, const std::optional<std::uint64_t> timeout_millis) {
    std::vector<std::uint8_t> buf{};
    buf.reserve(data_size);
    enable_timeout(this->m_sockfd.value(), timeout_millis);
    std::size_t recvd = 0;

    while (recvd < data_size) {
        const ssize_t ret = ::recv(this->m_sockfd.value(), buf.data() + recvd, data_size - recvd, 0);
        if (ret == -1) {
            disable_timeout(this->m_sockfd.value());
            throw Exception("smolsocket::Sock::send(): Failed to send(): ", {errno}, {});
        }
        recvd += static_cast<std::size_t>(ret);
    }
    disable_timeout(this->m_sockfd.value());
    return buf;
}

Sock::~Sock() {
    if (this->m_sockfd.has_value()) {
        close(this->m_sockfd.value());
    }
}

std::string ip_to_str(const std::array<uint8_t, V6_Len_Bytes>& bytes, const AddrKind kind) {
    std::array<char, std::max(V4_Len_Bytes, V6_Len_Bytes) + 1> buf;
    std::fill(buf.begin(), buf.end(), '\0');
    const char* res;
    switch (kind) {
        case AddrKind::V4:
            res = inet_ntop(AF_INET, bytes.data(), buf.data(), buf.size());
            break;
        case AddrKind::V6:
            res = inet_ntop(AF_INET6, bytes.data(), buf.data(), buf.size());
            break;
        default:
            std::cerr << "smolsocket::util::ip_to_str(): Unknown address kind. This is a bug." << std::endl;
            std::exit(EXIT_FAILURE);
            break;
    }
    if (res == nullptr) {
        throw Exception(
            "smolsocket::util::ip_to_str(): Failed to convert numeric IP to string representation: inet_ntop failed: ",
            {errno}, {});
    }
    auto as_str = std::string(buf.begin(), buf.end());
    as_str.erase(std::find(as_str.begin(), as_str.end(), '\0'), as_str.end());
    return as_str;
}

}  // namespace smolsocket
