// Copyright (c) 2025 Alex Li
// GameState.hpp
// Holds state information for a specific Othello game

#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "othello/GameBoard.hpp"
#include <vector>
#include <optional>
#include <cstdint> // for int8_t

namespace othello {

  /// @brief Represents a move in the Othello game
  /// @details Each move consists of a position, color, and a flag indicating
  ///          whether the move is a pass. The position is represented as an
  ///         integer, where the board is indexed from 0 to 63 (for an 8x8 board).
  struct Move {
    int8_t position;  // the position of the move on the board
    Color color;  // the color of the player making the move
    bool is_pass;  // flag to indicate if the move is a pass
  };

  /// @brief Represents the state of the Othello game
  /// @details The GameState struct holds the current game board, a log of moves,
  ///         the color of the player whose turn it is, the winner (if any), and
  ///        a flag indicating if the game is over. It also keeps track of the scores
  ///        for both players.
  struct GameState {
    GameBoard board;  // The current state of the game board
    std::vector<Move> move_log;  // Log of moves made in the game
    Color to_move;  // The color of the player whose turn it is to move
    std::optional<Color> winner = std::nullopt;  // The winner of the game (if any)
    bool game_over;  // Flag to indicate if the game is over
    int8_t black_score = 0;  // Score for black player
    int8_t white_score = 0;  // Score for white player
    void process_move(const Move &move);  // Process a move and update the game state accordingly
  };

}

#endif  // GAMESTATE_HPP
