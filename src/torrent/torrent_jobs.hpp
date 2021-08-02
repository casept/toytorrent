#pragma once

#include <memory>

#include "../job.hpp"
#include "torrent.hpp"
#include "tracker.hpp"

namespace tr = tt::tracker;

namespace tt::torrent {
/// A job for interacting with the tracker.
/// This means sending a start, update or stop.
/// For requests that return peers, the torrent's peer list is updated.
class TrackerInteractionJob final : public job::IJob {
   public:
    TrackerInteractionJob(std::shared_ptr<Torrent> torrent, tr::RequestKind kind);
    TrackerInteractionJob() = delete;
    void process() override;

   private:
    std::shared_ptr<Torrent> m_torrent;
    tr::RequestKind m_kind;
};

}  // namespace tt::torrent