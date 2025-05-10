// Copyright (c) Alex Li 2025
// OthelloRules.cpp
// Implementations of functions defined in OthelloRules.hpp
#include <vector>
#include <utility>            // for std::pair
#include "othello/OthelloRules.hpp"
#include "othello/Constants.hpp"      // for bitboard constants
#include "othello/GameBoard.hpp"      // for GameBoard and Color

namespace othello {

  uint64_t get_possible_moves(const GameBoard &b, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;

    std::vector<int> possible_moves;
    uint64_t empty = ~(my_board | op_board);

    uint64_t moves = get_directional_moves(my_board, op_board, empty, -1, othello::LEFT_EDGE_MASK); // West
    moves |= get_directional_moves(my_board, op_board, empty, 1, othello::RIGHT_EDGE_MASK);   // East
    moves |= get_directional_moves(my_board, op_board, empty, 8, othello::BOTTOM_EDGE_MASK);         // South
    moves |= get_directional_moves(my_board, op_board, empty, -8, othello::TOP_EDGE_MASK);        // North
    moves |= get_directional_moves(my_board, op_board, empty, -7, othello::TOP_EDGE_MASK & othello::RIGHT_EDGE_MASK); // North-East
    moves |= get_directional_moves(my_board, op_board, empty, -9, othello::TOP_EDGE_MASK & othello::LEFT_EDGE_MASK);  // North-West
    moves |= get_directional_moves(my_board, op_board, empty, 7, othello::BOTTOM_EDGE_MASK & othello::LEFT_EDGE_MASK); // South-West
    moves |= get_directional_moves(my_board, op_board, empty, 9, othello::BOTTOM_EDGE_MASK & othello::RIGHT_EDGE_MASK); // South-East
    return moves;
  }

  bool is_valid_move(const GameBoard &b, int position, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;
    uint64_t empty = ~(my_board | op_board);
    if ((1ULL << position) & ~empty) {
       // position is not empty
      return false;
    }
    uint64_t pos_board = 1ULL << position;
    return get_directional_moves(my_board, op_board, empty, -1, othello::LEFT_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, 1, othello::RIGHT_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, -8, othello::TOP_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, 8, othello::BOTTOM_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, -9, othello::TOP_EDGE_MASK & othello::LEFT_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, -7, othello::TOP_EDGE_MASK & othello::RIGHT_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, 7, othello::BOTTOM_EDGE_MASK & othello::LEFT_EDGE_MASK) & (pos_board) ||
           get_directional_moves(my_board, op_board, empty, 9, othello::BOTTOM_EDGE_MASK & othello::RIGHT_EDGE_MASK) & (pos_board);
  }

  bool is_terminal(const GameBoard &b) {
      return get_possible_moves(b, Color::BLACK) == 0 &&
             get_possible_moves(b, Color::WHITE) == 0;
  }

  std::pair<int, int> count_discs(const GameBoard &b){
    std::pair<int, int> disc_count;
    uint64_t black_board = b.black_bb;
    uint64_t white_board = b.white_bb;
    disc_count.first = __builtin_popcountll(black_board);
    disc_count.second = __builtin_popcountll(white_board);
    return disc_count;
  }

  GameBoard apply_move(const GameBoard &b, int position, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;
    uint64_t empty = ~(my_board | op_board);
    uint64_t pos_board = 1ULL << position;
    uint64_t flips = get_directional_flips(pos_board, my_board, op_board, empty, -1, othello::LEFT_EDGE_MASK);  // West
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, 1, othello::RIGHT_EDGE_MASK);   // East
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, 8, othello::BOTTOM_EDGE_MASK);         // South
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, -8, othello::TOP_EDGE_MASK);        // North
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, -7, othello::TOP_EDGE_MASK & othello::RIGHT_EDGE_MASK); // North-East
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, -9, othello::TOP_EDGE_MASK & othello::LEFT_EDGE_MASK);  // North-West
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, 7, othello::BOTTOM_EDGE_MASK & othello::LEFT_EDGE_MASK); // South-West
    flips |= get_directional_flips(pos_board, my_board, op_board, empty, 9, othello::BOTTOM_EDGE_MASK & othello::RIGHT_EDGE_MASK); // South-East
    my_board = my_board | pos_board | flips;
    op_board ^= flips;
    if (color == Color::BLACK) {
      return GameBoard(my_board, op_board);
    } else {
      return GameBoard(op_board, my_board);
    }
  }

}
