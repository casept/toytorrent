#include "piece.hpp"

#include <botan-2/botan/hash.h>
#include <botan-2/botan/hex.h>
#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "shared_constants.hpp"

namespace tt::piece {
Piece::Piece(const std::uint32_t size, const std::uint32_t idx,
             const std::array<std::uint8_t, Piece_Hash_Len> expected_hash)
    : m_size(size), m_idx(idx), m_expected_hash(expected_hash) {}

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

void Piece::flush_to_disk(std::fstream& f) {
    // Seek into position
    f.exceptions(std::fstream::failbit | std::fstream::badbit | std::fstream::eofbit);
    try {
        f.seekp(this->m_idx * this->m_size);
    } catch (const std::ios_base::failure& e) {
        throw std::runtime_error(fmt::format("Piece::flush_to_disk() failed: Failed to seek (Reason: {})", e.what()));
    }

    // Dump data
    try {
        for (auto& opt_subpiece : this->m_subpieces) {
            if (opt_subpiece.has_value()) {
                auto& subpiece = opt_subpiece.value();
                f.write(reinterpret_cast<char*>(subpiece.data()), subpiece.size() * sizeof(char));
            }
        }
    } catch (const std::ios_base::failure& e) {
        throw std::runtime_error(fmt::format("Piece::flush_to_disk() failed: Failed to write (Reason: {})", e.what()));
    }
}

Map::Map(std::vector<Piece> pieces) : m_pieces(pieces) {}
Piece& Map::get_piece(const std::size_t index) { return this->m_pieces.at(index); }
std::optional<std::reference_wrapper<Piece>> Map::get_best_wanted_piece() {
    for (auto& piece : this->m_pieces) {
        if (piece.m_state == State::Want) {
            return {piece};
        }
    }
    return {};
}
}  // namespace tt::piece
