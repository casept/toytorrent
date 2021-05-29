#include "torrent.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "metainfo.hpp"
#include "tracker.hpp"

namespace tt {
Piece::Piece(size_t size, std::array<char, Piece_SHA1_Len> expected_hash) {
    // Each piece starts out as wanted
    this->m_state = PieceState::Want;
    this->m_size = size;
    std::copy(expected_hash.begin(), expected_hash.end(), this->m_expected_hash.begin());
}

Torrent::Torrent(const MetaInfo &parsed_file) : m_our_peer(Peer(PeerID(), "127.0.0.1", 1337)) {
    // Our peer ID is already initialized above
    // TODO: Get IP and port (can we ask the tracker for IP?)

    // Initialize trackers (for now, just one)
    this->m_trackers = {};
    auto tracker = TrackerCommunicator(parsed_file.m_primary_tracker_url, this->m_our_peer.m_port,
                                       this->m_our_peer.m_id, parsed_file.truncated_infohash());
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
    fmt::print("Tracker told us to check in again in {} seconds\n", initial_next_checkin);
    for (const auto &peer : initial_peers) {
        fmt::print("Got peer from tracker: ID: {}, IP: {}, Port: {}\n", peer.m_id.as_string(), peer.m_ip, peer.m_port);
    }
    const auto [peers, next_checkin] = tracker.send_update();
    for (const auto &peer : peers) {
        fmt::print("Got peer from tracker: ID: {}, IP: {}, Port: {}\n", peer.m_id.as_string(), peer.m_ip, peer.m_port);
    }
}
}  // namespace tt