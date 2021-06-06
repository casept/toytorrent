#include "peer_message.hpp"

#include <cstdint>
#include <vector>

namespace tt::peer {
/* Implement the message types which carry no payload */
MessageType MessageChoke::get_type() const { return MessageType::Choke; }
std::vector<std::uint8_t> MessageChoke::serialize() const { return {}; };
MessageType MessageUnchoke::get_type() const { return MessageType::Unchoke; }
std::vector<std::uint8_t> MessageUnchoke::serialize() const { return {}; };
MessageType MessageInterested::get_type() const { return MessageType::Interested; }
std::vector<std::uint8_t> MessageInterested::serialize() const { return {}; };
MessageType MessageNotInterested::get_type() const { return MessageType::NotInterested; }
std::vector<std::uint8_t> MessageNotInterested::serialize() const { return {}; };

/* And the ones that do */

}  // namespace tt::peer