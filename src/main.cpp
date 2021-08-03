#include <fmt/core.h>

#include <cstdlib>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "job.hpp"
#include "torrent/metainfo.hpp"
#include "torrent/torrent.hpp"
#include "torrent/torrent_jobs.hpp"
#include "torrent/tracker.hpp"

const std::uint16_t PORT = 1337;

int main(int argc, char** argv) {
    if (argc != 2) {
        fmt::print(stderr, "Usage : {} file.torrent\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    tt::job::JobQueue jobs{};

    const auto metainfo{tt::metainfo_from_path(argv[1])};
    const std::optional<std::string_view> alternative_path{};
    auto torrent{std::make_shared<tt::Torrent>(metainfo, PORT, alternative_path)};

    auto tracker_start_job{
        std::make_unique<tt::torrent::TrackerInteractionJob>(torrent, tt::tracker::RequestKind::STARTED)};
    jobs.enqueue(std::move(tracker_start_job));
    auto handshake_jobs{torrent->create_handshake_jobs()};
    jobs.enqueue_vec(std::move(handshake_jobs));
    jobs.process();
}
