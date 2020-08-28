#include "tracker.h"
#include "bencode_parser.h"

#include <string>
#include <cstdint>
#include <deque>
#include <sstream>

#include <cpr/cpr.h>

TrackerCommunicator::TrackerCommunicator(std::string announce_url, std::uint32_t our_port, std::array<char, peer_id_length> our_peer_id, std::string infohash) {
    m_data_downloaded = {0};
    m_data_uploaded = {0};
    m_data_left = {0};
    m_announce_url = {announce_url};
    m_peer_id = {our_peer_id};
    m_port = {our_port};
    m_info_hash = {infohash};
}

void TrackerCommunicator::send_to_tracker(const std::string &event) {
    // TODO: Implement support for compact representation, because many trackers mandate it
 cpr::Parameters p = cpr::Parameters{
     {"info_hash", m_info_hash}, {"peer_id", std::string(m_peer_id.data())}, {"port", std::to_string(m_port)},
     {"uploaded", std::to_string(m_data_uploaded)}, {"downloaded", std::to_string(m_data_downloaded)},
     {"left", std::to_string(m_data_left)}, {"event", event}, {"compact", "0"}};
 cpr::Response r = cpr::Get(cpr::Url{m_announce_url}, p);
    auto queue = std::deque(r.text.begin(), r.text.end());
    auto parser = BEncodeParser(queue);
    auto resp_object = parser.next();

}

void TrackerCommunicator::send_completed() {
    send_to_tracker("completed");
}

void TrackerCommunicator::send_started() {
    send_to_tracker("started");
}

void TrackerCommunicator::send_stopped() {
    send_to_tracker("stopped");
}

void TrackerCommunicator::send_update() {
    send_to_tracker("empty");
}