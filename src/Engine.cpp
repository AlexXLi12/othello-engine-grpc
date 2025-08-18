// Copyright (c) 2025 Alex Li
// Engine.cpp
// Implementation of the game engine for Othello.

#include "othello/Engine.hpp"

#include <algorithm>
#include <chrono> // For timing
#include <climits>
#include <iostream>
#include <shared_mutex>
#include <atomic>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"

namespace othello {

int Engine::findBestMove(const GameBoard &board, int max_depth, Color color,
                         int time_limit_ms) {
  std::pair<int, int> best_pair = std::make_pair(INT_MIN, -1);
  const auto start_time = std::chrono::steady_clock::now();
  uint64_t legal_moves_bb = getPossibleMoves(board, color);
  if (legal_moves_bb == 0) {
    std::cout << "No legal moves available for color " << static_cast<int>(color)
              << std::endl;
    return -1; // No valid moves available
  }
  std::vector<int> legal_moves =
      bitboard_to_positions(legal_moves_bb);
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
    std::cout << "Searching at depth " << depth << ", total time elapsed: " << elapsed_ms << std::endl;
    if (elapsed_ms >= time_limit_ms) {
      std::cout << "Time limit reached at depth " << depth << std::endl;
      break;
    }
    std::vector<std::future<std::pair<int, int>>> futures;
    std::atomic<int> alpha{INT_MIN};
    int beta = INT_MAX;
    // TODO: Fix logic
    // call negamax as opponent
    // YBW seed
    std::pair<int, int> depth_best_pair;
    {
      GameBoard new_board = applyMove(board, legal_moves[0], color);
      const auto pair = negamax(new_board, depth - 1, -beta, -alpha.load(),
                                opponent(color));
      const int root_best_score = -pair.first;
      int cur = alpha.load();
      while (root_best_score > cur && !alpha.compare_exchange_weak(cur, root_best_score)) {}
      depth_best_pair = std::make_pair(root_best_score, legal_moves[0]);
    }
    for (int i = 1; i < legal_moves.size(); ++i) {
      const int move = legal_moves[i];
      GameBoard new_board = applyMove(board, move, color);
      // add the search to the thread pool
      std::future<std::pair<int, int>> pair = thread_pool.enqueue([=, this, &alpha]() {
        int local_alpha = alpha.load();
        const auto nm = negamax(new_board, depth-1, -INT_MAX, -local_alpha, opponent(color));
        int cur = alpha.load();
        // Negate the score to get the score for the current player
        int score = -nm.first;
        while (score > cur && !alpha.compare_exchange_weak(cur, score)) {
          // Try to update alpha until successful or -pair.first is not greater than cur
        }
        return std::make_pair(score, move); // Return score and move index
      });
      futures.push_back(std::move(pair));
    }
    // Wait for all futures to complete and find the best move
    for (auto &future : futures) {
      const auto pair = future.get();
      if (pair.first > depth_best_pair.first) {
        depth_best_pair = pair; // Update best move if found a better score
      }
    }
    best_pair = depth_best_pair;
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
  bool entry_exists = false;
  TTEntry entry_copy;
  {
    std::shared_lock<std::shared_mutex> lock(tt_mutex);
    auto it = transposition_table.find(board.zobrist_hash);
    if (it != transposition_table.end()) {
      entry_exists = true;
      entry_copy = it->second;
      const TTEntry &entry = it->second;
      if (entry.depth >= depth) {
        // Use the stored value if it's valid for the current depth and bounds
        if (entry.bound_type == BoundType::EXACT ||
            (entry.bound_type == BoundType::LOWER && entry.score >= alpha) ||
            (entry.bound_type == BoundType::UPPER && entry.score <= beta)) {
          ++cacheHits;
          return {entry.score, entry.move_index};
        }
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
    int tt_move = entry_copy.move_index;
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
