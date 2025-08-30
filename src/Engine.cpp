// Copyright (c) 2025 Alex Li
// Engine.cpp
// Implementation of the game engine for Othello.

#include "othello/Engine.hpp"

#include <algorithm>
#include <atomic>
#include <bit>
#include <chrono> // For timing
#include <cstdint>
#include <iostream>
#include <unordered_map>

#include "othello/Constants.hpp"
#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"

static constexpr int INF = 1 << 20;

namespace othello {

using TT = std::unordered_map<uint64_t, TTEntry>;

namespace {
std::vector<int> order_moves(uint64_t moves_bb,
                 const std::unordered_map<uint64_t, TTEntry> &tt,
                 uint64_t zobrist_hash) {
  // prefer corners and edges
  // 1. corners
  uint64_t corner_board = moves_bb & CORNER_MASK;
  moves_bb &= ~CORNER_MASK;
  // 2. edges
  uint64_t edge_board = moves_bb & (EDGE_MASK);
  moves_bb &= ~(EDGE_MASK);
  std::vector<int> moves;
  auto append_positions = [&moves](uint64_t bb) {
    while (bb) {
      int pos = std::countr_zero(bb);
      moves.push_back(pos);
      bb &= bb - 1;
    }
  };
  append_positions(corner_board);
  append_positions(edge_board);
  append_positions(moves_bb);

  auto it = tt.find(zobrist_hash);
  if (it != tt.end()) {
    int tt_move = it->second.move_index;
    auto pos = std::find(moves.begin(), moves.end(), tt_move);
    if (pos != moves.end()) {
      std::iter_swap(moves.begin(), pos);
    }
  }
  return moves;
}

} // namespace
int Engine::findBestMove(const GameBoard &board, uint8_t max_depth, Color color,
                         int time_limit_ms) {
  const auto start_time = std::chrono::steady_clock::now();

  uint64_t bb = getPossibleMoves(board, color);
  if (!bb)
    return -1;

  // One TT per root move (reused across depths)
  std::vector<TT> tt_per_move(std::popcount(bb));
  for (auto &tt : tt_per_move)
    tt.reserve(1 << 19); // tune this

  std::vector<int> moves = order_moves(bb, tt_per_move[0], board.zobrist_hash);

  std::pair<int, int> best_pair{-INF, -1};

  cacheHits = 0;
  nodesSearched = 0;
  for (int depth = 1; depth <= max_depth; ++depth) {

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start_time)
                          .count();
    if (elapsed_ms >= time_limit_ms) {
      std::cout << "Time limit reached at depth " << (depth - 1) << std::endl;
      break;
    }

    std::atomic<int> alpha{-INF};
    const int beta = INF;

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

    // Parallel brothers
    std::vector<std::future<std::pair<int, int>>> futs;
    futs.reserve(moves.size() > 0 ? moves.size() - 1 : 0);

    for (size_t i = 1; i < moves.size(); ++i) {
      int mv = moves[i];
      GameBoard child = applyMove(board, mv, color);
      TT *tt = &tt_per_move[i]; // stable address; safe to capture

      futs.push_back(thread_pool.enqueue([=, this, &alpha]() {
        int a = alpha.load(std::memory_order_relaxed);

        // Scout search (zero window)
        auto pr = negamax(child, *tt, depth - 1, -a - 1, -a, opponent(color));
        int probe = -pr.first;

        int score;
        if (probe > a) {
          // Re-search with full window
          auto fr = negamax(child, *tt, depth - 1, -INF, -a, opponent(color));
          score = -fr.first;
        } else {
          score = probe;
        }

        // Raise shared alpha
        int cur = alpha.load(std::memory_order_relaxed);
        while (score > cur && !alpha.compare_exchange_weak(cur, score)) {
        }
        return std::make_pair(score, mv);
      }));
    }
    for (auto &f : futs) {
      auto res = f.get();
      if (res.first > depth_best.first) {
        depth_best = res;
      }
    }

    best_pair = depth_best;
  }
  std::cout << "Nodes searched: " << nodesSearched
            << " | Cache hits: " << cacheHits << std::endl;
  std::cout << "Best move: " << best_pair.second
            << " | Score: " << best_pair.first << std::endl;
  return best_pair.second;
}

std::pair<int, int8_t>
Engine::negamax(const GameBoard &board,
                std::unordered_map<uint64_t, TTEntry> &transposition_table,
                uint8_t depth, int alpha, int beta, Color color) {
  int alpha_orig = alpha;
  auto it = transposition_table.find(board.zobrist_hash);
  if (it != transposition_table.end()) {
    const TTEntry &entry = it->second;
    if (entry.depth >= depth) {
      // Use the stored value if it's valid for the current depth and bounds
      if (entry.bound_type == BoundType::EXACT ||
          (entry.bound_type == BoundType::LOWER && entry.score >= beta) ||
          (entry.bound_type == BoundType::UPPER && entry.score <= alpha)) {
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

  std::vector<int> legal_moves = order_moves(legal_moves_bb, transposition_table, board.zobrist_hash);

  std::pair<int, int8_t> best_pair = {
      -INF, legal_moves[0]}; // initialize with worst case
  bool first = true;
  for (const int move : legal_moves) {
    const GameBoard new_board = applyMove(board, move, color);

    int score;
    if (first) {
      // First move: full window to seed alpha
      auto r = negamax(new_board, transposition_table, depth - 1, -beta, -alpha,
                       opponent(color));
      score = -r.first;
      first = false;
    } else {
      // PVS: scout (zero-window) first
      auto pr = negamax(new_board, transposition_table, depth - 1, -alpha - 1,
                        -alpha, opponent(color));
      int probe = -pr.first;

      if (probe > alpha) {
        // Fail-high -> re-search with full window
        auto fr = negamax(new_board, transposition_table, depth - 1, -beta,
                          -alpha, opponent(color));
        score = -fr.first;
      } else {
        // Fail-low -> accept scout
        score = probe;
      }
    }

    if (score > best_pair.first) {
      best_pair = {score, move};
    }
    alpha = std::max(alpha, score);
    if (alpha >= beta)
      break; // cutoff
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
