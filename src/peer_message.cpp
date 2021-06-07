#include "peer_message.hpp"

#include <array>
#include <cstdint>
#include <vector>

#include "smolsocket.hpp"

namespace tt::peer {
/* Implement the message types which carry no payload */
MessageType MessageChoke::get_type() const { return MessageType::Choke; }
std::vector<std::uint8_t> MessageChoke::serialize() const { return {}; }
MessageType MessageUnchoke::get_type() const { return MessageType::Unchoke; }
std::vector<std::uint8_t> MessageUnchoke::serialize() const { return {}; }
MessageType MessageInterested::get_type() const { return MessageType::Interested; }
std::vector<std::uint8_t> MessageInterested::serialize() const { return {}; }
MessageType MessageNotInterested::get_type() const { return MessageType::NotInterested; }
std::vector<std::uint8_t> MessageNotInterested::serialize() const { return {}; }

/* And the ones that do */

MessageRequest::MessageRequest(const std::uint32_t piece_idx, const std::uint32_t begin_offset,
                               const std::uint32_t length)
    : m_piece_idx(piece_idx), m_begin_offset(begin_offset), m_length(length) {}
MessageType MessageRequest::get_type() const { return MessageType::Request; }
std::vector<std::uint8_t> MessageRequest::serialize() const {
    using namespace smolsocket::util;
    std::vector<std::uint8_t> serialized{};
    serialized.reserve(12);

    const std::array<std::uint8_t, 4> serialized_piece_idx{int_to_arr(hton(this->m_piece_idx))};
    for (const std::uint8_t byte : serialized_piece_idx) {
        serialized.push_back(byte);
    }

    const auto serialized_offset{int_to_arr(hton(this->m_begin_offset))};
    for (const std::uint8_t byte : serialized_offset) {
        serialized.push_back(byte);
    }

    const auto serialized_length{int_to_arr(hton(this->m_length))};
    for (const std::uint8_t byte : serialized_length) {
        serialized.push_back(byte);
    }

    return serialized;
}
}  // namespace tt::peer