#pragma once

#include <gtest/gtest.h>

#include <boost/process.hpp>
#include <cstdint>
#include <string_view>
#include <vector>

class TorrentSwarmTestCtx {
   public:
    // Other peers in the swarm.
    std::vector<boost::process::child> m_aria2c_peers;
    std::vector<std::uint16_t> m_aria2c_peer_ports;

    TorrentSwarmTestCtx(const std::string_view& torrent_file_path, const std::string_view& torrent_data_dir_path);
    ~TorrentSwarmTestCtx();
};

class TrackerTestCtx {
   public:
    // Handle for the tracker process.
    boost::process::child m_opentracker;
    TrackerTestCtx();
    ~TrackerTestCtx();
};

class IntegrationTestCtx {
   public:
    bool m_have_handshaked;
    TrackerTestCtx m_tracker;
    TorrentSwarmTestCtx m_swarm;
    IntegrationTestCtx(const std::string_view& torrent_file_path, const std::string_view& torrent_data_dir_path);
};

const auto Torrent_File_Path = "../testdata/zip_10MB.zip.torrent";
const auto Torrent_Data_Dir = "../testdata";

class IntegrationTest : public ::testing::Test {
   public:
    inline static IntegrationTestCtx* m_ctx;
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};
