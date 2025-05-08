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

  // Forward declaration
  void check_empty(uint64_t temp, uint64_t empty_squares, std::vector<int> &possible_moves);

  std::vector<int> get_possible_moves(const GameBoard &b, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;

    std::vector<int> possible_moves;
    uint64_t empty_squares = ~(my_board | op_board);

    // Check West first
    // West = bit-shift right 1
    // this will get all frontier discs
    uint64_t temp = (my_board & ~othello::LEFT_EDGE_MASK) >> 1 & op_board;
    while (temp) {
      // shift right
      temp = (temp & ~othello::LEFT_EDGE_MASK) >> 1;
      // check if we are on any empty squares
      check_empty(temp, empty_squares, possible_moves);
      // prepare for next iteration; & with op_board
      temp &= op_board;
    }
    // Check East
    // East = bit-shift left 1
    temp = (my_board & ~othello::RIGHT_EDGE_MASK) << 1 & op_board;
    while (temp) {
      temp = (temp & ~othello::RIGHT_EDGE_MASK) << 1;
      check_empty(temp, empty_squares, possible_moves);
      temp &= op_board;
    }
    // Check North
    // North = bit-shift right 8
    temp = (my_board & ~othello::TOP_EDGE_MASK) >> 8 & op_board;
    while (temp) {
      temp = (temp & ~othello::TOP_EDGE_MASK) >> 8;
      check_empty(temp, empty_squares, possible_moves);
      temp &= op_board;
    }
    // Check South
    // South = bit-shift left 8
    temp = (my_board & ~othello::BOTTOM_EDGE_MASK) << 8 & op_board;
    while (temp) {
      temp = (temp & ~othello::BOTTOM_EDGE_MASK) << 8;
      check_empty(temp, empty_squares, possible_moves);
      temp &= op_board;
    }
    // Check North-West
    // North-West = bit-shift right 9
    temp = (my_board & ~othello::LEFT_EDGE_MASK & ~othello::TOP_EDGE_MASK) >> 9 & op_board;
    while (temp) {
      temp = (temp & ~othello::LEFT_EDGE_MASK & ~othello::TOP_EDGE_MASK) >> 9;
      check_empty(temp, empty_squares, possible_moves);
      temp &= op_board;
    }
    // Check North-East
    // North-East = bit-shift left 7
    temp = (my_board & ~othello::RIGHT_EDGE_MASK & ~othello::TOP_EDGE_MASK) << 7 & op_board;
    while (temp) {
      temp = (temp & ~othello::RIGHT_EDGE_MASK & ~othello::TOP_EDGE_MASK) << 7;
      check_empty(temp, empty_squares, possible_moves);
      temp &= op_board;
    }
    // Check South-West
    // South-West = bit-shift right 7
    temp = (my_board & ~othello::LEFT_EDGE_MASK & ~othello::BOTTOM_EDGE_MASK) >> 7 & op_board;
    while (temp) {
      temp = (temp & ~othello::LEFT_EDGE_MASK & ~othello::BOTTOM_EDGE_MASK) >> 7;
      check_empty(temp, empty_squares, possible_moves);
      temp &= op_board;
    }
    // Check South-East
    // South-East = bit-shift left 9
    temp = (my_board & ~othello::RIGHT_EDGE_MASK & ~othello::BOTTOM_EDGE_MASK) << 9 & op_board;
    while (temp) {
      temp = (temp & ~othello::RIGHT_EDGE_MASK & ~othello::BOTTOM_EDGE_MASK) << 9;
      check_empty(temp, empty_squares, possible_moves);
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

  /// @brief Check if given bitboard has empty squares.
  ///        If so, add all empty squares to possible_moves.
  /// @param temp The bitboard to check for empty squares
  /// @param empty_squares The bitboard representing all empty squares
  /// @param possible_moves The vector to add empty squares to
  void check_empty(uint64_t temp, uint64_t empty_squares, std::vector<int> &possible_moves) {
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
  }
}
