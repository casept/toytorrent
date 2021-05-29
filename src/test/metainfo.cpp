#include "../metainfo.hpp"

#include <errno.h>
#include <gtest/gtest.h>
#include <string.h>

#include <cmrc/cmrc.hpp>
#include <deque>
#include <fstream>
#include <string>

using namespace tt;

CMRC_DECLARE(test_resources);

TEST(Metainfo, parse_single_file_torrent) {
    const std::string expected_announce{"http://explodie.org:6969/announce"};
    const auto expected_download_type{DownloadType::SingleFile};
    const std::string expected_destination_name{"lubuntu-16.04.2-desktop-amd64.iso"};
    const std::int64_t expected_file_length{922746880};
    const std::int64_t expected_piece_length{524288};
    const std::size_t expected_num_pieces{1760};
    const std::string expected_infohash{"C1AA77DEA674D71FBD85559034B6082B8434D36E"};
    // The torrent file to test against is embedded in the executable
    auto fs = cmrc::test_resources::get_filesystem();
    const auto torrent_file = fs.open("lubuntu-16.04.torrent");
    std::deque<char> torrent_file_data_deque(torrent_file.begin(), torrent_file.end());
    auto metainfo = MetaInfo(torrent_file_data_deque);

    ASSERT_EQ(metainfo.m_primary_tracker_url, expected_announce);
    ASSERT_EQ(metainfo.m_download_type, expected_download_type);
    ASSERT_EQ(metainfo.m_suggested_name, expected_destination_name);
    ASSERT_TRUE(metainfo.m_file_length.has_value());
    ASSERT_EQ(metainfo.m_file_length.value(), expected_file_length);
    ASSERT_EQ(metainfo.m_piece_length, expected_piece_length);
    ASSERT_EQ(metainfo.m_pieces.size(), expected_num_pieces);
    ASSERT_EQ(metainfo.infohash(), expected_infohash);
}

// TODO: Implement
TEST(Metainfo, parse_directory_torrent) {}
