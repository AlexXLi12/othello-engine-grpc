#include "othello/evaluator/Evaluator.hpp"
#include "othello/GameBoard.hpp"
#include "othello/Constants.hpp"

namespace othello {
int PositionalEvaluator::evaluate(const GameBoard& board) const {
  int score = 0;

  // Count discs in each region using bitboard masks
  score += 200 * (__builtin_popcountll(board.black_bb & CORNER_MASK)
                - __builtin_popcountll(board.white_bb & CORNER_MASK));

  score -= 75 * (__builtin_popcountll(board.black_bb & X_SQUARE_MASK)
               - __builtin_popcountll(board.white_bb & X_SQUARE_MASK));

  score -= 75 * (__builtin_popcountll(board.black_bb & C_SQUARE_MASK)
               - __builtin_popcountll(board.white_bb & C_SQUARE_MASK));

  score += 24 * (__builtin_popcountll(board.black_bb & A_SQUARE_MASK)
               - __builtin_popcountll(board.white_bb & A_SQUARE_MASK)); // Python double-counted 12 + 12

  score += 2 * (__builtin_popcountll(board.black_bb & B_SQUARE_MASK)
             - __builtin_popcountll(board.white_bb & B_SQUARE_MASK));

  score += 3 * (__builtin_popcountll(board.black_bb & MID_SQUARE_MASK)
             - __builtin_popcountll(board.white_bb & MID_SQUARE_MASK));

  return score;
}
} // namespace othello

