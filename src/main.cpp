#include <fmt/core.h>

#include <cstdlib>

#include "metainfo.hpp"
#include "torrent.hpp"
#include "tracker.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        fmt::print(stderr, "Usage : {} file.torrent\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const auto metainfo = tt::metainfo_from_path(argv[1]);
    auto torrent = tt::Torrent(metainfo, 1337, {});
    torrent.download();
}
