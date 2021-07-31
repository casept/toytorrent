#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../reusable/smolsocket.hpp"

namespace tt::peer {

// Messages that can be sent to a peer.
enum class MessageType {
    Choke = 0,
    Unchoke = 1,
    Interested = 2,
    NotInterested = 3,
    Have = 4,
    Bitfield = 5,
    Request = 6,
    Piece = 7,
    Cancel = 8,
};

// An interface for a BitTorrent protocol peer message.
class IMessage {
   public:
    // Get the type of message this is.
    virtual MessageType get_type() const = 0;
    // Convert the message into the form suitable for transmission over the wire.
    virtual std::vector<std::uint8_t> serialize() const = 0;
    virtual ~IMessage() {}
};

/*
 * Read and parse a message from the given socket.
 * The reason why this has to be coupled to the socket is that various types of messages are dynamically-sized,
 * and therefore have to be able to request more data on their own.
 */
std::unique_ptr<IMessage> blocking_read_message_from_socket(smolsocket::Sock& sock,
                                                            std::optional<std::uint64_t> timeout_millis,
                                                            std::size_t piece_size);

class MessageChoke : public IMessage {
   public:
    MessageType get_type() const override;
    std::vector<std::uint8_t> serialize() const override;
    ~MessageChoke() override;
};
class MessageUnchoke : public IMessage {
   public:
    MessageType get_type() const override;
    std::vector<std::uint8_t> serialize() const override;
    ~MessageUnchoke() override;
};
class MessageInterested : public IMessage {
   public:
    MessageType get_type() const override;
    std::vector<std::uint8_t> serialize() const override;
    ~MessageInterested() override;
};
class MessageNotInterested : public IMessage {
   public:
    MessageType get_type() const override;
    std::vector<std::uint8_t> serialize() const override;
    ~MessageNotInterested() override;
};

class MessageRequest : public IMessage {
   private:
    std::uint32_t m_piece_idx;
    std::uint32_t m_begin_offset;
    std::uint32_t m_length;

   public:
    MessageRequest(const std::uint32_t piece_idx, const std::uint32_t begin_offset, const std::uint32_t length);

    MessageType get_type() const override;
    std::vector<std::uint8_t> serialize() const override;
    ~MessageRequest() override;
};

class MessagePiece : public IMessage {
   private:
    std::vector<std::uint8_t> m_piece_data;

   public:
    std::uint32_t m_piece_idx;
    std::uint32_t m_begin_offset;

    MessagePiece(smolsocket::Sock& s, const std::size_t piece_len);
    MessagePiece(const std::uint32_t piece_idx, const std::uint32_t begin_offset,
                 const std::vector<std::uint8_t>& piece_data);
    MessagePiece(const MessagePiece&) = default;

    MessageType get_type() const override;
    std::vector<std::uint8_t> serialize() const override;
    const std::vector<std::uint8_t>& get_piece_data() const;

    ~MessagePiece() override;
};

}  // namespace tt::peer

namespace fmt {
template <>
struct formatter<tt::peer::MessageType> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const tt::peer::MessageType& t, FormatContext& ctx) {
        using namespace tt::peer;
        std::string type = "Unknown";
        switch (t) {
            case MessageType::Choke:
                type = "Choke";
                break;
            case MessageType::Unchoke:
                type = "Unchoke";
                break;
            case MessageType::Interested:
                type = "Interested";
                break;
            case MessageType::NotInterested:
                type = "NotInterested";
                break;
            case MessageType::Have:
                type = "Have";
                break;
            case MessageType::Bitfield:
                type = "Bitfield";
                break;
            case MessageType::Request:
                type = "Request";
                break;
            case MessageType::Piece:
                type = "Piece";
                break;
            case MessageType::Cancel:
                type = "Cancel";
                break;
            default:
                type = "Unknown";
        }
        return format_to(ctx.out(), "{}", type.c_str());
    }
};

template <>
struct formatter<tt::peer::IMessage> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const tt::peer::IMessage& m, FormatContext& ctx) {
        std::string serialized{};
        for (const uint8_t c : m.serialize()) {
            serialized.push_back(static_cast<char>(c));
        }
        return format_to(ctx.out(), "{{Type: \"{}\", Serialized: \"{}\"}}", m.get_type(), serialized.c_str());
    }
};
}  // namespace fmt
