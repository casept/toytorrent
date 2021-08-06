#include "torrent_jobs.hpp"

#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>

#include "../log.hpp"
#include "tracker.hpp"

namespace tr = tt::tracker;

namespace tt::torrent {
TrackerInteractionJob::TrackerInteractionJob(std::shared_ptr<Torrent> torrent, const tr::RequestKind kind)
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

PieceDownloadJob::PieceDownloadJob(std::shared_ptr<Torrent> torrent, const std::size_t piece_idx)
    : m_torrent(torrent), m_piece_idx(piece_idx){};

void PieceDownloadJob::process() {
    // TODO: Use a more reasonable way to choose a peer
    const auto p = m_torrent->m_peers.at(0);
    auto wanted{m_torrent->m_piece_map.get_piece(m_piece_idx)};

    // Request subpieces from peer sequentially until finished
    // TODO: Make subpiece downloads their own jobs
    std::uint32_t subpiece_idx = 0;
    for (auto &subpiece : wanted->m_subpieces) {
        if (!subpiece.has_value()) {
            const auto req = peer::MessageRequest(wanted->m_idx, subpiece_idx * peer::Request_Subpiece_Size,
                                                  peer::Request_Subpiece_Size);
            p->send_message(req);
            const auto &msg = p->wait_for_message();
            // TODO: Have a message pump with peek() or something rather than discarding messages
            if (msg->get_type() != peer::MessageType::Piece) {
                log::log(log::Level::Debug, log::Subsystem::Torrent,
                         fmt::format("Torrent::download(): Expected message of type {}, got {}. Ignoring.",
                                     peer::MessageType::Piece, msg->get_type()));
            }
            // Push contents into subpiece
            const auto piece_msg = dynamic_cast<const peer::MessagePiece *>(msg.get());
            wanted->set_downloaded_subpiece_data(static_cast<std::size_t>(subpiece_idx), piece_msg->get_piece_data());
        }
        subpiece_idx++;
    }
    wanted->m_state = piece::State::HaveUnverified;
}

}  // namespace tt::torrent
