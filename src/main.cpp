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

MetaInfo parse(std::ifstream& file) {
    // Read the torrent file

    std::deque<char> data{};
    while (true) {
        const char byte = file.get();
        if (file.eof()) {
            break;
        }
        data.push_back(byte);
    }
    if (file.bad()) {
        std::cerr << "Failed to read .torrent file!\n";
        exit(EXIT_FAILURE);
    }
    file.close();

    auto metainfo = MetaInfo(data);
    return metainfo;
}

void download(const MetaInfo torrent) {
    // Contact tracker to ask for peers
    const PeerID peer_id = PeerID();
    fmt::print("File infohash (truncated): {}\n", torrent.truncated_infohash());

    auto tracker = TrackerCommunicator(torrent.m_primary_tracker_url, 1337, peer_id, torrent.truncated_infohash());
    const auto [initial_peers, initial_next_checkin] = tracker.send_started();
    fmt::print("Tracker told us to check in again in {}\n", initial_next_checkin);
    for (const auto peer : initial_peers) {
        fmt::print("Got peer from tracker: ID: {}, IP: {}, Port: {}\n", peer.m_id.as_string(), peer.m_ip, peer.m_port);
    }
    const auto [peers, next_checkin] = tracker.send_update();
    for (const auto peer : peers) {
        fmt::print("Got peer from tracker: ID: {}, IP: {}, Port: {}\n", peer.m_id.as_string(), peer.m_ip, peer.m_port);
    }
}

int main(int argc, char** argv) {
    std::ifstream f = parse_args(argc, argv);
    const auto torrent = parse(f);
    download(torrent);

    // TODO: Actually download
}
