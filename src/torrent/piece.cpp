#include "piece.hpp"

#include <botan-2/botan/hash.h>
#include <botan-2/botan/hex.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <array>
#include <exception>
#include <fstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../log.hpp"
#include "shared_constants.hpp"

namespace tt::piece {
Piece::Piece(const std::uint32_t size, const std::uint32_t idx,
             const std::array<std::uint8_t, Piece_Hash_Len> expected_hash, const State state)
    : m_state(state), m_size(size), m_idx(idx), m_expected_hash(expected_hash), m_subpieces({}) {
    auto num_subpieces = size / peer::Request_Subpiece_Size;
    if (size % peer::Request_Subpiece_Size != 0) {
        num_subpieces += 1;
    }
    this->m_subpieces.resize(num_subpieces);
    const std::optional<std::vector<std::uint8_t>>& nothing = {};
    std::fill(this->m_subpieces.begin(), this->m_subpieces.end(), nothing);
}

std::array<std::uint8_t, Piece_Hash_Len> Piece::get_curr_hash() {
    // Iterate over subpieces and collect into vector
    std::vector<std::uint8_t> data{};
    for (auto& opt_subpiece : this->m_subpieces) {
        if (opt_subpiece.has_value()) {
            const std::vector<std::uint8_t> subpiece = opt_subpiece.value();
            data.resize(data.size() + subpiece.size());
            data.insert(data.end(), subpiece.begin(), subpiece.end());
        }
    }
    // Compute hash
    auto hasher = Botan::HashFunction::create_or_throw("SHA1");
    hasher->update(data);
    auto hash_vec{hasher->final_stdvec()};
    std::array<std::uint8_t, Piece_Hash_Len> hash{};
    std::copy_n(hash_vec.begin(), Piece_Hash_Len, hash.begin());
    return hash;
}

std::string Piece::get_expected_hash_str() {
    return Botan::hex_encode(this->m_expected_hash.data(), this->m_expected_hash.size(), true);
}

std::string Piece::get_curr_hash_str() {
    const auto hash = this->get_curr_hash();
    return Botan::hex_encode(hash.data(), hash.size(), true);
}

bool Piece::hashes_match() {
    auto curr_hash{this->get_curr_hash()};
    return curr_hash == this->m_expected_hash;
}

void Piece::set_downloaded_subpiece_data(const std::size_t subpiece_idx, const std::vector<std::uint8_t>& data) {
    this->m_subpieces.at(subpiece_idx) = {data};
}

void Piece::flush_to_disk(std::shared_ptr<std::fstream> f) {
    // Seek into position
    f->exceptions(std::fstream::failbit | std::fstream::badbit | std::fstream::eofbit);
    try {
        f->seekp(this->m_idx * this->m_size);
    } catch (const std::ios_base::failure& e) {
        throw std::runtime_error(fmt::format("Piece::flush_to_disk() failed: Failed to seek (Reason: {})", e.what()));
    }

    // Dump data
    try {
        for (auto& opt_subpiece : this->m_subpieces) {
            if (opt_subpiece.has_value()) {
                auto& subpiece = opt_subpiece.value();
                f->write(reinterpret_cast<char*>(subpiece.data()),
                         static_cast<std::streamsize>(subpiece.size() * sizeof(char)));
            }
        }
    } catch (const std::ios_base::failure& e) {
        throw std::runtime_error(fmt::format("Piece::flush_to_disk() failed: Failed to write (Reason: {})", e.what()));
    }
}

Map::Map(std::vector<Piece> pieces) : m_pieces({}) {
    for (auto piece : pieces) {
        m_pieces.push_back(std::make_shared<Piece>(piece));
    }
}

std::shared_ptr<Piece> Map::get_piece(const std::size_t index) { return this->m_pieces.at(index); }

PieceVerificationJob::PieceVerificationJob(std::shared_ptr<piece::Piece> p) : m_piece(std::move(p)){};

void PieceVerificationJob::process() {
    if (m_piece->hashes_match()) {
        m_piece->m_state = piece::State::HaveVerified;
    } else {
        const auto msg{fmt::format("Failed to verify piece hash: expected {}, got {}", m_piece->get_expected_hash_str(),
                                   m_piece->get_curr_hash_str())};
        log::log(log::Level::Warning, log::Subsystem::Torrent, msg);
    }
}

PieceFlushJob::PieceFlushJob(std::shared_ptr<piece::Piece> p, std::shared_ptr<std::fstream> file)
    : m_piece(std::move(p)), m_file(std::move(file)){};

void PieceFlushJob::process() {
    if (m_piece->m_state != piece::State::HaveVerified) {
        throw std::runtime_error("PieceFlushJob::process(): Piece not verified");
    }
    m_piece->flush_to_disk(m_file);
}
}  // namespace tt::piece
