// Copyright (c) 2025 Alex Li
// Engine.cpp
// Implementation of the game engine for Othello.

#include "othello/Engine.hpp"

#include <chrono>  // For timing
#include <climits>
#include <iostream>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"

namespace othello {

Engine::Engine(const Evaluator &evaluator) : evaluator(evaluator) {
  // Initialize the transposition table
  transposition_table = std::map<uint64_t, TTEntry>();
}

int Engine::findBestMove(const GameBoard &board, int max_depth, Color color,
                         int time_limit_ms) {
  std::pair<int, int> best_pair = std::make_pair(INT_MIN, -1);
  const auto start_time = std::chrono::steady_clock::now();
  // iterative deepening
  for (int depth = 1; depth <= max_depth; ++depth) {
    std::cout << "Searching at depth " << depth << "..." << std::endl;
    const auto current_time = std::chrono::steady_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(current_time -
                                                              start_time)
            .count();
    if (elapsed_ms >= time_limit_ms) {
      std::cout << "Time limit reached at depth " << depth << std::endl;
      break;
    }
    // clear transposition table for each depth
    transposition_table.clear();
    const auto pair = negamax(board, depth, INT_MIN, INT_MAX, color);
    if (pair.first > best_pair.first) {
      best_pair = pair;
    }
  }

  return best_pair.second;
}

std::pair<int, int> Engine::negamax(const GameBoard &board, int depth,
                                    int alpha, int beta, Color color) {
  if (depth == 0) {
    const int score = static_cast<int>(color) * evaluator.evaluate(board);
    return {score, -1};  // Return score and no move index
  }
  uint64_t legal_moves_bb = othello::getPossibleMoves(board, color);
  if (legal_moves_bb == 0) {
    if (othello::getPossibleMoves(board, opponent(color)) == 0) {
      // No legal moves for both players, game over. Evaluate based on disc
      // count.
      const auto disc_count = othello::countDiscs(board);
      const int score =
          static_cast<int>(color) * (disc_count.first - disc_count.second);
      if (score > 0) {
        return {INT_MAX, -1};  // We win
      } else if (score < 0) {
        return {INT_MIN, -1};  // We lose
      } else {
        return {0, -1};  // Draw
      }
    }
    // pass turn
    return negamax(board, depth - 1, -beta, -alpha, opponent(color));
  }
  const std::vector<int> legal_moves =
      othello::bitboard_to_positions(legal_moves_bb);
  std::pair<int, int> best_pair = {
      INT_MIN, legal_moves[0]};  // initialize with worst case
  for (const int move : legal_moves) {
    GameBoard new_board = othello::applyMove(board, move, color);
    const auto pair = negamax(new_board, depth - 1, -beta, -alpha,
                              opponent(color));
    const int score = -pair.first;  // Negate the score for the opponent
    if (score > best_pair.first) {
      best_pair = {score, move};
    }
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      break;  // Alpha-beta pruning
    }
  }
  return best_pair;
}
}  // namespace othello

