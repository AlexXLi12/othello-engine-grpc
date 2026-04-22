// Copyright (c) Alex Li, 2026
// main.cpp
// Main entry point for the Othello game

#include <charconv>
#include <iostream>
#include <string_view>
#include <system_error>

#include "othello/Controller.hpp"
#include "othello/evaluator/Evaluator.hpp"
#include "utils/Profiler.hpp"

namespace {

struct EngineOptions {
  int depth = 15;
  int time_limit_ms = 2000;
};

bool parseInt(std::string_view value, int &result) {
  const auto *begin = value.data();
  const auto *end = begin + value.size();
  const auto [ptr, error] = std::from_chars(begin, end, result);
  return error == std::errc{} && ptr == end;
}

bool parseOptions(int argc, char **argv, EngineOptions &options) {
  if (argc <= 1) {
    return true;
  }

  int parsed_depth = 0;
  if (!parseInt(argv[1], parsed_depth) || parsed_depth <= 0 ||
      parsed_depth > 60) {
    std::cerr << "Invalid depth. Must be between 1 and 60." << std::endl;
    return false;
  }
  options.depth = parsed_depth;

  if (argc <= 2) {
    return true;
  }

  if (!parseInt(argv[2], options.time_limit_ms) ||
      options.time_limit_ms <= 0) {
    std::cerr << "Invalid time limit. Must be greater than 0." << std::endl;
    return false;
  }

  return true;
}

}  // namespace

int main(int argc, char **argv) {
  EngineOptions options;
  if (!parseOptions(argc, argv, options)) {
    return 1;
  }

  std::cout << "Using depth: " << options.depth
            << " and time limit: " << options.time_limit_ms << " ms"
            << std::endl;
  othello::initializeZobrist();
  othello::MobilityEvaluator evaluator;  // Create the mobility evaluator
  utils::profiler::start("cpu_single_thread.prof");
  utils::profiler::disable();
  utils::ThreadPool thread_pool(4); // Create a thread pool
  othello::Engine engine(evaluator, thread_pool);  // Create the engine with the evaluator
  othello::Controller controller(engine); // Create the controller with the evaluator
  controller.startGame(options.depth, options.time_limit_ms);  // Start the game loop
  return 0;                // Exit the program
}
