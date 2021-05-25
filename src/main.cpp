#include <errno.h>
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

// Returns the torrent file, or exits if path not provided or invalid.
std::ifstream parse_args(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-torrent-file.torrent>" << std::endl;
        exit(1);
    };
    const std::string path = argv[1];
    std::ifstream file{path};
    if (!file) {
        std::cerr << "The torrent file cannot be opened: " << strerror(errno) << std::endl;
        exit(1);
    }
    return file;
};

int main(int argc, char** argv) {
    // Read the torrent file
    auto torrent_file = parse_args(argc, argv);
    std::deque<char> data{};
    // TODO: Figure out how to use std::copy here
    while (!torrent_file.eof()) {
        data.push_back(torrent_file.get());
    }
    torrent_file.close();
    auto metainfo = MetaInfo(data);

    // TODO: Actually download
}