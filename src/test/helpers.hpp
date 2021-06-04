#pragma once

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