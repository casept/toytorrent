#include "torrent.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include "log.hpp"
#include "metainfo.hpp"
#include "peer.hpp"
#include "peer_message.hpp"
#include "piece.hpp"
#include "shared_constants.hpp"
#include "tracker.hpp"

namespace tt {

// Tries to open a file, creating it if it doesn't exist.
static std::fstream file_open_or_create(const std::filesystem::path &path) {
    std::fstream f;
    std::ios::openmode mode = std::ios::binary | std::ios::in | std::ios::out;

    if (std::filesystem::exists(path)) {
        // Check whether file is usable
        if (!std::filesystem::is_regular_file(path) && !std::filesystem::is_symlink(path)) {
            throw std::runtime_error(
                fmt::format("Torrent::file_open_or_create(): Failed to open destination file {} (Reason: File is not "
                            "regular file or symlink)",
                            path.c_str()));
        }
        f.open(path.c_str(), mode);
    } else {
        // Create if not exists
        mode = mode | std::ios::trunc;
    }
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        f.open(path.c_str(), mode);
    } catch (std::system_error &e) {
        throw std::runtime_error(
            fmt::format("Torrent::file_open_or_create(): Failed to open destination file {} (Reason: {})", path.c_str(),
                        strerror(errno)));
    }
    return f;
}

Torrent::Torrent(const MetaInfo &parsed_file, const std::uint16_t our_port,
                 std::optional<std::string_view> alternative_path)
    : m_metainfo(parsed_file),
      m_piece_map({}),
      m_us_peer{peer::Peer(peer::ID(), "127.0.0.1", our_port)},
      m_tracker_req({
          tracker::RequestKind::STARTED,
          parsed_file.truncated_infohash_binary(),
          tracker::Stats{0, 0, 0},
          this->m_us_peer.m_id,
          this->m_us_peer.m_ip,
          this->m_us_peer.m_port,
      }) {
    // Open file
    if (alternative_path.has_value()) {
        const std::filesystem::path p{alternative_path.value()};
        this->m_file = file_open_or_create(p);
    } else {
        const std::filesystem::path p{this->m_metainfo.m_suggested_name};
        this->m_file = file_open_or_create(p);
    }

    // Initialize pieces
    std::vector<piece::Piece> pieces{};
    std::uint32_t piece_idx = 0;
    for (const auto &expected_hash : parsed_file.m_pieces) {
        // TODO: Handle last piece being shorter
        pieces.push_back({static_cast<std::uint32_t>(parsed_file.m_piece_length), piece_idx, expected_hash});
        piece_idx++;
    }
    this->m_piece_map = {pieces};
}

// Handshakes with all peers.
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

void Torrent::download_prepare() {
    // Initial tracker checkin
    auto [initial_peers, initial_next_checkin] =
        tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);

    // Handshake with all peers that aren't we ourselves
    // TODO: Send keepalives to all peers periodically
    handshake(initial_peers, this->m_us_peer, this->m_metainfo.truncated_infohash_binary());
    log::log(log::Level::Debug, log::Subsystem::Torrent, "Torrent::download(): Tried all peers");
}

static void download_piece_from(piece::Piece &wanted, peer::Peer &p, std::fstream &f) {
    // Request subpieces from peer sequentially until finished
    std::uint32_t subpiece_idx = 0;
    for (auto &subpiece : wanted.m_subpieces) {
        if (!subpiece.has_value()) {
            const auto req = peer::MessageRequest(wanted.m_idx, subpiece_idx * peer::Request_Subpiece_Size,
                                                  peer::Request_Subpiece_Size);
            p.send_message(req);
            const auto &msg = p.wait_for_message();
            if (msg->get_type() != peer::MessageType::Piece) {
                log::log(log::Level::Debug, log::Subsystem::Torrent,
                         fmt::format("Torrent::download(): Expected message of type {}, got {}. Ignoring.",
                                     peer::MessageType::Piece, msg->get_type()));
            }
            // TODO: Push contents into piece's subpiece
        }
        subpiece_idx++;
    }
    wanted.m_state = piece::State::HaveUnverified;

    // Compare hashes
    if (!wanted.hashes_match()) {
        // TODO: We probably want to reset the piece and enqueue for redownload
        log::log(log::Level::Fatal, log::Subsystem::Torrent, "Piece hash mismatch");
    }

    // Flush to disk
    wanted.flush_to_disk(f);
    wanted.m_state = piece::State::HaveVerified;
}

void Torrent::download_piece(const std::size_t piece_idx) {
    this->download_prepare();

    // For now, perform a very stupid strategy of downloading sequentially from a single peer.
    // TODO: Do load balancing and all that
    auto &single_peer = this->m_peers.at(0);
    // TODO: Probably need to unchoke peer

    while (true) {
        auto wanted = this->m_piece_map.get_piece(piece_idx);
        download_piece_from(wanted, single_peer, this->m_file);
    }
    log::log(log::Level::Debug, log::Subsystem::Torrent, "Torrent::download(): Download complete!");

    // Tracker checkout
    this->m_tracker_req.kind = tracker::RequestKind::STOPPED;
    tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);
}

void Torrent::download() {
    this->download_prepare();

    // For now, perform a very stupid strategy of downloading sequentially from a single peer.
    // TODO: Do load balancing and all that
    auto &single_peer = this->m_peers.at(0);
    // TODO: Probably need to unchoke peer

    while (true) {
        auto wanted_wrapped = this->m_piece_map.get_best_wanted_piece();
        if (!wanted_wrapped.has_value()) {
            break;  // Done
        }
        auto &wanted = wanted_wrapped.value().get();
        download_piece_from(wanted, single_peer, this->m_file);
    }
    log::log(log::Level::Debug, log::Subsystem::Torrent, "Torrent::download(): Download complete!");

    // Tracker checkout
    this->m_tracker_req.kind = tracker::RequestKind::STOPPED;
    tracker::send_request(this->m_metainfo.m_primary_tracker_url, this->m_tracker_req);
}

}  // namespace tt
