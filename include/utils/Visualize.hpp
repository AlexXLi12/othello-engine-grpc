// Copyright (c) 2025, Alex Li
// Visualize.hpp
// Utility functions for visualizing the Othello game board

#ifndef VISUALIZE_HPP
#define VISUALIZE_HPP

#include <string>

#include "../othello/GameBoard.hpp"  // For GameBoard struct

namespace othello {
/// @brief Return a string representation of the game board
/// @details 'X' represents a black piece, 'O' represents a white piece,
///          and '.' represents an empty square.
/// @param board The GameBoard struct to visualize
/// @return A string representation of the game board
std::string board_to_string(const GameBoard& board);

/// @brief Return a string representation of the game board with possible moves
/// marked.
/// @details 'X' represents a black piece, 'O' represents a white piece,
/// '.' represents an empty square, and '*' represents a possible
/// move.
/// @param board The GameBoard struct to visualize
/// @param possible_moves A bitboard representing the possible moves
/// @return A string representation of the game board with possible moves marked
std::string board_to_string_with_moves(const GameBoard& board, uint64_t possible_moves);

/// @brief Return a string representation of a single bitboard
/// @details '1' represents a piece, '0' represents an empty square.
/// @param bb The bitboard to visualize
/// @return A string representation of the bitboard
std::string bitboard_to_string(uint64_t bb);
}  // namespace othello

#endif  // VISUALIZE_HPP
