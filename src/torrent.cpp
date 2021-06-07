#include "torrent.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#include "log.hpp"
#include "metainfo.hpp"
#include "piece.hpp"
#include "tracker.hpp"

namespace tt {
Torrent::Torrent(const MetaInfo &parsed_file)
    : m_metainfo(parsed_file),
      m_piece_map({}),
      m_us_peer{peer::Peer(peer::ID(), "127.0.0.1", 1337)},
      m_tracker_req({
          tracker::RequestKind::STARTED,
          parsed_file.truncated_infohash_binary(),
          tracker::Stats{0, 0, 0},
          this->m_us_peer.m_id,
          this->m_us_peer.m_ip,
          this->m_us_peer.m_port,
      }) {
    // Our peer ID is already initialized above

    // Initialize pieces
    std::vector<piece::Piece> pieces{};
    for (const auto &expected_hash : parsed_file.m_pieces) {
        // TODO: Handle last piece being shorter
        pieces.push_back({static_cast<std::uint32_t>(parsed_file.m_piece_length), expected_hash});
    }
    this->m_piece_map = {pieces};
}

static void handshake(std::vector<peer::Peer> &peers, const peer::Peer &us_peer,
                      const std::vector<std::uint8_t> &trunc_infohash_binary) {
    for (peer::Peer &peer : peers) {
        // Don't handshake with ourselves if the tracker returns us for whatever reason
        if ((peer.m_id != us_peer.m_id) && (peer.m_ip != us_peer.m_ip) && (peer.m_port != us_peer.m_port)) {
            try {
                peer.handshake(trunc_infohash_binary, us_peer.m_id);
            } catch (const peer::Exception &e) {
                // TODO: Send into a retry queue and do exponential backoff or something
                log::log(log::Level::Warning, log::Subsystem::Torrent,
                         fmt::format("Torrent::handshake(): Peer failed: {}", e.what()));
            }
        }
    }
}

void Torrent::download() {
    // Initial tracker checkin
    auto [initial_peers, initial_next_checkin] =
        tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);
    // Handshake with all peers that aren't we ourselves
    // TODO: Send keepalives to all peers periodically
    handshake(initial_peers, this->m_us_peer, this->m_metainfo.truncated_infohash_binary());
    log::log(log::Level::Debug, log::Subsystem::Torrent, "Torrent::download(): Tried all peers");

    // Tracker checkout
    this->m_tracker_req.kind = tracker::RequestKind::STOPPED;
    tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);
}
}  // namespace tt
