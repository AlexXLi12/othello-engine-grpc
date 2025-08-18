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
inline constexpr uint64_t EDGE_MASK        =  0xFF818181818181FFULL; // Mask for the edges of the board
inline constexpr uint64_t LEFT_EDGE_MASK   = ~0x101010101010101ULL; // Reverse mask for the left edge of the board
inline constexpr uint64_t RIGHT_EDGE_MASK  = ~0x8080808080808080ULL; // Reverse mask for the right edge of the board
inline constexpr uint64_t TOP_EDGE_MASK    = ~0x00000000000000FFULL; // Reverse mask for the top edge of the board
inline constexpr uint64_t BOTTOM_EDGE_MASK = ~0xFF00000000000000ULL; // Reverse mask for the bottom edge of the board

// Special board position masks
inline constexpr uint64_t CORNER_MASK     = 0x8100000000000081ULL; // Mask for the corners of the board
inline constexpr uint64_t X_SQUARE_MASK   = 0x4200000000420000ULL; // Mask for the X squares (9, 14, 49, 54)
inline constexpr uint64_t C_SQUARE_MASK   = 0x4281000000008142ULL; // Mask for the C squares (1, 6, 8, 15, 48, 55, 57, 62)
inline constexpr uint64_t A_SQUARE_MASK   = 0x4281000000008142ULL; // Mask for the A squares (2, 5, 16, 23, 40, 47, 58, 61)
inline constexpr uint64_t B_SQUARE_MASK   = 0x1800008181000018ULL; // Mask for the B squares (3, 4, 24, 31, 32, 39, 59, 60)
inline constexpr uint64_t MID_SQUARE_MASK = 0x0000001818000000ULL; // Mask for the middle 4 squares (27, 28, 35, 36)

// Engine constants
inline constexpr size_t TT_INITIAL_SIZE = 1000000; // Initial size of the transposition table

}  // namespace othello

