// Copyright (c) Alex Li 2025
// OthelloRules.cpp
// Implementations of functions defined in OthelloRules.hpp

#include <vector>
#include <utility>            // for std::pair
#include "othello/OthelloRules.hpp"
#include "othello/Constants.hpp"      // for bitboard constants
#include "othello/GameBoard.hpp"      // for GameBoard and Color
#include "utils/Visualize.hpp" // for boardToString

namespace othello {

  
  std::vector<int> get_possible_moves(const GameBoard &b, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;

    std::vector<int> possible_moves;
    uint64_t empty_squares = ~(my_board | op_board);

    // Check West first
    // West = bit-shift right
    
    // this will get all frontier discs
    uint64_t temp = (my_board ) >> 1 & op_board;
    while (temp) {
      // shift left
      temp = (temp & ~othello::LEFT_EDGE_MASK) >> 1;
      // check if we are on any empty squares
      if (uint64_t empty = temp & empty_squares) {
        // we have valid moves
        while (empty) {
          // get the position of the first set bit
          int position = __builtin_ctzll(empty);
          possible_moves.push_back(position);
          // set the bit to 0
          empty &= empty - 1;
        }
      }
      // prepare for next iteration; & with op_board
      temp &= op_board;
    }
    return possible_moves;
  }

  bool is_valid_move(const GameBoard &b, int position, Color color) {
    // TODO: Implement
    return false;
  }

  bool is_terminal(const GameBoard &b) {
    // TODO: Implement
    return false;
  }

  std::pair<int, int> count_discs(const GameBoard &b){
    std::pair<int, int> disc_count;
    // TODO: Implement
    return disc_count;
  }

  GameBoard apply_move(const GameBoard &b, int position, Color color) {
    // TODO: Implement
    return b;
  }
}
