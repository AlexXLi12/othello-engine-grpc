// Copyright (c) 2025, Alex Li
// BitboardUtils.hpp
// Utility functions for translating between bitboard and 0-63 coordinates

#pragma once

#include <cstdint>
#include <vector>

namespace othello {
/// @brief Convert a bitboard to a vector of positions
/// @param bitboard The bitboard to convert
/// @return A vector of positions (0-63) corresponding to the set bits in the bitboard
///         The positions are in the order of least significant bit to most significant bit.
inline std::vector<int> bitboard_to_positions(uint64_t bitboard) {
  std::vector<int> positions;
  while (bitboard) {
    int position = __builtin_ctzll(bitboard); // Get the index of the least significant bit
    positions.push_back(position);
    bitboard &= (bitboard - 1); // Clear the least significant bit
  }
  return positions;
}

/// @brief Convert a vector of positions (0-63) to a bitboard
/// @param std::vector<int> The positions to convert
/// @return A bitboard with the bit at the specified position set
inline uint64_t positions_to_bitboard(const std::vector<int>& positions) {
  uint64_t bitboard = 0;
  for (int position : positions) {
    bitboard |= (1ULL << position);
  }
  return bitboard;
}

}  // namespace othello

