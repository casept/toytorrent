#pragma once

/*
 * This file contains small constants that multiple modules
 * which are otherwise unrelated need to know.
 * This is to prevent a compile-time explosion due to every
 * header (indirectly) including every other header.
 */
#include <cstdint>

namespace tt::piece {
const std::size_t Piece_Hash_Len = 20;
}