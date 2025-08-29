// Copyright (c) Alex Li, 2025
// Controller.cpp
// Implementation of the Controller class for user input and game state
// management

#include "othello/Controller.hpp"

#include <iostream>
#include <limits>  // For std::numeric_limits
#include <set>
#include <gperftools/profiler.h>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"
#include "utils/Visualize.hpp"

namespace othello {

void Controller::startGame(int depth, int time_limit_ms) {
  Color current_color = Color::BLACK;  // Start with black player
  int moves = 0;
  while (true) {
    ++moves;
    if (isTerminal(board)) {
      checkGameOver();
      std::cout << board_to_string(board) << std::endl;
      break;
    }
    std::vector<int> possible_moves_vec =
        othello::bitboard_to_positions(getPossibleMoves(board, current_color));
    std::cout << board_to_string_with_moves(board, getPossibleMoves(board, current_color)) << std::endl;
    std::cout << "AI is thinking..." << std::endl;
    for (const auto &m : possible_moves_vec) {
      std::cout << m << " ";
    }
    std::cout << std::endl;
    ProfilerEnable();
    int ai_move = engine.findBestMove(board, depth, current_color, time_limit_ms);
    ProfilerDisable();
    if (ai_move == -1) {
      std::cout << "No valid moves available for AI. Passing." << std::endl;
    } else {
      board = applyMove(board, ai_move, current_color);
      std::string ai_color_str =
          current_color == Color::BLACK ? "Black" : "White";
      std::cout << ai_color_str << " played move: " << ai_move << std::endl;
    }
    current_color = opponent(current_color);  // Switch player
  }
  ProfilerStop();
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

