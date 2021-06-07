#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

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
    virtual ~IMessage(){};
    // Get the type of message this is.
    virtual MessageType get_type() const = 0;
    // Convert the message into the form suitable for transmission over the wire.
    virtual std::vector<std::uint8_t> serialize() const = 0;
};

class MessageChoke : public IMessage {
   public:
    MessageType get_type() const;
    std::vector<std::uint8_t> serialize() const;
};
class MessageUnchoke : public IMessage {
   public:
    MessageType get_type() const;
    std::vector<std::uint8_t> serialize() const;
};
class MessageInterested : public IMessage {
   public:
    MessageType get_type() const;
    std::vector<std::uint8_t> serialize() const;
};
class MessageNotInterested : public IMessage {
   public:
    MessageType get_type() const;
    std::vector<std::uint8_t> serialize() const;
};

class MessageRequest : public IMessage {
   private:
    std::uint32_t m_piece_idx;
    std::uint32_t m_begin_offset;
    std::uint32_t m_length;

   public:
    MessageRequest(const std::uint32_t piece_idx, const std::uint32_t begin_offset, const std::uint32_t length);

    MessageType get_type() const;
    std::vector<std::uint8_t> serialize() const;
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