// Copyright (c) Alex Li, 2025
// main.cpp
// Main entry point for the Othello game

#include <gperftools/profiler.h>

#include "othello/Controller.hpp"
#include "othello/GameBoard.hpp"
#include "othello/evaluator/Evaluator.hpp"

int main() {
  // Create the engine with a positional evaluator
  othello::initializeZobrist();
  othello::PositionalEvaluator evaluator;  // Create the positional evaluator
  ProfilerStart("cpu.prof");
  ProfilerDisable();
  utils::ThreadPool thread_pool(5); // Create a thread pool
  othello::Engine engine(evaluator, thread_pool);  // Create the engine with the evaluator
  othello::Controller controller(engine); // Create the controller with the evaluator
  controller.startGame();  // Start the game loop
  return 0;                // Exit the program
}

