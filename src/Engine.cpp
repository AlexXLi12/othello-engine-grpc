// Copyright (c) 2025 Alex Li
// Engine.cpp
// Implementation of the game engine for Othello.

#include "othello/Engine.hpp"

#include <algorithm>
#include <chrono>  // For timing
#include <climits>
#include <iostream>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"

namespace othello {

Engine::Engine(const Evaluator &evaluator)
    : evaluator(evaluator), transposition_table() {}

int Engine::findBestMove(const GameBoard &board, int max_depth, Color color,
                         int time_limit_ms) {
  std::pair<int, int> best_pair = std::make_pair(INT_MIN, -1);
  const auto start_time = std::chrono::steady_clock::now();
  // clear transposition table at the start of each search
  transposition_table.clear();
  cacheHits = 0;
  nodesSearched = 0;
  // iterative deepening
  for (int depth = 1; depth <= max_depth; ++depth) {
    const auto current_time = std::chrono::steady_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(current_time -
                                                              start_time)
            .count();
    if (elapsed_ms >= time_limit_ms) {
      std::cout << "Time limit reached at depth " << depth << std::endl;
      break;
    }
    const auto pair = negamax(board, depth, INT_MIN, INT_MAX, color);
    best_pair = pair;
    std::cout << "Search at depth " << depth << ": best score = " << pair.first
              << ", best move = " << pair.second << std::endl;
  }
  std::cout << "size of transposition table: " << transposition_table.size()
            << std::endl;
  std::cout << "cache hits: " << cacheHits << std::endl;
  std::cout << "nodes searched: " << nodesSearched << std::endl;
  return best_pair.second;
}

std::pair<int, int> Engine::negamax(const GameBoard &board, int depth,
                                    int alpha, int beta, Color color) {
  int alpha_orig = alpha;
  const TTEntry *tt_entry = nullptr;
  auto it = transposition_table.find(board.zobrist_hash);
  if (it != transposition_table.end()) {
    const TTEntry &entry = it->second;
    if (entry.depth >= depth) {
      // Use the stored value if it's valid for the current depth and bounds
      if (entry.bound_type == BoundType::EXACT ||
          (entry.bound_type == BoundType::LOWER && entry.score >= alpha) ||
          (entry.bound_type == BoundType::UPPER && entry.score <= beta)) {
        ++cacheHits;
        return {entry.score, entry.move_index};
      }
      tt_entry = &entry;  // Use the entry for further checks
    }
  }
  ++nodesSearched;
  if (depth == 0) {
    const int score = static_cast<int>(color) * evaluator.evaluate(board);
    return {score, -1};  // Return score and no move index
  }
  uint64_t legal_moves_bb = getPossibleMoves(board, color);
  if (legal_moves_bb == 0) {
    if (othello::getPossibleMoves(board, opponent(color)) == 0) {
      // No legal moves for both players, game over. Evaluate based on disc
      // count.
      const auto disc_count = countDiscs(board);
      const int score = 100 * static_cast<int>(color) *
                        (disc_count.first - disc_count.second);
      return {score, -1};  // Return score and no move index
    }
    // pass turn
    const std::pair<int, int> pair =
        negamax(board, depth - 1, -beta, -alpha, opponent(color));
    return {-pair.first, -1};  // Negate the opponent's score
  }

  std::vector<int> legal_moves = bitboard_to_positions(legal_moves_bb);
  // move transposition table entry to the front if it exists
  if (tt_entry != nullptr) {
    int tt_move = tt_entry->move_index;
    auto it = std::find(legal_moves.begin(), legal_moves.end(), tt_move);
    if (it != legal_moves.end()) {
      std::iter_swap(legal_moves.begin(), it);
    }
  }
  std::pair<int, int> best_pair = {
      INT_MIN, legal_moves[0]};  // initialize with worst case
  for (const int move : legal_moves) {
    GameBoard new_board = applyMove(board, move, color);
    const auto pair =
        negamax(new_board, depth - 1, -beta, -alpha, opponent(color));
    const int score = -pair.first;  // Negate the opponent's score
    if (score > best_pair.first) {
      best_pair = {score, move};
    }
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      break;  // Alpha-beta pruning
    }
  }
  BoundType bound_type;
  if (best_pair.first <= alpha_orig)
    bound_type = BoundType::UPPER;
  else if (best_pair.first >= beta)
    bound_type = BoundType::LOWER;
  else
    bound_type = BoundType::EXACT;
  transposition_table[board.zobrist_hash] =
      TTEntry{best_pair.first, depth, bound_type, best_pair.second};
  return best_pair;
}
}  // namespace othello

