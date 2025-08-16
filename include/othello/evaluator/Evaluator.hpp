// Copyright (c) 2025 Alex Li
// Evaluator.hpp
// Evaluator class for Othello game

#pragma once

#include "../GameBoard.hpp"

namespace othello {

/// @brief Evaluator class for Othello
/// @details This class defines an interface for evaluating the game board.
class Evaluator {
 public:
  /// @brief Evaluates the game board
  /// @param board The game board to evaluate
  /// @return The evaluation score of the board
  virtual int evaluate(const GameBoard &board) const = 0;

  /// @brief Destructor for the Evaluator class
  virtual ~Evaluator() = default;
};

/// @brief PositionalEvaluator class for Othello
/// @details This class implements an evaluation function for the game board
///          based solely on positional factors.
class PositionalEvaluator : public Evaluator {
 public:
  /// @brief Evaluates the game board
  /// @param board The game board to evaluate
  /// @return The evaluation score of the board
  int evaluate(const GameBoard &board) const override;
};

/// @brief MobilityEvaluator class for Othello
/// @details This class implements an evaluation function for the game board
///          based on positional factors and mobility heuristics.
class MobilityEvaluator : public Evaluator {
 public:
  /// @brief Evaluates the game board
  /// @param board The game board to evaluate
  /// @return The evaluation score of the board
  int evaluate(const GameBoard &board) const override;
};
}  // namespace othello

