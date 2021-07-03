#include "peer_message.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "byteorder.hpp"
#include "log.hpp"
#include "smolsocket.hpp"

namespace tt::peer {

std::unique_ptr<IMessage> blocking_read_message_from_socket(smolsocket::Sock& sock,
                                                            std::optional<std::uint64_t> timeout_millis) {
    // Read the single byte specifying the message type.
    auto type_buf = sock.recv(1, timeout_millis);
    MessageType msg_type = MessageType(type_buf.at(0));

    switch (msg_type) {
        case MessageType::Piece:
            return std::make_unique<MessagePiece>(MessagePiece(sock));
            break;
        default:
            log::log(log::Level::Fatal, log::Subsystem::Peer,
                     fmt::format("blocking_read_message_from_socket(): Received message of type {} which we don't know "
                                 "how to handle yet!",
                                 msg_type));
            exit(EXIT_FAILURE);
            break;
    }
}

/* Implement the message types which carry no payload */
MessageType MessageChoke::get_type() const { return MessageType::Choke; }
std::vector<std::uint8_t> MessageChoke::serialize() const { return {}; }
MessageChoke::~MessageChoke(){};
MessageType MessageUnchoke::get_type() const { return MessageType::Unchoke; }
std::vector<std::uint8_t> MessageUnchoke::serialize() const { return {}; }
MessageUnchoke::~MessageUnchoke(){};
MessageType MessageInterested::get_type() const { return MessageType::Interested; }
std::vector<std::uint8_t> MessageInterested::serialize() const { return {}; }
MessageInterested::~MessageInterested(){};
MessageType MessageNotInterested::get_type() const { return MessageType::NotInterested; }
std::vector<std::uint8_t> MessageNotInterested::serialize() const { return {}; }
MessageNotInterested::~MessageNotInterested(){};

/* And the ones that do */

/* MessageRequest */

MessageRequest::MessageRequest(const std::uint32_t piece_idx, const std::uint32_t begin_offset,
                               const std::uint32_t length)
    : m_piece_idx(piece_idx), m_begin_offset(begin_offset), m_length(length) {}

MessageType MessageRequest::get_type() const { return MessageType::Request; }

std::vector<std::uint8_t> MessageRequest::serialize() const {
    std::vector<std::uint8_t> serialized{};
    serialized.reserve(12);

    const std::array<std::uint8_t, 4> serialized_piece_idx{bo::int_to_arr(bo::hton(this->m_piece_idx))};
    for (const std::uint8_t byte : serialized_piece_idx) {
        serialized.push_back(byte);
    }

    const auto serialized_offset{bo::int_to_arr(bo::hton(this->m_begin_offset))};
    for (const std::uint8_t byte : serialized_offset) {
        serialized.push_back(byte);
    }

    const auto serialized_length{bo::int_to_arr(bo::hton(this->m_length))};
    for (const std::uint8_t byte : serialized_length) {
        serialized.push_back(byte);
    }

    return serialized;
}

MessageRequest::~MessageRequest(){};

/* MessagePiece */

MessagePiece::MessagePiece(smolsocket::Sock& s) {
    const auto data = s.recv(12, {2000});
    std::array<std::uint8_t, 4> idx_arr{};
    std::copy_n(data.begin(), 4, idx_arr.begin());
    this->m_piece_idx = bo::ntoh(bo::arr_to_int(idx_arr));

    std::array<std::uint8_t, 4> begin_arr{};
    std::copy_n(data.begin() + 4, 4, begin_arr.begin());
    this->m_begin_offset = bo::ntoh(bo::arr_to_int(begin_arr));

    std::array<std::uint8_t, 4> len_arr{};
    std::copy_n(data.begin() + 8, 4, len_arr.begin());
    this->m_length = bo::ntoh(bo::arr_to_int(len_arr));
}

MessagePiece::MessagePiece(const std::uint32_t piece_idx, const std::uint32_t begin_offset, const std::uint32_t length)
    : m_piece_idx(piece_idx), m_begin_offset(begin_offset), m_length(length) {}

MessageType MessagePiece::get_type() const { return MessageType::Piece; }

std::vector<std::uint8_t> MessagePiece::serialize() const {
    std::vector<std::uint8_t> serialized{};
    serialized.reserve(12);
    const auto idx_arr = bo::int_to_arr(bo::hton(this->m_piece_idx));
    for (const auto b : idx_arr) {
        serialized.push_back(b);
    }
    const auto begin_arr = bo::int_to_arr(bo::hton(this->m_begin_offset));
    for (const auto b : begin_arr) {
        serialized.push_back(b);
    }
    const auto len_arr = bo::int_to_arr(bo::hton(this->m_length));
    for (const auto b : len_arr) {
        serialized.push_back(b);
    }
    return serialized;
}
MessagePiece::~MessagePiece(){};

}  // namespace tt::peer
