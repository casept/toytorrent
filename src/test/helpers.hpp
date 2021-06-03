#pragma once

#include <boost/process.hpp>
#include <cstdint>
#include <string_view>
#include <vector>

class IntegrationTestCtx {
   public:
    // The tracker.
    boost::process::child m_opentracker;
    // Other peers in the swarm.
    std::vector<boost::process::child> m_aria2c_peers;
    std::vector<std::uint16_t> m_aria2c_peer_ports;

    IntegrationTestCtx(const std::string_view& torrent_file_path);
    ~IntegrationTestCtx();
};