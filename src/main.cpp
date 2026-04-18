// Copyright (c) Alex Li, 2025
// main.cpp
// Main entry point for the Othello game

#include <cstdlib>
#include <iostream>

#include "othello/Controller.hpp"
#include "othello/evaluator/Evaluator.hpp"
#include "utils/Profiler.hpp"

int main(int argc, char **argv) {
  // Create the engine with a positional evaluator
  uint8_t depth;
  int time_limit_ms = 2000;
  if (argc > 1) {
    const int parsed_depth = std::atoi(argv[1]);
    if (parsed_depth <= 0 || parsed_depth > 60) {
      std::cerr << "Invalid depth. Must be between 1 and 60." << std::endl;
      return 1;
    }
    depth = static_cast<uint8_t>(parsed_depth);

    if (argc > 2) {
      time_limit_ms = std::atoi(argv[2]);
      if (time_limit_ms <= 0) {
        std::cerr << "Invalid time limit. Must be greater than 0." << std::endl;
        return 1;
      }
    }
  } else {
    depth = 15; // Default depth
  }
  std::cout << "Using depth: " << depth
            << " and time limit: " << time_limit_ms << " ms" << std::endl;
  othello::initializeZobrist();
  othello::MobilityEvaluator evaluator;  // Create the mobility evaluator
  utils::profiler::start("cpu_single_thread.prof");
  utils::profiler::disable();
  utils::ThreadPool thread_pool(4); // Create a thread pool
  othello::Engine engine(evaluator, thread_pool);  // Create the engine with the evaluator
  othello::Controller controller(engine); // Create the controller with the evaluator
  controller.startGame(depth, time_limit_ms);  // Start the game loop
  return 0;                // Exit the program
}
