#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "metainfo.hpp"
#include "tracker.hpp"

namespace tt {

// Current state of a piece.
enum class PieceState {
    // Want this piece
    Want,
    // Have this piece, but the hash hasn't yet checked out
    HaveUnverified,
    // Have the piece and it's hash checked out
    HaveVerified,
    // Don't have this piece and don't want it
    Unwanted,
};

// A variable-size piece of the torrent.
class Piece {
   private:
    PieceState m_state;
    size_t m_size;
    std::array<char, Piece_SHA1_Len> m_expected_hash;

   public:
    Piece(size_t size, std::array<char, Piece_SHA1_Len> expected_hash);
};

// A currently live torrent.
class Torrent {
   private:
    // Raw torrent metainfo.
    MetaInfo m_metainfo;
    // All pieces of the torrent.
    std::vector<Piece> m_pieces;
    // Our peer identity.
    peer::Peer m_us_peer;
    // Our tracker request params.
    tracker::Request m_tracker_req;

   public:
    // Create a torrent from the given parsed torrent file.
    Torrent(const MetaInfo& parsed_file);
    // Block until the torrent has been downloaded (this is of course a temporary interface until something more async
    // is developed)
    void download();
};
}  // namespace tt
