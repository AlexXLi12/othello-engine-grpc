// Copyright (c) 2025 Alex Li
// Engine.cpp
// Implementation of the game engine for Othello.

#include "othello/Engine.hpp"

#include <algorithm>
#include <atomic>
#include <chrono> // For timing
#include <climits>
#include <cstdint>
#include <unordered_map>

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/BitboardUtils.hpp"

namespace othello {

int Engine::findBestMove(const GameBoard &board, int max_depth, Color color,
                         int time_limit_ms) {
  using TT = std::unordered_map<uint64_t, TTEntry>;
  const auto start_time = std::chrono::steady_clock::now();

  uint64_t bb = getPossibleMoves(board, color);
  if (!bb)
    return -1;
  std::vector<int> moves = bitboard_to_positions(bb);

  // One TT per root move (reused across depths)
  std::vector<TT> tt_per_move(moves.size());
  for (auto &tt : tt_per_move)
    tt.reserve(1 << 19); // tune this

  std::pair<int, int> best_pair{INT_MIN, -1};

  for (int depth = 1; depth <= max_depth; ++depth) {
    cacheHits = 0;
    nodesSearched = 0;

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start_time)
                          .count();
    if (elapsed_ms >= time_limit_ms)
      break;

    std::atomic<int> alpha{INT_MIN};
    const int beta = INT_MAX;

    // YBW seed
    std::pair<int, int> depth_best;
    {
      GameBoard child = applyMove(board, moves[0], color);
      auto &tt0 = tt_per_move[0];
      auto r =
          negamax(child, tt0, depth - 1, -beta, -alpha.load(), opponent(color));
      int root_score = -r.first;
      int cur = alpha.load();
      while (root_score > cur &&
             !alpha.compare_exchange_weak(cur, root_score)) {
      }
      depth_best = {root_score, moves[0]};
    }

    // Parallel brothers (moving-window optional)
    std::vector<std::future<std::pair<int, int>>> futs;
    futs.reserve(moves.size() > 0 ? moves.size() - 1 : 0);

    for (size_t i = 1; i < moves.size(); ++i) {
      int mv = moves[i];
      GameBoard child = applyMove(board, mv, color);
      TT *tt = &tt_per_move[i]; // stable address; safe to capture

      futs.push_back(thread_pool.enqueue([=, this, &alpha]() {
        int a = alpha.load();
        auto r = negamax(child, *tt, depth - 1, -INT_MAX, -a, opponent(color));
        int score = -r.first;
        int cur = alpha.load();
        while (score > cur && !alpha.compare_exchange_weak(cur, score)) {
        }
        return std::make_pair(score, mv);
      }));
    }

    for (auto &f : futs) {
      auto res = f.get();
      if (res.first > depth_best.first)
        depth_best = res;
    }

    best_pair = depth_best;
  }

  return best_pair.second;
}

std::pair<int, int>
Engine::negamax(const GameBoard &board,
                std::unordered_map<uint64_t, TTEntry> &transposition_table,
                int depth, int alpha, int beta, Color color) {
  int alpha_orig = alpha;
  bool entry_exists = false;
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
    const std::pair<int, int> pair = negamax(
        board, transposition_table, depth - 1, -beta, -alpha, opponent(color));
    return {-pair.first, -1}; // Negate the opponent's score
  }

  std::vector<int> legal_moves = bitboard_to_positions(legal_moves_bb);
  // move transposition table entry to the front if it exists
  if (entry_exists) {
    int tt_move = it->second.move_index;
    auto it = std::find(legal_moves.begin(), legal_moves.end(), tt_move);
    if (it != legal_moves.end()) {
      std::iter_swap(legal_moves.begin(), it);
    }
  }
  std::pair<int, int> best_pair = {
      INT_MIN, legal_moves[0]}; // initialize with worst case
  for (const int move : legal_moves) {
    const GameBoard new_board = applyMove(board, move, color);
    const auto pair = negamax(new_board, transposition_table, depth - 1, -beta,
                              -alpha, opponent(color));
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
  transposition_table[board.zobrist_hash] =
      TTEntry{best_pair.first, depth, bound_type, best_pair.second};
  return best_pair;
}
} // namespace othello
