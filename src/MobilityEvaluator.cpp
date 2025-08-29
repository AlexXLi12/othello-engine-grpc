// Copyright (c) 2025 Alex Li
// MobilityEvaluator.cpp
// Implementation of the MobilityEvaluator class for Othello game

#include <bit>

#include "othello/Constants.hpp"
#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "othello/evaluator/Evaluator.hpp"

namespace othello {
int MobilityEvaluator::evaluate(const GameBoard &board) const {
  int score = 0;

  // Count discs in each region using bitboard masks
  score += 1000 * (std::popcount(board.black_bb & CORNER_MASK) -
                 std::popcount(board.white_bb & CORNER_MASK));

  score -= 300 * (std::popcount(board.black_bb & X_SQUARE_MASK) -
                 std::popcount(board.white_bb & X_SQUARE_MASK));
  //
  // score -= 50 * (std::popcount(board.black_bb & C_SQUARE_MASK) -
  //                std::popcount(board.white_bb & C_SQUARE_MASK));
  //
  // score += 10 * (std::popcount(board.black_bb & A_SQUARE_MASK) -
  //                std::popcount(board.white_bb & A_SQUARE_MASK));
  //
  // score += 2 * (std::popcount(board.black_bb & B_SQUARE_MASK) -
  //               std::popcount(board.white_bb & B_SQUARE_MASK));
  //
  // score += 3 * (std::popcount(board.black_bb & MID_SQUARE_MASK) -
  //               std::popcount(board.white_bb & MID_SQUARE_MASK));

  // Evaluate mobility
  uint64_t black_mobility_board = getPossibleMoves(board, Color::BLACK);
  uint64_t white_mobility_board = getPossibleMoves(board, Color::WHITE);

  score += 10 * (std::popcount(black_mobility_board) - std::popcount(white_mobility_board));

  score += 2 * (std::popcount(board.black_bb) - std::popcount(board.white_bb));

  return score;
}
}  // namespace othello

