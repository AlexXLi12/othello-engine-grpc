// Copyright (c) 2025, Alex Li
// benchmark.cpp
// Source code for benchmarking engine performance under various configurations.

#include <chrono>
#include <gperftools/profiler.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <stdint.h>

#include "othello/Engine.hpp"
#include "othello/GameBoard.hpp"
#include "utils/ThreadPool.hpp"

int main() {
  // Initialize zobrist hashing / random seeds
  othello::initializeZobrist();

  // Create evaluator + thread pool
  othello::PositionalEvaluator evaluator;
  utils::ThreadPool thread_pool(5);
  othello::Engine engine(evaluator, thread_pool);

  // Benchmarking parameters
  const std::vector<int> depths = {1, 5, 10, 15, 18};
  const int num_runs = 5; // Number of runs per depth

  // Start position
  othello::GameBoard board = othello::createInitialBoard();

  std::cout << "Beginning benchmarking..." << std::endl;
  std::cout << std::fixed << std::setprecision(3);

  // Start CPU profiler
  ProfilerStart("cpu_5_thread.prof");

  for (int depth : depths) {
    double total_ms = 0.0;
    for (int run = 0; run < num_runs; ++run) {
      auto start = std::chrono::high_resolution_clock::now();

      // Run engine search at given depth (no time limit, say 10s cap)
      int best_move = engine.findBestMove(board, depth, othello::Color::BLACK,
                                          /*time_limit_ms=*/INT32_MAX);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      total_ms += elapsed.count();

      std::cout << "Run " << (run + 1) << " at depth " << depth
                << " → best move: " << best_move
                << " | elapsed: " << elapsed.count() << " ms\n";
    }
    double avg_ms = total_ms / num_runs;
    std::cout << "[Depth " << depth << "] Average time: " << avg_ms << " ms\n";
    std::cout << "--------------------------------------\n";
  }

  // Stop CPU profiler
  ProfilerStop();

  std::cout << "Benchmarking complete.\n";
  return 0;
}

