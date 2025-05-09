// Copyright (c) 2025 Alex Li
// OthelloRules.hpp
// Performs logic for Othello game rules

#ifndef RULES_HPP
#define RULES_HPP

#include <vector>
#include <utility> // For std::pair
#include "othello/GameBoard.hpp"

namespace othello {
  
  /// @brief Return a vector of all possible move positions for the given color 
  /// @param b The game board to check for possible moves
  /// @param color The color whose possible moves to check
  /// @return A bitboard representing the possible moves
  uint64_t get_possible_moves(const GameBoard &b, Color color);

  /// @brief Return whether a move is valid for the given color
  /// @param b The game board to check for the move
  /// @param position The position of the move to check
  /// @param color The color of the player making the move
  /// @return true if the move is valid, false otherwise
  bool is_valid_move(const GameBoard &b, int position, Color color);

  /// @brief Return whether the game is over
  /// @param b The game board to check for game over condition
  /// @return true if the game is over, false otherwise
  bool is_terminal(const GameBoard &b);

  /// @brief  Return the disc count for each color
  /// @param b The game board to check for disc counts
  /// @return std::pair<int, int> representing the count of black and white discs
  ///         respectively.
  std::pair<int, int> count_discs(const GameBoard &b);

  /// @brief Apply the move to the game board and return a new game board
  /// @details This function assumes that the move is valid.
  ///          Passing in an invalid move will result in undefined behavior.
  /// @param b The game board to apply the move to
  /// @param position The position of the move to apply
  /// @param color The color of the player making the move
  /// @return A new GameBoard with the move applied
  GameBoard apply_move(const GameBoard &b, int position, Color color);
}

#endif  // RULES_HPP
