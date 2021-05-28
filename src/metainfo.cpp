#include "metainfo.h"

#include <botan-2/botan/hash.h>
#include <botan-2/botan/hex.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <map>

#include "bencode_parser.h"

MetaInfo::MetaInfo(std::deque<char> in) {
    // metainfo files are basically just a giant dictionary
    auto top_level_parser = BEncodeParser(in);
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
    } else {
        throw std::runtime_error{"Torrent metainfo file must contain 'length' or 'files' key"};
    }

    // Create a mapping between piece indices and their hashes
    m_pieces = {};
    // All the hashes are one long string rather than a list of smaller ones
    auto pieces = info_dict.find("pieces")->second.str.value();
    if ((pieces.length() % sha1_len) != 0) {
        throw std::runtime_error{"Pieces list must only contain whole hashes"};
    }
    int64_t num_pieces = pieces.length() / sha1_len;
    for (std::int64_t i = 0; i < num_pieces; i++) {
        std::array<char, sha1_len> hash{};
        std::copy_n(pieces.begin(), sha1_len, hash.begin());
        pieces.erase(0, sha1_len);
        m_pieces.push_back(hash);
    }
}

std::string MetaInfo::infohash() const {
    auto hasher = Botan::HashFunction::create_or_throw("SHA1");
    // const_cast should be fine here because the buffer isn't touched
    auto data = const_cast<char*>(this->m_bencoded_info.data());
    const auto hash_vec = hasher.get()->process(reinterpret_cast<uint8_t*>(data), this->m_bencoded_info.size());
    const auto hash = Botan::hex_encode(hash_vec, true);
    return hash;
}

std::string MetaInfo::truncated_infohash() const { return this->infohash().substr(0, 20); }