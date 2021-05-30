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

#include "metainfo.hpp"
#include "torrent.hpp"
#include "tracker.hpp"

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
}

tt::MetaInfo file_to_metainfo(std::ifstream& file) {
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

    return tt::MetaInfo(data);
}

int main(int argc, char** argv) {
    std::ifstream f = parse_args(argc, argv);
    const auto metainfo = file_to_metainfo(f);
    auto torrent = tt::Torrent(metainfo);
    torrent.download();
}
