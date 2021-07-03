#include "helpers.hpp"

#include <fmt/core.h>

#include <boost/process.hpp>
#include <chrono>
#include <string>
#include <string_view>

namespace bp = boost::process;
TorrentSwarmTestCtx::TorrentSwarmTestCtx(const std::string_view& torrent_file_path,
                                         const std::string_view& torrent_data_dir_path) {
    // Silence
    bp::std_out.null();
    bp::std_in.null();

    // Spawn 5 peers
    const std::uint16_t initial_port = 4096;
    for (auto i = 0; i < 5; i++) {
        const auto port = initial_port + i;
        this->m_aria2c_peer_ports.push_back(port);
        this->m_aria2c_peers.emplace_back(
            bp::child(bp::search_path("aria2c"),
                      bp::args({"--bt-external-ip=127.0.0.1", "--bt-hash-check-seed=false", "--check-integrity=false",
                                "--seed-ratio=0.0", "--quiet", "--enable-dht=false", "--enable-dht6=false",
                                "--listen-port", std::to_string(port), "--dir", std::string(torrent_data_dir_path),
                                std::string(torrent_file_path)})));
    }
    // Wait for peers to get up
    // FIXME: This should be dynamic
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20s);
}

TorrentSwarmTestCtx::~TorrentSwarmTestCtx() {
    for (auto& aria2c : this->m_aria2c_peers) {
        aria2c.terminate();
    }
}

TrackerTestCtx::TrackerTestCtx() : m_opentracker(bp::search_path("opentracker"), bp::args({"-p", "6969"})) {}

TrackerTestCtx::~TrackerTestCtx() { this->m_opentracker.terminate(); }

IntegrationTestCtx::IntegrationTestCtx(const std::string_view& torrent_file_path,
                                       const std::string_view& torrent_data_dir_path)
    : m_have_handshaked(false), m_tracker(TrackerTestCtx()), m_swarm({torrent_file_path, torrent_data_dir_path}) {}

void IntegrationTest::SetUpTestSuite() {
    IntegrationTest::m_ctx = new IntegrationTestCtx(Torrent_File_Path, Torrent_Data_Dir);
}

void IntegrationTest::TearDownTestSuite() { delete m_ctx; }