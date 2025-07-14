// Copyright (c) 2025 Alex Li
// Engine.hpp
// Game engine class for Othello. Performs negamax search with alpha-beta
// pruning.

#pragma once

#include <map>  // For std::map

#include "GameBoard.hpp"  // For GameBoard
#include "evaluator/Evaluator.hpp"

namespace othello {
/// @brief Represents a transposition table entry
/// @details score is stored as an integer with positive values for black
/// winning and negative values for white winning.
struct TTEntry {
  int score;  ///< The score of the position
  int depth;  ///< The depth at which the position was evaluated
};

/// @brief Represents the game engine for Othello
/// @details The engine performs a negamax search with alpha-beta pruning to
/// find the best move.
class Engine final {
 public:
  /// @brief Constructor for Engine
  /// @param evaluator The evaluator to use for scoring the board
  Engine(const Evaluator &evaluator);
  /// @brief Finds the best move for the current player
  /// @param board The current game board
  /// @param max_depth The search depth for the negamax algorithm
  /// @param color The color of the player to move
  /// @param time_limit_ms The time limit for the search in milliseconds
  /// @return The index  of the best move found or -1 if
  ///         no valid moves are available.
  int findBestMove(const GameBoard &board, int max_depth, Color color,
                   int time_limit_ms);

 private:
  /// @brief Negamax search algorithm with alpha-beta pruning
  /// @param board Current game board
  /// @param depth Current search depth
  /// @param alpha Alpha value.
  /// @param beta Beta value.
  /// @param color The color of the player to move
  /// @return Pair of (score, move index)
  std::pair<int, int> negamax(const GameBoard &board, int depth, int alpha,
                              int beta, Color color);

  std::map<uint64_t, TTEntry>
      transposition_table;  ///< Transposition table for storing previously
                            ///< evaluated positions

  const Evaluator &evaluator;  ///< The evaluator to use for scoring the board
};

}  // namespace othello

