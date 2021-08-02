#include "../torrent/torrent.hpp"

#include <bits/stdint-uintn.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <string>

#include "../torrent/metainfo.hpp"
#include "../torrent/peer.hpp"
#include "../torrent/tracker.hpp"
#include "helpers.hpp"

using namespace tt;

static std::string random_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

TEST_F(IntegrationTest, torrent_download_piece) {
    // Setup
    const auto info = metainfo_from_path(Torrent_File_Path);
    const std::uint16_t us_port = 12345;
    const auto download_path = std::filesystem::temp_directory_path().append(random_string(32)).string();

    // Test
    Torrent t{info, us_port, download_path};
    // TODO: Write helper for actually downloading piece

    ASSERT_EQ(true, false);
}