// Copyright (c) 2025 Alex Li
// Visualize.cpp
// Implementation of utility functions for visualizing the Othello game board

#include "utils/Visualize.hpp"

#include <cstdint>  // For uint64_t

#include "othello/GameBoard.hpp"

namespace othello {
std::string board_to_string(const GameBoard& board) {
  std::string result;
  uint64_t curPos = 1;
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      if (board.black_bb & curPos) {
        result += "X";
      } else if (board.white_bb & curPos) {
        result += "O";
      } else {
        result += ".";  // Empty square
      }
      curPos <<= 1;  // Move to the next square
    }
    result += "\n";
  }
  return result;
}

std::string board_to_string_with_moves(const GameBoard& board, uint64_t possible_moves) {
    std::string result;
    uint64_t curPos = 1;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board.black_bb & curPos) {
                result += "X";
            } else if (board.white_bb & curPos) {
                result += "O";
            } else if (possible_moves & curPos) {
                result += "*";  // Possible move
            } else {
                result += ".";  // Empty square
            }
            curPos <<= 1;  // Move to the next square
        }
        result += "\n";
    }
    return result;
}

std::string bitboard_to_string(uint64_t bb) {
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
}  // namespace othello

