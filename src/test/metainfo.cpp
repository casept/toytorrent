#include "../metainfo.h"
#include <gtest/gtest.h>

#include <deque>
#include <string>
#include <fstream>

#include <errno.h>
#include <string.h>

TEST(Metainfo, parse_single_file_torrent) {
    const std::string expected_announce {"http://explodie.org:6969/announce"};
    const auto expected_download_type {DownloadType::SingleFile};
    const std::string expected_destination_name {"lubuntu-16.04.2-desktop-amd64.iso"};
    const std::int64_t expected_file_length {922746880};
    const std::int64_t expected_piece_length {524288};
    const std::size_t expected_num_pieces {1760};

    
    const std::string torrent_path {"../src/test/testdata/lubuntu-16.04.torrent"};
    std::ifstream file{torrent_path};
    if (!file) {
        std::cerr << "The torrent file cannot be opened: " << strerror(errno) << std::endl;
        exit(1);
    }
    std::deque<char> data {};
    // TODO: Figure out how to use std::copy here
    while (!file.eof()) {
        data.push_back(file.get());
    }
    auto metainfo = MetaInfo(data);
    file.close();


    ASSERT_EQ(metainfo.m_primary_tracker_url, expected_announce);
    ASSERT_EQ(metainfo.m_download_type, expected_download_type);
    ASSERT_EQ(metainfo.m_suggested_name, expected_destination_name);
    ASSERT_TRUE(metainfo.m_file_length.has_value());
    ASSERT_EQ(metainfo.m_file_length.value(), expected_file_length);
    ASSERT_EQ(metainfo.m_piece_length, expected_piece_length);
    ASSERT_EQ(metainfo.m_pieces.size(), expected_num_pieces);
}

// TODO: Implement
TEST(Metainfo, parse_directory_torrent) {

}