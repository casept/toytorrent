#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
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
   public:
    State m_state;
    std::uint32_t m_size;
    std::uint32_t m_idx;
    std::array<std::uint8_t, Piece_Hash_Len> m_expected_hash;
    // TODO: Keep track of peers that have this piece here
    // Subpieces small enough to request with a single peer message  that make up this chunk.
    std::vector<std::optional<std::vector<std::uint8_t>>> m_subpieces{};

    Piece(const std::uint32_t size, const std::uint32_t m_idx,
          const std::array<std::uint8_t, Piece_Hash_Len> expected_hash);

    std::array<std::uint8_t, Piece_Hash_Len> get_curr_hash();
    std::string get_expected_hash_str();
    std::string get_curr_hash_str();
    bool hashes_match();

    void set_downloaded_subpiece_data(const std::size_t subpiece_idx, const std::vector<std::uint8_t> &data);
    void flush_to_disk(std::fstream& f);
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
    /*
     * Return the piece that would be most valuable to obtain next, if any.
     * For now, that's simply the first wanted piece.
     */
    std::optional<std::reference_wrapper<Piece>> get_best_wanted_piece();
};
}  // namespace tt::piece