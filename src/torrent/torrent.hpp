#pragma once

#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "metainfo.hpp"
#include "peer.hpp"
#include "piece.hpp"
#include "tracker.hpp"

namespace tt {

/// A currently live torrent.
class Torrent {
   private:
    /// Raw torrent metainfo.
    MetaInfo m_metainfo;
    /// Data structure managing pieces of the torrent.
    piece::Map m_piece_map;
    /// Handle to file on disk.
    std::fstream m_file{};
    /// Our peer identity.
    std::shared_ptr<peer::Peer> m_us_peer;
    /// Other peers we know about.
    std::vector<std::shared_ptr<peer::Peer>> m_peers;
    /// Statistics for the tracker.
    tracker::Stats m_tracker_stats;

   public:
    // Create a torrent from the given parsed torrent file.
    Torrent(const MetaInfo& parsed_file, const std::uint16_t our_port,
            std::optional<std::string_view> alternative_path);
    /// Send a start message to the torrent's tracker.
    ///
    /// This will register the client and download the initial peer list.
    void start_tracker();
    /// Construct a handshake job for each peer.
    std::vector<std::unique_ptr<peer::PeerHandshakeJob>> create_handshake_jobs();
};
}  // namespace tt
