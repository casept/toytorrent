#include "metainfo.hpp"

#include <bits/stdint-uintn.h>
#include <botan-2/botan/hash.h>
#include <botan-2/botan/hex.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "bencode.hpp"
#include "shared_constants.hpp"

static std::ifstream read_torrent_file(const std::string_view& path) {
    std::ifstream f{std::string(path), std::ios::in | std::ios::binary};
    if (!f) {
        throw std::runtime_error{"The torrent file cannot be opened"};
    }
    return f;
}

namespace tt {
MetaInfo metainfo_from_path(const std::string_view& path) {
    auto f = read_torrent_file(path);
    std::deque<char> data{};
    while (true) {
        const char byte = f.get();
        if (f.eof()) {
            break;
        }
        data.push_back(byte);
    }
    if (f.bad()) {
        throw std::runtime_error{"Failed to read .torrent file!"};
    }
    f.close();
    return tt::MetaInfo(data);
}

MetaInfo::MetaInfo(std::deque<char> in) {
    // metainfo files are basically just a giant dictionary
    auto top_level_parser = bencode::Parser(in);
    auto top_level_dict = top_level_parser.next().value().dict.value();

    // Primary tracker announce URL
    m_primary_tracker_url = top_level_dict.find("announce")->second.str.value();

    // info itself is also a dict
    auto info = top_level_dict.find("info")->second;
    auto info_dict = info.dict.value();
    // We need this to compute the infohash later
    m_bencoded_info = top_level_dict.find("info")->second.as_raw_bytes();
    // FIXME: Figure out why another e sneaks up on us here
    m_bencoded_info.pop_back();

    m_piece_length = info_dict.find("piece length")->second.integer.value();
    // TODO: Handle the directory case
    m_suggested_name = info_dict.find("name")->second.str.value();

    // If the dict contains "length", the torrent is a single file.
    // Otherwise, it has to contain "files" and is a directory.
    // Both at the same time are illegal.
    bool has_length = info_dict.count("length") == 1;
    bool has_files = info_dict.count("files") == 1;
    if (has_files && has_length) {
        throw std::runtime_error{"Torrent metainfo file may only contain one of the 'length' or 'files' keys"};
    }
    if (has_length) {
        m_download_type = DownloadType::SingleFile;
        m_file_length = info_dict.find("length")->second.integer;
    } else if (has_files) {
        m_download_type = DownloadType::Directory;
        // TODO:: Create file list
        throw std::runtime_error{"Directory torrents not yet supported!"};
    } else {
        throw std::runtime_error{"Torrent metainfo file must contain 'length' or 'files' key"};
    }

    // Create a mapping between piece indices and their hashes
    m_pieces = {};
    // All the hashes are one long string rather than a list of smaller ones
    auto pieces = info_dict.find("pieces")->second.str.value();
    if ((pieces.length() % piece::Piece_Hash_Len) != 0) {
        throw std::runtime_error{"Pieces list must only contain whole hashes"};
    }
    int64_t num_pieces = pieces.length() / piece::Piece_Hash_Len;
    for (std::int64_t i = 0; i < num_pieces; i++) {
        std::array<std::uint8_t, piece::Piece_Hash_Len> hash{};
        std::copy_n(pieces.begin(), piece::Piece_Hash_Len, hash.begin());
        pieces.erase(0, piece::Piece_Hash_Len);
        m_pieces.push_back(hash);
    }
}

std::vector<uint8_t> MetaInfo::infohash_binary() const {
    auto hasher = Botan::HashFunction::create_or_throw("SHA1");
    // Copy to avoid casting away const
    std::vector<uint8_t> copy{};
    for (char byte : this->m_bencoded_info) {
        copy.push_back(static_cast<uint8_t>(byte));
    }
    // Hash
    auto hasher_real = hasher.get();
    hasher_real->update(copy);
    auto hash = hasher_real->final_stdvec();
    return hash;
}

std::string MetaInfo::infohash() const { return Botan::hex_encode(this->infohash_binary(), false); }

std::vector<std::uint8_t> MetaInfo::truncated_infohash_binary() const {
    std::vector<std::uint8_t> trunc{};
    trunc.reserve(20);
    trunc.resize(20);
    std::copy_n(this->infohash_binary().begin(), 20, trunc.begin());
    return trunc;
}

std::string MetaInfo::truncated_infohash() const { return this->infohash().substr(0, 20); }

std::size_t MetaInfo::total_size() const {
    if (!this->m_file_length.has_value()) {
        throw std::runtime_error{"Failed to get size of torrent file: length key was None"};
    }
    return this->m_file_length.value();
}
}  // namespace tt
