// Copyright (c) 2025 Alex Li
// Visualize.cpp
// Implementation of utility functions for visualizing the Othello game board

#include "utils/Visualize.hpp"
#include "othello/GameBoard.hpp" 
#include <cstdint>                // For uint64_t

namespace othello {
  std::string boardToString(const GameBoard& board) {
    std::string result;
    uint64_t curPos = 1;
    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 8; ++col) {
        if (board.black_bb & curPos) {
          result += "X";
        } else if (board.white_bb & curPos) {
          result += "O";
        } else {
          result += "."; // Empty square
        }
        curPos <<= 1; // Move to the next square
      }
      result += "\n";
    }
    return result;
  }

  std::string bitboardToString(uint64_t bb) {
    std::string result;
    for (int i = 0; i < 64; ++i) {
      result += (bb & 1) ? '1' : '0';
      bb >>= 1;
      if ((i + 1) % 8 == 0) {
        result += '\n';
      }
    }
    return result;
  }
}