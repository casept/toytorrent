#include "../peer.hpp"

#include <gtest/gtest.h>

#include "../metainfo.hpp"
#include "../tracker.hpp"
#include "helpers.hpp"

using namespace tt;

TEST(Peer, handshake) {
    // Setup processes
    const auto torrent_file_path = "../testdata/zip_10MB.zip.torrent";
    const auto torrent_data_dir = "../testdata";
    TrackerTestCtx tracker_ctx{};
    TorrentSwarmTestCtx torrent_ctx{torrent_file_path, torrent_data_dir};
    // Get peers
    const auto info = metainfo_from_path(torrent_file_path);
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", 12345);
    auto req = tracker::Request{
        tracker::RequestKind::STARTED,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer.m_id,
        us_peer.m_ip,
        us_peer.m_port,
    };
    auto [peers, timeout] = tracker::send_request(info.m_primary_tracker_url, req);
    // Attempt handshake
    for (auto& peer : peers) {
        peer.handshake(info.truncated_infohash_binary(), us_peer.m_id);
    }
}