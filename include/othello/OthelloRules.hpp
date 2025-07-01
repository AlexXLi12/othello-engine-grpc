// Copyright (c) 2025 Alex Li
// OthelloRules.hpp
// Performs logic for Othello game rules

#pragma once

#include <utility> // For std::pair
#include "othello/GameBoard.hpp"

namespace othello {

/// @brief Return a vector of all possible move positions for the given color 
/// @param b The game board to check for possible moves
/// @param color The color whose possible moves to check
/// @return A bitboard representing the possible moves
uint64_t getPossibleMoves(const GameBoard &b, Color color);

/// @brief Return whether a move is valid for the given color
/// @param b The game board to check for the move
/// @param position The position of the move to check
/// @param color The color of the player making the move
/// @return true if the move is valid, false otherwise
bool isValidMove(const GameBoard &b, int position, Color color);

/// @brief Return whether the game is over
/// @param b The game board to check for game over condition
/// @return true if the game is over, false otherwise
bool isTerminal(const GameBoard &b);

/// @brief  Return the disc count for each color
/// @param b The game board to check for disc counts
/// @return std::pair<int, int> representing the count of black and white discs
///         respectively.
std::pair<int, int> countDiscs(const GameBoard &b);

/// @brief Get the possible moves in the given shift direction
/// @param my_board The bitboard of the player's discs
/// @param op_board The bitboard of the opponent's discs
/// @param empty The bitboard of empty squares
/// @param shift The shift direction (positive for left, negative for right)
/// @param edge_mask The edge mask to apply to the bitboard
/// @return A bitfield of the possible moves in the given direction
inline uint64_t getDirectionalMoves(
  uint64_t my_board,
  uint64_t op_board,
  uint64_t empty,
  int shift,
  uint64_t edge_mask
) {
  auto shift_op = [&](uint64_t x) -> uint64_t {
    return shift > 0 ? (x & edge_mask) << shift : (x & edge_mask) >> -shift;
  };

  uint64_t t = shift_op(my_board) & op_board;

  for (int i = 0; i < 5; ++i) {
    t |= shift_op(t) & op_board;
  }

  return shift_op(t) & empty;
}

/// @brief Get bitfield of discs that can be flipped in the given direction
/// @param move The bitboard position of the move
/// @param my_board The bitboard of the player's discs
/// @param op_board The bitboard of the opponent's discs
/// @param empty The bitboard of empty squares
/// @param shift The shift direction (positive for left, negative for right)
/// @param edge_mask The edge mask to apply to the bitboard
/// @return A bitfield of the discs that can be flipped in the given direction
inline uint64_t getDirectionalFlips(
  uint64_t move,
  uint64_t my_board,
  uint64_t op_board,
  uint64_t empty,
  int shift,
  uint64_t edge_mask
) {
  auto shift_op = [&](uint64_t x) -> uint64_t {
    return shift > 0 ? (x & edge_mask) << shift : (x & edge_mask) >> -shift;
  };
  uint64_t flips = 0;
  while (true) {
    move = shift_op(move);
    if (move & op_board) {
      // still flipping opponent's discs
      flips |= move;
    } else if (move & my_board) {
      // reached our own disc; stop flipping
      break;
    } else {
      // we're on empty square; no flips in this direction
      return 0;
    }
  }
  return flips;
}

}  // namespace othello

