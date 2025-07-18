// Copyright (c) 2025 Alex Li
// Gameboard.hpp
// Holds the game board representation for Othello

#pragma once

#include <array>    // For std::array
#include <cstdint>  // For uint64_t

#include "Constants.hpp"  // For bitboard constants

namespace othello {

using ZobristTable = std::array<std::array<uint64_t, 2>, 64>;

extern ZobristTable zobrist_table;
extern uint64_t zobrist_black_turn;

/// @brief Represents the color of a piece on the Othello board
/// @details The colors are represented as an enum class for type safety.
enum class Color : int {
  BLACK = 1,
  WHITE = -1,
};

/// @brief Generate a zobrist hash for the game board
/// @param black_bb (uint64_t) : The bitboard for black pieces
/// @param white_bb (uint64_t) : The bitboard for white pieces
/// @return int : The Zobrist hash for the board state
uint64_t zobristHash(uint64_t black_bb, uint64_t white_bb, Color turn);

/// @brief Represents the game board for Othello
/// @details The game board is represented using bitboards for both players.
///          Each bitboard is a 64-bit unsigned integer, where each bit
///          represents a square on the 8x8 board. A bit is set to 1 if the
///          corresponding square is occupied by a piece of that color, and 0 if
///          it is empty. The top-left corner of the board is represented by the
///          least significant bit (LSB), and the bottom-right corner is
///          represented by the most significant bit (MSB). The initial state of
///          the board is set to the standard Othello starting position.
struct GameBoard {
  uint64_t black_bb;      ///< Bitboard for black pieces
  uint64_t white_bb;      ///< Bitboard for white pieces
  Color current_turn;     ///< The color of the player to move
  uint64_t zobrist_hash;  ///< Zobrist hash for the board state

  GameBoard(uint64_t black,
            uint64_t white, Color turn,
            uint64_t hash)
      : black_bb(black),
        white_bb(white),
        current_turn(turn),
        zobrist_hash(hash) {}
};

/// @brief Factory function to create the initial game board
inline GameBoard createInitialBoard() {
  return GameBoard(
    INITIAL_BLACK,
    INITIAL_WHITE,
    Color::BLACK,
    zobristHash(INITIAL_BLACK, INITIAL_WHITE, Color::BLACK)
  );
}


/// @brief Returns the opponent color for a given color
/// @param c The color for which to find the opponent
/// @return The opponent color (BLACK or WHITE)
inline Color opponent(Color c) {
  return static_cast<Color>(-static_cast<int>(c));
}

/// @brief Apply the move to the game board and return a new game board
/// @details This function assumes that the move is valid.
///          Passing in an invalid move will result in undefined behavior.
/// @param b The game board to apply the move to
/// @param position The position of the move to apply
/// @param color The color of the player making the move
/// @param prev_passed Whether the previous player passed their turn
/// @return A new GameBoard with the move applied
GameBoard applyMove(const GameBoard &b, int position, Color color, 
                    bool prev_passed);
}  // namespace othello

