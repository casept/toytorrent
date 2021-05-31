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
    : m_metainfo(parsed_file), m_us_peer(peer::Peer(peer::ID(), "127.0.0.1", 1337)) {
    // Our peer ID is already initialized above
    // TODO: Get IP and port (can we ask the tracker for IP?)

    // Initialize trackers (for now, just one)
    this->m_trackers = {};
    auto tracker = tracker::TrackerCommunicator(parsed_file.m_primary_tracker_url, this->m_us_peer.m_port,
                                                this->m_us_peer.m_id, parsed_file.truncated_infohash());

    this->m_trackers.push_back(tracker);

    // Initialize pieces
    this->m_pieces = {};
    for (const auto &expected_hash : parsed_file.m_pieces) {
        // TODO: Handle last piece being shorter
        const Piece piece = Piece(parsed_file.m_piece_length, expected_hash);
        this->m_pieces.push_back(piece);
    }
}

void Torrent::download() {
    // Initial tracker checkin
    auto tracker = this->m_trackers.at(0);
    const auto [initial_peers, initial_next_checkin] = tracker.send_started();

    // Periodic tracker checkin
    auto [peers, next_checkin] = tracker.send_update();
    // Handshake with all peers that aren't we ourselves
    // TODO: Send keepalives to all peers periodically
    for (peer::Peer &peer : peers) {
        if ((peer.m_id != this->m_us_peer.m_id) && (peer.m_ip != this->m_us_peer.m_ip) &&
            (peer.m_port != this->m_us_peer.m_port)) {
            while (!peer.is_connected()) {
                try {
                    peer.connect(this->m_metainfo.truncated_infohash());
                } catch (const peer::Exception &e) {
                    // TODO: Send into a retry queue and do exponential backoff or something
                    log::log(log::Level::Warning, log::Subsystem::Torrent,
                             fmt::format("Torrent::download(): Peer failed: {}; retrying in 5s", e.what()));
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(5s);
                }
            }
        }
    }

    // Tracker checkout
    tracker.send_stopped();
}
}  // namespace tt
