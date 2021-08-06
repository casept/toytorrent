#include "torrent.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../log.hpp"
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
    } else {
        // Create if not exists
        mode = mode | std::ios::trunc;
    }

    f.exceptions(std::fstream::failbit | std::fstream::badbit);
    try {
        f.open(path.c_str(), mode);
    } catch (std::system_error &e) {
        throw std::runtime_error(fmt::format(
            "Torrent::file_open_or_create(): Failed to open destination file {} (Reason: {}), original exception: {}",
            path.c_str(), strerror(errno), e.what()));
    }
    return f;
}

Torrent::Torrent(const MetaInfo &parsed_file, const std::uint16_t our_port,
                 std::optional<std::string_view> alternative_path)
    : m_metainfo(parsed_file),
      m_piece_map({}),
      m_us_peer{std::make_shared<peer::Peer>(peer::Peer(peer::ID(), "127.0.0.1", our_port))},
      m_peers(std::vector<std::shared_ptr<peer::Peer>>()),
      m_tracker_stats({0, 0, 0}) {
    // Open file
    if (alternative_path.has_value()) {
        const std::filesystem::path p{alternative_path.value()};
        m_file = file_open_or_create(p);
    } else {
        const std::filesystem::path p{this->m_metainfo.m_suggested_name};
        m_file = file_open_or_create(p);
    }

    // Initialize pieces
    std::vector<piece::Piece> pieces{};
    std::uint32_t piece_idx = 0;
    for (const auto &expected_hash : parsed_file.m_pieces) {
        // TODO: Handle last piece being shorter
        pieces.emplace_back(static_cast<std::uint32_t>(parsed_file.m_piece_length), piece_idx, expected_hash,
                            piece::State::Want);
        piece_idx++;
    }
    m_piece_map = {pieces};
}

void Torrent::start_tracker() {
    // Request
    const auto req = tracker::Request{
        tracker::RequestKind::STARTED,
        m_metainfo.truncated_infohash_binary(),
        m_tracker_stats,
        m_us_peer->m_id,
        m_us_peer->m_ip,
        m_us_peer->m_port,
    };
    auto [new_peers, next_checkin] = tracker::send_request(m_metainfo.m_primary_tracker_url, req);
    std::vector<std::shared_ptr<peer::Peer>> new_peers_ptrs{};
    new_peers_ptrs.reserve(new_peers.size());
    for (auto &peer : new_peers) {
        new_peers_ptrs.push_back(std::make_shared<peer::Peer>(std::move(peer)));
    }

    // Update peers by merging old and new peer lists
    for (auto &peer : new_peers_ptrs) {
        // If peer is already present, keep the old version
        if (std::find(m_peers.begin(), m_peers.end(), peer) == std::end(m_peers)) {
            continue;
        } else {
            // Otherwise, insert (but only if the peer aren't we ourselves)
            if (peer != m_us_peer) {
                m_peers.push_back(std::move(peer));
            }
        }
    }
}

std::vector<std::unique_ptr<peer::PeerHandshakeJob>> Torrent::create_handshake_jobs() {
    std::vector<std::unique_ptr<peer::PeerHandshakeJob>> jobs{};
    for (auto peer : m_peers) {
        jobs.emplace_back(
            std::make_unique<peer::PeerHandshakeJob>(peer, m_metainfo.truncated_infohash_binary(), m_us_peer->m_id));
    }
    return jobs;
}
}  // namespace tt
