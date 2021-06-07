#include "piece.hpp"

#include <utility>
#include <vector>

#include "shared_constants.hpp"

namespace tt::piece {
Piece::Piece(const std::uint32_t size, const std::array<std::uint8_t, Piece_Hash_Len> expected_hash)
    : m_size(size), m_expected_hash(expected_hash) {}

Map::Map(std::vector<Piece> pieces) : m_pieces(pieces) {}
Piece& Map::get_piece(const std::size_t index) { return this->m_pieces.at(index); }
}  // namespace tt::piece