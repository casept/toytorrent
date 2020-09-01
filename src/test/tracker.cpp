#include "../tracker.h"
#include <gtest/gtest.h>

/*
These are integration tests, and are designed to be run with a tracker reachable at a fixed address.
The easiest way to get them running is using the provided arion-compose.nix to create a docker network
with several different client implementations serving test torrents and an opentracker instance.
*/

TEST(TrackerCommunication, simple_announce) {
    // FIXME: This should run locally

}