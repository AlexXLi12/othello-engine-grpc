// Copyright (c) 2025, Alex Li
// Visualize.hpp
// Utility functions for visualizing the Othello game board

#ifndef VISUALIZE_HPP
#define VISUALIZE_HPP

#include <iostream>
#include "othello/GameBoard.hpp" // For GameBoard struct

namespace othello {
  /// @brief Return a string representation of the game board
  /// @details 'X' represents a black piece, 'O' represents a white piece,
  ///          and '.' represents an empty square.
  /// @param board The GameBoard struct to visualize
  /// @return A string representation of the game board
  std::string boardToString(const GameBoard& board);

  /// @brief Return a string representation of a single bitboard
  /// @details '1' represents a piece, '0' represents an empty square.
  /// @param bb The bitboard to visualize
  /// @return A string representation of the bitboard
  std::string bitboardToString(uint64_t bb);
}

#endif // VISUALIZE_HPP
