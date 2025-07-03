// Copyright (c) 2025 Alex Li
// GameBoard.cpp
// Implements functions defined in GameBoard.hpp

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp" // For isValidMove

namespace othello {
GameBoard applyMove(const GameBoard &b, int position, Color color) {
  uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
  uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;
  uint64_t empty = ~(my_board | op_board);
  uint64_t pos_board = 1ULL << position;
  uint64_t flips = getDirectionalFlips(pos_board, my_board, op_board, empty, -1,
                                       othello::LEFT_EDGE_MASK); // West
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, 1,
                               othello::RIGHT_EDGE_MASK); // East
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, 8,
                               othello::BOTTOM_EDGE_MASK); // South
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, -8,
                               othello::TOP_EDGE_MASK); // North
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, -7,
                               othello::TOP_EDGE_MASK &
                                   othello::RIGHT_EDGE_MASK); // North-East
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, -9,
                               othello::TOP_EDGE_MASK &
                                   othello::LEFT_EDGE_MASK); // North-West
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, 7,
                               othello::BOTTOM_EDGE_MASK &
                                   othello::LEFT_EDGE_MASK); // South-West
  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, 9,
                               othello::BOTTOM_EDGE_MASK &
                                   othello::RIGHT_EDGE_MASK); // South-East
  my_board = my_board | pos_board | flips;
  op_board ^= flips;
  if (color == Color::BLACK) {
    return GameBoard(my_board, op_board);
  } else {
    return GameBoard(op_board, my_board);
  }
}
} // namespace othello

