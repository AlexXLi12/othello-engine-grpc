// Copyright (c) Alex Li 2025
// OthelloRules.cpp
// Implementations of functions defined in OthelloRules.hpp

#include <vector>
#include <utility>  // for std::pair
#include "OthelloRules.hpp"
#include "GameBoard.hpp"

namespace othello {

  
  std::vector<int> get_possible_moves(const GameBoard &b, Color color) {
    uint64_t my_board = b.black_bb ? color == Color::BLACK : b.white_bb;
    uint64_t op_board = b.white_bb ? color == Color::WHITE : b.black_bb;

    // TODO: Implement
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
  }
}
