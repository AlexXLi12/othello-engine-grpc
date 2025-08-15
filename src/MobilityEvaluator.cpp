// Copyright (c) 2025 Alex Li
// MobilityEvaluator.cpp
// Implementation of the MobilityEvaluator class for Othello game

#include "othello/Constants.hpp"
#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "othello/evaluator/Evaluator.hpp"

namespace othello {
int MobilityEvaluator::evaluate(const GameBoard &board) const {
  int score = 0;

  // Count discs in each region using bitboard masks
  score += 50 * (__builtin_popcountll(board.black_bb & CORNER_MASK) -
                 __builtin_popcountll(board.white_bb & CORNER_MASK));

  score -= 50 * (__builtin_popcountll(board.black_bb & X_SQUARE_MASK) -
                 __builtin_popcountll(board.white_bb & X_SQUARE_MASK));

  score -= 20 * (__builtin_popcountll(board.black_bb & C_SQUARE_MASK) -
                 __builtin_popcountll(board.white_bb & C_SQUARE_MASK));

  score += 10 * (__builtin_popcountll(board.black_bb & A_SQUARE_MASK) -
                 __builtin_popcountll(board.white_bb & A_SQUARE_MASK));

  score += 2 * (__builtin_popcountll(board.black_bb & B_SQUARE_MASK) -
                __builtin_popcountll(board.white_bb & B_SQUARE_MASK));

  score += 3 * (__builtin_popcountll(board.black_bb & MID_SQUARE_MASK) -
                __builtin_popcountll(board.white_bb & MID_SQUARE_MASK));

  // Evaluate mobility
  uint64_t black_mobility_score = 2 * getPossibleMoves(board, Color::BLACK);
  uint64_t white_mobility_score = 2 * getPossibleMoves(board, Color::WHITE);

  score +=
      __builtin_popcountll(black_mobility_score) - __builtin_popcountll(white_mobility_score);

  return score;
}
}  // namespace othello
// namespace othello
