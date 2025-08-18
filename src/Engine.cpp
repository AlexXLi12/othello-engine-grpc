// Copyright (c) 2025 Alex Li
// Engine.cpp
// Implementation of the game engine for Othello.

#include "othello/Engine.hpp"

#include <algorithm>
#include <chrono> // For timing
#include <climits>
#include <iostream>
#include <shared_mutex>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"

namespace othello {

int Engine::findBestMove(const GameBoard &board, int max_depth, Color color,
                         int time_limit_ms) {
  std::pair<int, int> best_pair = std::make_pair(INT_MIN, -1);
  const auto start_time = std::chrono::steady_clock::now();
  // clear transposition table at the start of each search
  {
    std::unique_lock<std::shared_mutex> lock(tt_mutex);
    transposition_table.clear();
  }
  cacheHits = 0; // reset cache hits
  nodesSearched = 0; // reset nodes searched
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
    uint64_t legal_moves_bb = getPossibleMoves(board, color);
    std::vector<int> legal_moves =
        bitboard_to_positions(legal_moves_bb);
    std::vector<std::future<std::pair<int, int>>> futures;
    // TODO: Fix logic
    // call negamax as opponent
    // YBW seed
    // shared alpha and beta (atomic)
    // time budgeting
    for (const int move : legal_moves) {
      GameBoard new_board = applyMove(board, move, color);
      // add the search to the thread pool
      std::future<std::pair<int, int>> pair = thread_pool.enqueue([this, move, new_board, depth, color]() {
        const auto pair = negamax(new_board, depth, INT_MIN, INT_MAX, color);
        return std::make_pair(pair.first, move); // Return score and move index
      });
      futures.push_back(std::move(pair));
    }
    // Wait for all futures to complete and find the best move
    for (auto &future : futures) {
      const auto pair = future.get();
      if (pair.first > best_pair.first) {
        best_pair = pair; // Update best move if found a better score
      }
    }
  }
  {
    std::shared_lock<std::shared_mutex> lock(tt_mutex);
    std::cout << "size of transposition table: " << transposition_table.size()
              << std::endl;
  }
  std::cout << "cache hits: " << cacheHits << std::endl;
  std::cout << "nodes searched: " << nodesSearched << std::endl;
  return best_pair.second;
}

std::pair<int, int> Engine::negamax(const GameBoard &board, int depth,
                                    int alpha, int beta, Color color) {
  int alpha_orig = alpha;
  const TTEntry *tt_entry = nullptr;
  bool entry_exists = false;
  {
    std::shared_lock<std::shared_mutex> lock(tt_mutex);
    auto it = transposition_table.find(board.zobrist_hash);
    if (it != transposition_table.end()) {
      entry_exists = true;
      const TTEntry &entry = it->second;
      if (entry.depth >= depth) {
        // Use the stored value if it's valid for the current depth and bounds
        if (entry.bound_type == BoundType::EXACT ||
            (entry.bound_type == BoundType::LOWER && entry.score >= alpha) ||
            (entry.bound_type == BoundType::UPPER && entry.score <= beta)) {
          ++cacheHits;
          return {entry.score, entry.move_index};
        }
        tt_entry = &entry; // Use the entry for further checks
      }
    }
  }
  ++nodesSearched;
  if (depth == 0) {
    const int score = static_cast<int>(color) * evaluator.evaluate(board);
    return {score, -1}; // Return score and no move index
  }
  uint64_t legal_moves_bb = getPossibleMoves(board, color);
  if (legal_moves_bb == 0) {
    if (othello::getPossibleMoves(board, opponent(color)) == 0) {
      // No legal moves for both players, game over. Evaluate based on disc
      // count.
      const auto disc_count = countDiscs(board);
      const int score = 100 * static_cast<int>(color) *
                        (disc_count.first - disc_count.second);
      return {score, -1}; // Return score and no move index
    }
    // pass turn
    const std::pair<int, int> pair =
        negamax(board, depth - 1, -beta, -alpha, opponent(color));
    return {-pair.first, -1}; // Negate the opponent's score
  }

  std::vector<int> legal_moves = bitboard_to_positions(legal_moves_bb);
  // move transposition table entry to the front if it exists
  if (entry_exists) {
    int tt_move = tt_entry->move_index;
    auto it = std::find(legal_moves.begin(), legal_moves.end(), tt_move);
    if (it != legal_moves.end()) {
      std::iter_swap(legal_moves.begin(), it);
    }
  }
  std::pair<int, int> best_pair = {
      INT_MIN, legal_moves[0]}; // initialize with worst case
  for (const int move : legal_moves) {
    const GameBoard new_board = applyMove(board, move, color);
    const auto pair =
        negamax(new_board, depth - 1, -beta, -alpha, opponent(color));
    const int score = -pair.first; // Negate the opponent's score
    if (score > best_pair.first) {
      best_pair = {score, move};
    }
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      break; // Alpha-beta pruning
    }
  }
  BoundType bound_type;
  if (best_pair.first <= alpha_orig)
    bound_type = BoundType::UPPER;
  else if (best_pair.first >= beta)
    bound_type = BoundType::LOWER;
  else
    bound_type = BoundType::EXACT;
  {
    std::unique_lock<std::shared_mutex> lock(tt_mutex);
    transposition_table[board.zobrist_hash] =
        TTEntry{best_pair.first, depth, bound_type, best_pair.second};
  }
  return best_pair;
}
} // namespace othello
