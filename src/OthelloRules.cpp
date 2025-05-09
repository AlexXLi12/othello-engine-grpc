// Copyright (c) Alex Li 2025
// OthelloRules.cpp
// Implementations of functions defined in OthelloRules.hpp
#include <vector>
#include <utility>            // for std::pair
#include "othello/OthelloRules.hpp"
#include "othello/Constants.hpp"      // for bitboard constants
#include "othello/GameBoard.hpp"      // for GameBoard and Color

namespace othello {

  // These functions are used to get the possible move boards in each direction
  uint64_t get_west_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_east_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_north_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_south_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_north_west_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_north_east_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_south_west_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);
  uint64_t get_south_east_moves(uint64_t my_board, uint64_t op_board, uint64_t empty);

  uint64_t get_possible_moves(const GameBoard &b, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;

    std::vector<int> possible_moves;
    uint64_t empty_squares = ~(my_board | op_board);

    uint64_t moves = get_west_moves(my_board, op_board, empty_squares);
    moves |= get_east_moves(my_board, op_board, empty_squares);
    moves |= get_north_moves(my_board, op_board, empty_squares);
    moves |= get_south_moves(my_board, op_board, empty_squares);
    moves |= get_north_west_moves(my_board, op_board, empty_squares);
    moves |= get_north_east_moves(my_board, op_board, empty_squares);
    moves |= get_south_west_moves(my_board, op_board, empty_squares);
    moves |= get_south_east_moves(my_board, op_board, empty_squares);
    return moves;
  }

  bool is_valid_move(const GameBoard &b, int position, Color color) {
    uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
    uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;
    uint64_t empty_squares = ~(my_board | op_board);
    if ((1ULL << position) & ~empty_squares) {
       // position is not empty
      return false;
    }
    uint64_t pos_board = 1ULL << position;
    return get_west_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_east_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_north_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_south_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_north_west_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_north_east_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_south_west_moves(my_board, op_board, empty_squares) & (pos_board) ||
           get_south_east_moves(my_board, op_board, empty_squares) & (pos_board);
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
    // TODO: Implement
    return b;
  }

  uint64_t get_west_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // West = shift right 1
    uint64_t mask = op_board & othello::LEFT_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::LEFT_EDGE_MASK) >> 1);
    t |= mask & ((t & othello::LEFT_EDGE_MASK) >> 1);
    t |= mask & ((t & othello::LEFT_EDGE_MASK) >> 1);
    t |= mask & ((t & othello::LEFT_EDGE_MASK) >> 1);
    t |= mask & ((t & othello::LEFT_EDGE_MASK) >> 1);
    t |= mask & ((t & othello::LEFT_EDGE_MASK) >> 1);
    return empty & ((t & othello::LEFT_EDGE_MASK) >> 1);
  }

  uint64_t get_east_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // East = shift left 1
    uint64_t mask = op_board & othello::RIGHT_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::RIGHT_EDGE_MASK) << 1);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK) << 1);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK) << 1);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK) << 1);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK) << 1);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK) << 1);
    return empty & ((t & othello::RIGHT_EDGE_MASK) << 1);
  }

  uint64_t get_north_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // North = shift right 8
    uint64_t mask = op_board & othello::TOP_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::TOP_EDGE_MASK) >> 8);
    t |= mask & ((t & othello::TOP_EDGE_MASK) >> 8);
    t |= mask & ((t & othello::TOP_EDGE_MASK) >> 8);
    t |= mask & ((t & othello::TOP_EDGE_MASK) >> 8);
    t |= mask & ((t & othello::TOP_EDGE_MASK) >> 8);
    t |= mask & ((t & othello::TOP_EDGE_MASK) >> 8);
    return empty & ((t & othello::TOP_EDGE_MASK) >> 8);
  }

  uint64_t get_south_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // South = shift left 8
    uint64_t mask = op_board & othello::BOTTOM_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::BOTTOM_EDGE_MASK) << 8);
    t |= mask & ((t & othello::BOTTOM_EDGE_MASK) << 8);
    t |= mask & ((t & othello::BOTTOM_EDGE_MASK) << 8);
    t |= mask & ((t & othello::BOTTOM_EDGE_MASK) << 8);
    t |= mask & ((t & othello::BOTTOM_EDGE_MASK) << 8);
    t |= mask & ((t & othello::BOTTOM_EDGE_MASK) << 8);
    return empty & ((t & othello::BOTTOM_EDGE_MASK) << 8);
  }

  uint64_t get_north_west_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // North-West = shift right 9
    uint64_t mask = op_board & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
    return empty & ((t & othello::LEFT_EDGE_MASK & othello::TOP_EDGE_MASK) >> 9);
  }

  uint64_t get_north_east_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // North-East = shift left 7
    uint64_t mask = op_board & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
    return empty & ((t & othello::RIGHT_EDGE_MASK & othello::TOP_EDGE_MASK) << 7);
  }

  uint64_t get_south_west_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // South-West = shift right 7
    uint64_t mask = op_board & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
    t |= mask & ((t & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
    return empty & ((t & othello::LEFT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) >> 7);
  }

  uint64_t get_south_east_moves(uint64_t my_board, uint64_t op_board, uint64_t empty) {
    // South-East = shift left 9
    uint64_t mask = op_board & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK;
    uint64_t t = mask & ((my_board & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
    t |= mask & ((t & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
    return empty & ((t & othello::RIGHT_EDGE_MASK & othello::BOTTOM_EDGE_MASK) << 9);
  }
}
