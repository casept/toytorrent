#include <errno.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
#include <deque>
#include <string>

#include "bencode_parser.h"

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
    auto torrent_file = parse_args(argc, argv);
    // Iterate over all the objects in the bencoded file
    std::deque<char> data {};
    // TODO: Figure out how to use std::copy here
    while (!torrent_file.eof()) {
        data.push_back(torrent_file.get());
    }
    auto parser = BEncodeParser(data);
    torrent_file.close();
    while (true) {
        auto next = parser.next();
        if (next.has_value()) {
            std::cout << "Have a bencoded value" << std::endl;
        }
        // No more values: We're done
        else {
            break;
        }
    }
}