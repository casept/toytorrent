#include "../tracker.hpp"

#include <gtest/gtest.h>

#include "../metainfo.hpp"
#include "helpers.hpp"

using namespace tt;

TEST(TrackerCommunication, send_started) {
    const auto torrent_file_path = "../src/test/testdata/lubuntu-16.04-localtrack.torrent";
    IntegrationTestCtx ctx{torrent_file_path};
    const auto info = metainfo_from_path(torrent_file_path);
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", 12345);
    const auto req = tracker::Request{
        tracker::RequestKind::STARTED,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer,
    };
    tracker::send_request(info.m_primary_tracker_url, req);
}

TEST(TrackerCommunication, send_stopped) {
    const auto torrent_file_path = "../src/test/testdata/lubuntu-16.04-localtrack.torrent";
    IntegrationTestCtx ctx{torrent_file_path};
    const auto info = metainfo_from_path(torrent_file_path);
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", 12345);
    const auto req = tracker::Request{
        tracker::RequestKind::STOPPED,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer,
    };
    tracker::send_request(info.m_primary_tracker_url, req);
}

TEST(TrackerCommunication, send_completed) {
    const auto torrent_file_path = "../src/test/testdata/lubuntu-16.04-localtrack.torrent";
    IntegrationTestCtx ctx{torrent_file_path};
    const auto info = metainfo_from_path(torrent_file_path);
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", 12345);
    const auto req = tracker::Request{
        tracker::RequestKind::COMPLETED,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer,
    };
    tracker::send_request(info.m_primary_tracker_url, req);
}

TEST(TrackerCommunication, send_update) {
    const auto torrent_file_path = "../src/test/testdata/lubuntu-16.04-localtrack.torrent";
    IntegrationTestCtx ctx{torrent_file_path};
    const auto info = metainfo_from_path(torrent_file_path);
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", 12345);
    const auto req = tracker::Request{
        tracker::RequestKind::UPDATE,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer,
    };
    tracker::send_request(info.m_primary_tracker_url, req);
}
