// Copyright (c) 2025 Alex Li
// Constants.hpp
// Bitboard constants for Othello game

#pragma once

#include <cstdint>  // For uint64_t

namespace othello {

// Constants for starting positions of pieces
inline constexpr uint64_t INITIAL_BLACK = 0x810000000ULL; // Initial position for black pieces
inline constexpr uint64_t INITIAL_WHITE = 0x1008000000ULL; // Initial position for white pieces

// Bitboard masks for the edges of the board
inline constexpr uint64_t EDGE_MASK = 0xFF818181818181FFULL; // Mask for the edges of the board
inline constexpr uint64_t CORNER_MASK = 0x8100000000000081ULL; // Mask for the corners of the board
inline constexpr uint64_t LEFT_EDGE_MASK = ~0x101010101010101ULL; // Reverse mask for the left edge of the board
inline constexpr uint64_t RIGHT_EDGE_MASK = ~0x8080808080808080ULL; // Reverse mask for the right edge of the board
inline constexpr uint64_t TOP_EDGE_MASK = ~0xFF00000000000000ULL; // Reverse mask for the top edge of the board
inline constexpr uint64_t BOTTOM_EDGE_MASK = ~0x00000000000000FFULL; // Reverse mask for the bottom edge of the board

}  // namespace othello

