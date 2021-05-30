#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace tt {
// TODO: Implement multifile torrents

// What kind of download the metainfo describes.
enum class DownloadType { SingleFile, Directory };

// The length of a piece SHA1 hash in bytes
constexpr size_t Piece_SHA1_Len = 20;

class MetaInfo {
   private:
    // Used for computing the infohash
    std::vector<char> m_bencoded_info;

   public:
    // Primary tracker URL embedded in this metainfo.
    std::string m_primary_tracker_url;
    // Whether we're downloading a file or directory
    DownloadType m_download_type;
    // The file/directory name the metainfo file suggests we store the data under
    std::string m_suggested_name;
    // The length of a single piece in bytes. Note that the last piece in a file may be smaller.
    std::int64_t m_piece_length;
    // The length of the file in bytes. Will only contain a value if m_download_type is SingleFile.
    std::optional<std::int64_t> m_file_length;
    // Mapping of piece indices to their SHA1 hashes.
    // The piece index is the index into the vector.
    std::vector<std::array<char, Piece_SHA1_Len>> m_pieces;

    // Parses the data in the queue into a MetaInfo instance
    MetaInfo(std::deque<char> in);
    // Computes the infohash for this MetaInfo.
    std::string infohash() const;
    // Computes the infohash for this MetaInfo, truncated to 20 bytes.
    std::string truncated_infohash() const;
};
}  // namespace tt