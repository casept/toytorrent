#include "../tracker.h"
#include "../metainfo.h"

#include <gtest/gtest.h>

/*
These are integration tests, and are designed to be run with a tracker reachable at a fixed address.
The easiest way to get them running is using the provided arion-compose.nix to create a docker network
with several different client implementations serving test torrents and an opentracker instance.
*/

// TODO: Load and parse the given torrent file and construct a tracker communicator for it.
//MetaInfo tracker_test_setup() {
    // Load torrent file
    // In order to allow running the test binary outside the source dir, the resources are embedded within it.
    

//}

TEST(TrackerCommunication, send_started) {

}


TEST(TrackerCommunication, send_stopped) {

}

TEST(TrackerCommunication, send_completed) {

}

TEST(TrackerCommunication, send_update) {

}