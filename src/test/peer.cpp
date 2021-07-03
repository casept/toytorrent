#include "../peer.hpp"

#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>

#include <cstdint>
#include <memory>
#include <tuple>

#include "../metainfo.hpp"
#include "../tracker.hpp"
#include "helpers.hpp"

using namespace tt;

const auto Torrent_File_Path = "../testdata/zip_10MB.zip.torrent";
const auto Torrent_Data_Dir = "../testdata";
class Peer : public ::testing::Test {
   public:
    inline static IntegrationTestCtx* m_ctx;
    static void SetUpTestSuite() { Peer::m_ctx = new IntegrationTestCtx(Torrent_File_Path, Torrent_Data_Dir); }

    static void TearDownTestSuite() { delete m_ctx; }
};

TEST_F(Peer, handshake) {
    const auto info = metainfo_from_path(Torrent_File_Path);
    const std::uint16_t us_port = 12345;
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", us_port);
    auto req = tracker::Request{
        tracker::RequestKind::STARTED,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer.m_id,
        us_peer.m_ip,
        us_peer.m_port,
    };
    auto [peers, timeout] = tracker::send_request(info.m_primary_tracker_url, req);

    // Attempt handshake
    for (auto& peer : peers) {
        // TODO: This should probably be handled somewhere else
        if (peer.m_port != us_peer.m_port) {
            peer.handshake(info.truncated_infohash_binary(), us_peer.m_id);
        }
    }
}

TEST_F(Peer, request_piece) {
    // Get peers
    const auto info = metainfo_from_path(Torrent_File_Path);
    const std::uint16_t us_port = 12345;
    const auto us_peer = peer::Peer(peer::ID(), "127.0.0.1", us_port);
    auto req = tracker::Request{
        tracker::RequestKind::STARTED,
        info.truncated_infohash_binary(),
        tracker::Stats{0, 0, 0},
        us_peer.m_id,
        us_peer.m_ip,
        us_peer.m_port,
    };
    auto [peers, timeout] = tracker::send_request(info.m_primary_tracker_url, req);

    // Attempt handshake and request piece
    const std::uint32_t piece_to_request = 0;
    const std::uint32_t offset_to_request = 0;
    for (auto& peer : peers) {
        if (peer.m_port != us_peer.m_port) {
            peer.handshake(info.truncated_infohash_binary(), us_peer.m_id);

            const auto request = tt::peer::MessageRequest(0, 0, info.m_piece_length);
            peer.send_message(request);

            // Process and verify reply
            auto response = peer.wait_for_message();
            if (response->get_type() != tt::peer::MessageType::Piece) {
                FAIL() << "Got unexpected message type";
            }
            const auto& response_piece = dynamic_cast<const tt::peer::MessagePiece&>((*response.release()));
            ASSERT_EQ(response_piece.m_piece_idx, piece_to_request);
            ASSERT_EQ(response_piece.m_begin_offset, offset_to_request);
            ASSERT_EQ(response_piece.get_piece_data().size(), info.m_piece_length);
        }
    }
}