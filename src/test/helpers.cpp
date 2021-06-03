#include "helpers.hpp"

#include <boost/process.hpp>
#include <string>
#include <string_view>

namespace bp = boost::process;
IntegrationTestCtx::IntegrationTestCtx(const std::string_view& torrent_file_path)
    : m_opentracker(bp::search_path("opentracker"), bp::args({"-p", "6969"})) {
    // Spawn 5 peers
    std::uint16_t initial_port = 4096;
    for (auto i = 0; i < 5; i++) {
        const auto port = initial_port + i;
        this->m_aria2c_peer_ports.push_back(port);
        this->m_aria2c_peers.emplace_back(
            bp::child(bp::search_path("aria2c"),
                      bp::args({"--bt-external-ip=127.0.0.1", "--bt-hash-check-seed=false", "--seed-ratio=0.0",
                                "--listen-port", std::to_string(port), std::string(torrent_file_path)})));
    }
}

IntegrationTestCtx::~IntegrationTestCtx() {
    this->m_opentracker.terminate();
    for (auto& aria2c : this->m_aria2c_peers) {
        aria2c.terminate();
    }
}