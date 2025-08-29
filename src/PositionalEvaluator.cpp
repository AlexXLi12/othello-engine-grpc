// Copyright (c) 2025 Alex Li

#include <bit>

#include "othello/Constants.hpp"
#include "othello/GameBoard.hpp"
#include "othello/evaluator/Evaluator.hpp"

namespace othello {
int PositionalEvaluator::evaluate(const GameBoard &board) const {
  int score = 0;

  // Count discs in each region using bitboard masks
  score += 50 * (std::popcount(board.black_bb & CORNER_MASK) -
                 std::popcount(board.white_bb & CORNER_MASK));

  score -= 50 * (std::popcount(board.black_bb & X_SQUARE_MASK) -
                 std::popcount(board.white_bb & X_SQUARE_MASK));

  score -= 20 * (std::popcount(board.black_bb & C_SQUARE_MASK) -
                 std::popcount(board.white_bb & C_SQUARE_MASK));

  score += 10 * (std::popcount(board.black_bb & A_SQUARE_MASK) -
                 std::popcount(board.white_bb & A_SQUARE_MASK));

  score += 2 * (std::popcount(board.black_bb & B_SQUARE_MASK) -
                std::popcount(board.white_bb & B_SQUARE_MASK));

  score += 3 * (std::popcount(board.black_bb & MID_SQUARE_MASK) -
                std::popcount(board.white_bb & MID_SQUARE_MASK));

  score += (std::popcount(board.black_bb) - std::popcount(board.white_bb));

  return score;
}
} // namespace othello

