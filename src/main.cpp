#include <fmt/core.h>

#include "metainfo.hpp"
#include "torrent.hpp"
#include "tracker.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        fmt::print(stderr, "Usage : {} file.torrent", argv[0]);
    }
    const auto metainfo = tt::metainfo_from_path(argv[1]);
    auto torrent = tt::Torrent(metainfo);
    torrent.download();
}
