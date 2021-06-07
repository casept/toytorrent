#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "shared_constants.hpp"

namespace tt::piece {

// Current state of a piece.
enum class State {
    // Want this piece
    Want,
    // Have this piece, but the hash hasn't yet checked out
    HaveUnverified,
    // Have the piece and it's hash checked out
    HaveVerified,
    // Don't have this piece and don't want it
    Unwanted,
};

// Descriptor for a variable-sized piece of the torrent.
class Piece {
   private:
    State m_state;
    std::uint32_t m_size;
    std::array<std::uint8_t, Piece_Hash_Len> m_expected_hash;
    // TODO: Keep track of peers that have this piece here
   public:
    Piece(const std::uint32_t size, const std::array<std::uint8_t, Piece_Hash_Len> expected_hash);
};

/*
 *  Mapping of piece indices to piece descriptors.
 *  Piece descriptors are owned by this data structure.
 */
class Map {
   private:
    std::vector<Piece> m_pieces;

   public:
    /*
     * Create a new map with indices based on the ordering of pieces in the vector.
     */
    Map(std::vector<Piece> pieces);
    // Obtain a reference to a given piece.
    Piece& get_piece(const std::size_t index);
};
}  // namespace tt::piece