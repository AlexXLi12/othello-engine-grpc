// Copyright (c) Alex Li, 2025
// main.cpp
// Main entry point for the Othello game

#include <gperftools/profiler.h>
#include <iostream>

#include "othello/Controller.hpp"
#include "othello/evaluator/Evaluator.hpp"

int main(int argc, char **argv) {
  // Create the engine with a positional evaluator
  int depth, time_limit_ms;
  if (argc > 1) {
    // validate 0 < argv[1] <= 60
    if (std::atoi(argv[1]) <= 0 || std::atoi(argv[1]) > 60) {
      std::cerr << "Invalid depth. Must be between 1 and 60." << std::endl;
      return 1;
    }
    if (argc > 2) {
      if (std::atoi(argv[2]) <= 0) {
        std::cerr << "Invalid time limit. Must be greater than 0." << std::endl;
        return 1;
      }
      time_limit_ms = std::atoi(argv[2]);
    } else {
      time_limit_ms = 2000; // Default time limit 2 seconds
    }
    depth = std::atoi(argv[1]);
    time_limit_ms = std::atoi(argv[2]);
  } else {
    depth = 15; // Default depth
    time_limit_ms = 2000; // Default time limit 2 seconds
  }
  std::cout << "Using depth: " << depth
            << " and time limit: " << time_limit_ms << " ms" << std::endl;
  othello::initializeZobrist();
  othello::MobilityEvaluator evaluator;  // Create the mobility evaluator
  ProfilerStart("cpu_single_thread.prof");
  ProfilerDisable();
  utils::ThreadPool thread_pool(4); // Create a thread pool
  othello::Engine engine(evaluator, thread_pool);  // Create the engine with the evaluator
  othello::Controller controller(engine); // Create the controller with the evaluator
  controller.startGame(depth, time_limit_ms);  // Start the game loop
  return 0;                // Exit the program
}

