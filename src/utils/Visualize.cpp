// Copyright (c) 2025 Alex Li
// Visualize.cpp
// Implementation of utility functions for visualizing the Othello game board

#include "Visualize.hpp"
#include "../GameBoard.hpp" 

std::string boardToString(const GameBoard& board) {
  std::string result;
  int curPos = 1;
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      if (board.black_bb & curPos) {
        result += "X";
      } else if (board.white_bb & curPos) {
        result += "O";
      } else {
        result += "."; // Empty square
      }
    }
    result += "\n";
  }
  return result;
}