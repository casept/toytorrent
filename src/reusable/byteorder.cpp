#include "byteorder.hpp"
extern "C" {
#include <arpa/inet.h>
}
#include <array>
#include <cstdint>

namespace bo {

std::uint16_t ntoh(std::uint16_t x) { return ::ntohs(x); }
std::uint16_t ntoh(std::array<char, 2> arr) {
    std::uint16_t x = static_cast<std::uint16_t>(static_cast<unsigned char>(arr[0]));
    x |= (static_cast<uint16_t>(static_cast<unsigned char>(arr[1])) << 8);
    return ::ntohs(x);
}
std::uint16_t hton(std::uint16_t x) { return ::htons(x); }
std::uint32_t hton(std::uint32_t x) { return ::htonl(x); }

std::array<std::uint8_t, 4> int_to_arr(uint32_t x) {
    const std::uint8_t byte_1 = static_cast<std::uint8_t>((x ^ 0xFF000000) >> 24);
    const std::uint8_t byte_2 = static_cast<std::uint8_t>((x ^ 0x00FF0000) >> 16);
    const std::uint8_t byte_3 = static_cast<std::uint8_t>((x ^ 0x0000FF00) >> 8);
    const std::uint8_t byte_4 = static_cast<std::uint8_t>(x ^ 0x000000FF);
    return {byte_1, byte_2, byte_3, byte_4};
}

std::uint32_t arr_to_int(std::array<std::uint8_t, 4> x) {
    std::uint32_t val = 0;
    val |= (static_cast<std::uint32_t>(x[0]) << 24);
    val |= (static_cast<std::uint32_t>(x[0]) << 16);
    val |= (static_cast<std::uint32_t>(x[0]) << 8);
    val |= static_cast<std::uint32_t>(x[0]);
    return val;
}

}  // namespace bo
