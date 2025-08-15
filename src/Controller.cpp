// Copyright (c) Alex Li, 2025
// Controller.cpp
// Implementation of the Controller class for user input and game state
// management

#include "othello/Controller.hpp"

#include <iostream>
#include <limits>  // For std::numeric_limits
#include <set>
#include <vector>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"
#include "utils/Visualize.hpp"

namespace othello {

void Controller::startGame() {
  Color current_color = Color::BLACK;  // Start with black player
  while (true) {
    if (isTerminal(board)) {
      checkGameOver();
      break;
    }

    if (current_color == Color::BLACK) {
      // check if user has valid moves
      const uint64_t possible_moves_bb = getPossibleMoves(board, current_color);
      const std::vector<int> possible_moves =
          bitboard_to_positions(possible_moves_bb);
      if (possible_moves.empty()) {
        std::cout << "No valid moves available for Player. Passing."
                  << std::endl;
      } else {
        std::cout << board_to_string_with_moves(board, possible_moves_bb)
                  << std::endl;
        int move = handleUserInput(
            std::set<int>(possible_moves.begin(), possible_moves.end()));
        board = applyMove(board, move, Color::BLACK);
      }
    } else {
      std::cout << "AI is thinking..." << std::endl;
      int ai_move = engine.findBestMove(board, 30, current_color, 3000);
      if (ai_move == -1) {
        std::cout << "No valid moves available for AI. Passing." << std::endl;
      } else {
        std::cout << board_to_string(board) << std::endl;
        board = applyMove(board, ai_move, current_color);
        std::cout << "AI played move: " << ai_move << std::endl;
      }
    }
    current_color = opponent(current_color);  // Switch player
  }
}

int Controller::handleUserInput(const std::set<int> &possible_moves) {
  int move;
  std::cout << "Enter your move from the following options: ";
  for (const auto &m : possible_moves) {
    std::cout << m << " ";
  }
  std::cout << std::endl;
  while (true) {
    std::cin >> move;
    if (std::cin.fail() || !possible_moves.contains(move)) {
      std::cin.clear();  // Clear the error flag
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                      '\n');  // Discard invalid input
      std::cout
          << "Invalid input. Please enter a valid move from the options: ";
      for (const auto &m : possible_moves) {
        std::cout << m << " ";
      }
      std::cout << std::endl;
    } else {
      return move;  // Valid input
    }
  }
}

void Controller::checkGameOver() {
  const std::pair<int, int> disc_count = countDiscs(board);
  std::cout << "Game Over!" << std::endl;
  std::cout << "Black discs: " << disc_count.first
            << ", White discs: " << disc_count.second << std::endl;
  if (disc_count.first > disc_count.second) {
    std::cout << "Black wins!" << std::endl;
  } else if (disc_count.first < disc_count.second) {
    std::cout << "White wins!" << std::endl;
  } else {
    std::cout << "It's a draw!" << std::endl;
  }
}
}  // namespace othello

