#include "torrent_jobs.hpp"

#include <exception>
#include <memory>
#include <stdexcept>

#include "tracker.hpp"

namespace tr = tt::tracker;

namespace tt::torrent {
TrackerInteractionJob::TrackerInteractionJob(std::shared_ptr<Torrent> torrent, tr::RequestKind kind)
    : m_torrent(torrent), m_kind(kind){};

void TrackerInteractionJob::process() {
    switch (m_kind) {
        case tr::RequestKind::STARTED:
            m_torrent->start_tracker();
            break;
        case tr::RequestKind::COMPLETED:
        case tr::RequestKind::STOPPED:
        case tr::RequestKind::UPDATE:
            throw std::runtime_error("TrackerInteractionJob::process(): Unimplemented tracker request kind, aborting");
    }
}
}  // namespace tt::torrent