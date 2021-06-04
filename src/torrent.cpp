#include "torrent.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#include "log.hpp"
#include "metainfo.hpp"
#include "tracker.hpp"

namespace tt {
Piece::Piece(size_t size, std::array<char, Piece_SHA1_Len> expected_hash) {
    // Each piece starts out as wanted
    this->m_state = PieceState::Want;
    this->m_size = size;
    std::copy(expected_hash.begin(), expected_hash.end(), this->m_expected_hash.begin());
}

Torrent::Torrent(const MetaInfo &parsed_file)
    : m_metainfo(parsed_file),
      m_us_peer(peer::Peer(peer::ID(), "127.0.0.1", 1337)),
      m_tracker_req({
          tracker::RequestKind::STARTED,
          parsed_file.truncated_infohash_binary(),
          tracker::Stats{0, 0, 0},
          this->m_us_peer,
      }) {
    // Our peer ID is already initialized above

    // Initialize pieces
    this->m_pieces = {};
    for (const auto &expected_hash : parsed_file.m_pieces) {
        // TODO: Handle last piece being shorter
        const Piece piece = Piece(parsed_file.m_piece_length, expected_hash);
        this->m_pieces.push_back(piece);
    }

    // Initialize tracker params
    this->m_tracker_req = {
        tracker::RequestKind::STARTED,
        parsed_file.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        this->m_us_peer,
    };
}

void Torrent::download() {
    // Initial tracker checkin
    auto [initial_peers, initial_next_checkin] =
        tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);
    // Handshake with all peers that aren't we ourselves
    // TODO: Send keepalives to all peers periodically
    for (peer::Peer &peer : initial_peers) {
        if ((peer.m_id != this->m_us_peer.m_id) && (peer.m_ip != this->m_us_peer.m_ip) &&
            (peer.m_port != this->m_us_peer.m_port)) {
            try {
                peer.handshake(this->m_metainfo.truncated_infohash_binary(), this->m_us_peer.m_id);
            } catch (const peer::Exception &e) {
                // TODO: Send into a retry queue and do exponential backoff or something
                log::log(log::Level::Warning, log::Subsystem::Torrent,
                         fmt::format("Torrent::download(): Peer failed: {}", e.what()));
            }
        }
    }
    log::log(log::Level::Debug, log::Subsystem::Torrent, "Torrent::download(): Tried all peers");

    // Tracker checkout
    this->m_tracker_req.kind = tracker::RequestKind::STOPPED;
    tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);
}
}  // namespace tt
