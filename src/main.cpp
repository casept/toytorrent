#include <fmt/core.h>
#include <string.h>

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "metainfo.h"
#include "tracker.h"

// Returns the torrent file, or exits if path not provided or invalid.
std::ifstream parse_args(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-torrent-file.torrent>" << std::endl;
        exit(1);
    };
    const std::string path = argv[1];
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) {
        std::cerr << "The torrent file cannot be opened\n";
        exit(EXIT_FAILURE);
    }
    return f;
};

int main(int argc, char** argv) {
    // Read the torrent file
    auto f = parse_args(argc, argv);
    std::deque<char> data{};
    while (true) {
        const char byte = f.get();
        if (f.eof()) {
            break;
        }
        data.push_back(byte);
    }
    if (f.bad()) {
        std::cerr << "Failed to read .torrent file!\n";
        exit(EXIT_FAILURE);
    }
    f.close();

    auto metainfo = MetaInfo(data);
    // Contact tracker to ask for peers
    const PeerID peer_id = {"ffffffffffffffffffff"};
    fmt::print("File infohash (truncated): {}\n", metainfo.truncated_infohash());

    auto tracker = TrackerCommunicator(metainfo.m_primary_tracker_url, 1337, peer_id, metainfo.truncated_infohash());
    const auto [peers, next_checkin] = tracker.send_started();
    for (const auto peer : peers) {
        fmt::print("Got peer from tracker: ID: {}, IP: {}, Port: {}\n", peer.m_id.data(), peer.m_ip, peer.m_port);
    }
    // TODO: Actually download
}