// Copyright (c) 2025 Alex Li
// Engine.hpp
// Game engine class for Othello. Performs negamax search with alpha-beta
// pruning.

#pragma once

#include "Evaluator.hpp"
#include "GameBoard.hpp" // For GameBoard
#include <map>           // For std::map

namespace othello {
/// @brief Represents a transposition table entry
/// @details score is stored as an integer with positive values for black
/// winning and negative values for white winning.
struct TTEntry {
  int score; ///< The score of the position
  int depth; ///< The depth at which the position was evaluated
};

/// @brief Represents the game engine for Othello
/// @details The engine performs a negamax search with alpha-beta pruning to
/// find the best move.
class Engine {
public:
  /// @brief Finds the best move for the current player
  /// @param depth The search depth for the negamax algorithm
  /// @return The index  of the best move found
  std::pair<int, int> findBestMove(const GameBoard &board, int max_depth,
                                   Color color, const Evaluator &evaluator);

private:
  /// @brief Negamax search algorithm with alpha-beta pruning
  /// @param board Current game board
  /// @param depth Current search depth
  /// @param alpha Alpha value.
  /// @param beta Beta value.
  /// @param is_maximizer True if the current player is the maximizer
  /// @param evaluator The evaluator to use for scoring the board
  /// @return Pair of (score, move index)
  std::pair<int, int> negamax(const GameBoard &board, int depth, int alpha,
                              int beta, bool is_maximizer,
                              const Evaluator &evaluator);

  std::map<uint64_t, TTEntry>
      transposition_table; ///< Transposition table for storing previously
                           ///< evaluated positions
};

} // namespace othello
