#pragma once

#include <memory>

#include "../job.hpp"
#include "piece.hpp"
#include "torrent.hpp"
#include "tracker.hpp"

namespace tr = tt::tracker;

namespace tt::torrent {
/// A job for interacting with the tracker.
/// This means sending a start, update or stop.
/// For requests that return peers, the torrent's peer list is updated.
class TrackerInteractionJob final : public job::IJob {
   public:
    TrackerInteractionJob(std::shared_ptr<Torrent> torrent, const tr::RequestKind kind);
    TrackerInteractionJob() = delete;
    void process() override;

   private:
    std::shared_ptr<Torrent> m_torrent;
    tr::RequestKind m_kind;
};

/// Downloads a piece from a peer, but does not verify the hash.
/// For now, the first peer is always chosen. The API will change this in future.
class PieceDownloadJob final : public job::IJob {
   public:
    PieceDownloadJob(std::shared_ptr<Torrent> torrent, const std::size_t piece_idx);
    PieceDownloadJob() = delete;
    void process() override;

   private:
    std::shared_ptr<Torrent> m_torrent;
    std::size_t m_piece_idx;
};
}  // namespace tt::torrent