// Copyright (c) Alex Li 2025
// OthelloRules.cpp
// Implements functions defined in OthelloRules.hpp

#include "othello/OthelloRules.hpp"
#include "othello/Constants.hpp" // for bitboard constants
#include "othello/GameBoard.hpp" // for GameBoard and Color

#include <utility> // for std::pair
#include <vector>

namespace othello {

uint64_t getPossibleMoves(const GameBoard &b, Color color) {
  uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
  uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;

  std::vector<int> possible_moves;
  uint64_t empty = ~(my_board | op_board);

  uint64_t moves = getDirectionalMoves(my_board, op_board, empty, -1,
                                       othello::LEFT_EDGE_MASK); // West
  moves |= getDirectionalMoves(my_board, op_board, empty, 1,
                               othello::RIGHT_EDGE_MASK); // East
  moves |= getDirectionalMoves(my_board, op_board, empty, 8,
                               othello::BOTTOM_EDGE_MASK); // South
  moves |= getDirectionalMoves(my_board, op_board, empty, -8,
                               othello::TOP_EDGE_MASK); // North
  moves |= getDirectionalMoves(my_board, op_board, empty, -7,
                               othello::TOP_EDGE_MASK &
                                   othello::RIGHT_EDGE_MASK); // North-East
  moves |= getDirectionalMoves(my_board, op_board, empty, -9,
                               othello::TOP_EDGE_MASK &
                                   othello::LEFT_EDGE_MASK); // North-West
  moves |= getDirectionalMoves(my_board, op_board, empty, 7,
                               othello::BOTTOM_EDGE_MASK &
                                   othello::LEFT_EDGE_MASK); // South-West
  moves |= getDirectionalMoves(my_board, op_board, empty, 9,
                               othello::BOTTOM_EDGE_MASK &
                                   othello::RIGHT_EDGE_MASK); // South-East
  return moves;
}

bool isValidMove(const GameBoard &b, int position, Color color) {
  uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
  uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;
  uint64_t empty = ~(my_board | op_board);
  if ((1ULL << position) & ~empty) {
    // position is not empty
    return false;
  }
  uint64_t pos_board = 1ULL << position;
  return getDirectionalMoves(my_board, op_board, empty, -1,
                             othello::LEFT_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, 1,
                             othello::RIGHT_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, -8,
                             othello::TOP_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, 8,
                             othello::BOTTOM_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, -9,
                             othello::TOP_EDGE_MASK & othello::LEFT_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, -7,
                             othello::TOP_EDGE_MASK &
                                 othello::RIGHT_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, 7,
                             othello::BOTTOM_EDGE_MASK &
                                 othello::LEFT_EDGE_MASK) &
             (pos_board) ||
         getDirectionalMoves(my_board, op_board, empty, 9,
                             othello::BOTTOM_EDGE_MASK &
                                 othello::RIGHT_EDGE_MASK) &
             (pos_board);
}

bool isTerminal(const GameBoard &b) {
  return getPossibleMoves(b, Color::BLACK) == 0 &&
         getPossibleMoves(b, Color::WHITE) == 0;
}

std::pair<int, int> countDiscs(const GameBoard &b) {
  std::pair<int, int> disc_count;
  uint64_t black_board = b.black_bb;
  uint64_t white_board = b.white_bb;
  disc_count.first = __builtin_popcountll(black_board);
  disc_count.second = __builtin_popcountll(white_board);
  return disc_count;
}

} // namespace othello

