// Copyright (c) 2025 Alex Li
// Engine.hpp
// Game engine class for Othello. Performs negamax search with alpha-beta
// pruning.

#pragma once

#include <unordered_map>  // For std::map

#include "GameBoard.hpp"  // For GameBoard
#include "evaluator/Evaluator.hpp"

namespace othello {

/// @brief Represents the type of bound for a transposition table entry
/// @details This enum is used to indicate whether the score is an exact score,
/// a lower bound, or an upper bound.
enum class BoundType {
  EXACT,  ///< Exact score
  LOWER,  ///< Lower bound
  UPPER   ///< Upper bound
};

/// @brief Represents a transposition table entry
/// @details score is stored as an integer with positive values for black
/// winning and negative values for white winning.
struct TTEntry {
  int score;             ///< The score of the position
  int depth;             ///< The depth at which the position was evaluated
  BoundType bound_type;  ///< The type of bound (exact, lower, upper)
  int move_index;        ///< The index of the move that led to this position
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
  /// @param prev_passed Whether the previous player passed their turn
  /// @param time_limit_ms The time limit for the search in milliseconds
  /// @return The index  of the best move found or -1 if
  ///         no valid moves are available.
  int findBestMove(const GameBoard &board, int max_depth, Color color,
                   int time_limit_ms);

 private:

  int nodesSearched = 0;  ///< Number of nodes

  int cacheHits = 0;  ///< Number of cache hits in the transposition table

  /// @brief Negamax search algorithm with alpha-beta pruning
  /// @param board Current game board
  /// @param depth Current search depth
  /// @param alpha Alpha value.
  /// @param beta Beta value.
  /// @param color The color of the player to move
  /// @return Pair of (score, move index)
  std::pair<int, int> negamax(const GameBoard &board, int depth, int alpha,
                              int beta, Color color);

  /// Transposition table for storing previously evaluated positions
  std::unordered_map<uint64_t, TTEntry> transposition_table;

  /// The evaluator to use for scoring the board
  const Evaluator &evaluator;
};

}  // namespace othello

