// Copyright Alex Li, 2025
// Controller.hpp
// Controller class for user input and game state management.

#pragma once

#include <set>
#include "Engine.hpp"  // For Engine
#include "GameBoard.hpp"

namespace othello {
/// @brief Controller class for managing user input and game state
class Controller {
 public:
  /// @brief Constructor for Controller
  /// @param engine The game engine to use for Othello
  Controller(Engine &engine)
      : engine(engine), board(createInitialBoard()) {};

  /// @brief Starts the game loop
  void startGame();

 private:
  Engine &engine;  ///< The game engine for Othello

  GameBoard board;  ///< The current game board

  /// @brief Handles user input for the game
  int handleUserInput(const std::set<int> &possible_moves);

  /// @brief Checks if the game is over and displays the result
  void checkGameOver();
};
}  // namespace othello

