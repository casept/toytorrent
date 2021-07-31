#include <array>
#include <cstdint>

namespace bo {
// Convert from network to host endianness
std::uint16_t ntoh(std::uint16_t x);
std::uint16_t ntoh(std::array<char, 2> arr);
// Convert from host to network endianness
std::uint16_t hton(std::uint16_t x);
std::uint32_t hton(std::uint32_t x);
// Convert from integer type to byte array (endianness is not changed!)
std::array<std::uint8_t, 4> int_to_arr(std::uint32_t x);
// Convert from byte array to integer type (endianness is not changed!)
std::uint32_t arr_to_int(std::array<std::uint8_t, 4> x);
}  // namespace bo
