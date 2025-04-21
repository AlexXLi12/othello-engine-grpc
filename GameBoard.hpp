// Copyright (c) 2025 Alex Li
// Gameboard.hpp
// Holds the game board representation for Othello

#ifndef GAME_BOARD_HPP
#define GAME_BOARD_HPP

#include <cstdint> // For uint64_t
#include <utility> // For std::unreachable

namespace othello {

  /// @brief Represents the color of a piece on the Othello board
  /// @details The colors are represented as an enum class for type safety.
  ///          BLACK and WHITE represent the two players, while NONE is used
  ///          to indicate an empty square on the board.
  enum class Color
  {
    BLACK,
    WHITE,
    NONE,  // to handle when no winner
  };

  /// @brief Returns the opponent color for a given color
  /// @param c The color for which to find the opponent
  /// @return The opponent color (BLACK or WHITE)
  inline Color opponent(Color c) {
    switch (c) {
      case Color::BLACK: return Color::WHITE;
      case Color::WHITE: return Color::BLACK;
    }
    std::unreachable(); // This should never happen
  }

  /// @brief Represents the game board for Othello
  /// @details The game board is represented using bitboards for both players.
  ///          Each bitboard is a 64-bit unsigned integer, where each bit represents
  ///          a square on the 8x8 board. A bit is set to 1 if the corresponding square
  ///          is occupied by a piece of that color, and 0 if it is empty.
  struct GameBoard {
    uint64_t black_bb = 0; // Bitboard for black pieces
    uint64_t white_bb = 0; // Bitboard for white pieces
  };

}  // namespace othello

#endif  // GAME_BOARD_HPP
