#include "../torrent/tracker.hpp"

#include <gtest/gtest.h>

#include "../torrent/metainfo.hpp"
#include "helpers.hpp"

using namespace tt;

TEST(TrackerCommunication, send) {
    const auto torrent_file_path = "../testdata/zip_10MB.zip.torrent";
    TrackerTestCtx ctx{};
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
    for (auto& req_kind : {tracker::RequestKind::STARTED, tracker::RequestKind::UPDATE, tracker::RequestKind::STOPPED,
                           tracker::RequestKind::COMPLETED}) {
        req.kind = req_kind;
        tracker::send_request(info.m_primary_tracker_url, req);
    }
}
