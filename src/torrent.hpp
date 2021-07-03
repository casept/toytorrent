#pragma once

#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <optional>
#include <string_view>
#include <vector>

#include "metainfo.hpp"
#include "piece.hpp"
#include "tracker.hpp"

namespace tt {

// A currently live torrent.
class Torrent {
   private:
    // Raw torrent metainfo.
    MetaInfo m_metainfo;
    // Data structure managing pieces of the torrent.
    piece::Map m_piece_map;
    // Handle to file on disk.
    std::fstream m_file{};
    // Our peer identity.
    peer::Peer m_us_peer;
    // Other peers we know about.
    std::vector<peer::Peer> m_peers;
    // Our tracker request params.
    tracker::Request m_tracker_req;
    // Prepares for download.
    void download_prepare();

   public:
    // Create a torrent from the given parsed torrent file.
    Torrent(const MetaInfo& parsed_file, const std::uint16_t our_port,
            std::optional<std::string_view> alternative_path);
    // Block until the torrent has been downloaded (this is of course a temporary interface until something more async
    // is developed)
    void download();
    // Block until piece at the given index is downloaded (mainly useful for testing)
    void download_piece(const std::size_t piece_idx);
};
}  // namespace tt
