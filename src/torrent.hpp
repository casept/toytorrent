#pragma once

#include <cstdint>
#include <string>
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
    // Our peer identity.
    peer::Peer m_us_peer;
    // Other peers we know about.
    std::vector<peer::Peer> m_peers;
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
