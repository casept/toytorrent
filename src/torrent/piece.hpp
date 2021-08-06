#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../job.hpp"
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

    Piece(const std::uint32_t size, const std::uint32_t idx,
          const std::array<std::uint8_t, Piece_Hash_Len> expected_hash, const State state);

    std::array<std::uint8_t, Piece_Hash_Len> get_curr_hash();
    std::string get_expected_hash_str();
    std::string get_curr_hash_str();
    bool hashes_match();

    void set_downloaded_subpiece_data(const std::size_t subpiece_idx, const std::vector<std::uint8_t>& data);
    void flush_to_disk(std::shared_ptr<std::fstream> f);
};

/*
 *  Mapping of piece indices to piece descriptors.
 *  Piece descriptors are owned by this data structure.
 */
class Map {
   private:
    std::vector<std::shared_ptr<Piece>> m_pieces;

   public:
    /// Create a new map with indices based on the ordering of pieces in the vector.
    /// From now on, the map owns the pieces.
    Map(std::vector<Piece> pieces);
    /// Obtain a non-owning, mutable reference to a given piece.
    std::shared_ptr<Piece> get_piece(const std::size_t index);
};

/// Verifies the hash of an already downloaded piece.
/// The state of the piece is set accordingly.
class PieceVerificationJob final : public job::IJob {
   public:
    PieceVerificationJob(std::shared_ptr<piece::Piece> p);
    PieceVerificationJob() = delete;
    void process() override;

   private:
    std::shared_ptr<piece::Piece> m_piece;
};

/// Flushes a piece to disk.
/// Note that the piece has to be verified, or an exception will be thrown.
class PieceFlushJob final : public job::IJob {
   public:
    PieceFlushJob(std::shared_ptr<piece::Piece> p, std::shared_ptr<std::fstream> file);
    PieceFlushJob() = delete;
    void process() override;

   private:
    std::shared_ptr<piece::Piece> m_piece;
    std::shared_ptr<std::fstream> m_file;
};
}  // namespace tt::piece
