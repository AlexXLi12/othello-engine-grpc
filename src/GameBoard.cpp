// Copyright (c) 2025 Alex Li
// GameBoard.cpp
// Implements functions defined in GameBoard.hpp

#include "othello/GameBoard.hpp"

#include <chrono>  // For time-based seeding
#include <random>  // For random number generation
#include <bit>

#include "othello/OthelloRules.hpp"  // For isValidMove

namespace {
/// @brief Update the zobrist hash for a move
/// @param hash (uint64_t) : The current Zobrist hash
/// @param position (int) : The position of the move
/// @param flip_bb (uint64_t) : The bitboard representing the pieces that will
/// be flipped
/// @param color (Color) : The color of the player making the move
/// @return uint64_t : The updated Zobrist hash after the move
uint64_t updateZobristHash(uint64_t hash, int position, uint64_t flip_bb,
                           othello::Color color) {
  if (color == othello::Color::BLACK) {
    hash ^= othello::zobrist_table[position][0];  // Black piece
  } else {
    hash ^= othello::zobrist_table[position][1];  // White piece
  }
  while (flip_bb) {
    int flip_pos = std::countr_zero(flip_bb);
    // Flip the piece
    hash ^= othello::zobrist_table[flip_pos][0];
    hash ^= othello::zobrist_table[flip_pos][1];
    flip_bb &= (flip_bb - 1);  // Clear the least significant bit
  }
  return hash;
}
}  // namespace

namespace othello {

ZobristTable zobrist_table;
uint64_t zobrist_black_turn;

GameBoard applyMove(const GameBoard &b, int position, Color color) {
  uint64_t my_board = color == Color::BLACK ? b.black_bb : b.white_bb;
  uint64_t op_board = color == Color::BLACK ? b.white_bb : b.black_bb;
  uint64_t empty = ~(my_board | op_board);
  uint64_t pos_board = 1ULL << position;

  uint64_t flips = getDirectionalFlips(pos_board, my_board, op_board, empty, -1,
                                       othello::LEFT_EDGE_MASK);  // West

  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, 1,
                               othello::RIGHT_EDGE_MASK);  // East

  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, 8,
                               othello::BOTTOM_EDGE_MASK);  // South

  flips |= getDirectionalFlips(pos_board, my_board, op_board, empty, -8,
                               othello::TOP_EDGE_MASK);  // North

  flips |= getDirectionalFlips(
      pos_board, my_board, op_board, empty, -7,
      othello::TOP_EDGE_MASK & othello::RIGHT_EDGE_MASK);  // North-East

  flips |= getDirectionalFlips(
      pos_board, my_board, op_board, empty, -9,
      othello::TOP_EDGE_MASK & othello::LEFT_EDGE_MASK);  // North-West

  flips |= getDirectionalFlips(
      pos_board, my_board, op_board, empty, 7,
      othello::BOTTOM_EDGE_MASK & othello::LEFT_EDGE_MASK);  // South-West

  flips |= getDirectionalFlips(
      pos_board, my_board, op_board, empty, 9,
      othello::BOTTOM_EDGE_MASK & othello::RIGHT_EDGE_MASK);  // South-East

  my_board = my_board | pos_board | flips;
  op_board ^= flips;
  uint64_t new_hash = updateZobristHash(b.zobrist_hash, position, flips, color);
  uint64_t new_black = color == Color::BLACK ? my_board : op_board;
  uint64_t new_white = color == Color::BLACK ? op_board : my_board;
  Color next_player = color;
  if (getPossibleMoves(GameBoard(new_black, new_white, 0, color),
                       opponent(color)) != 0) {
    next_player = opponent(color);
    new_hash ^= zobrist_black_turn;  // Switch turn only if opponent has moves
  }
  return GameBoard(new_black, new_white, new_hash, next_player);
}

void initializeZobrist() {
  std::mt19937_64 rng(
      std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<uint64_t> dist;
  for (int i = 0; i < 64; ++i) {
    for (int j = 0; j < 2; ++j) {
      zobrist_table[i][j] = dist(rng);
    }
  }
  zobrist_black_turn = dist(rng);
}

/// @brief Generate a Zobrist hash for the game board
/// @param black_bb (uint64_t) : The bitboard for black pieces
/// @param white_bb (uint64_t) : The bitboard for white pieces
/// @param turn (Color) : The color of the player to move
/// @return uint64_t : The Zobrist hash for the board state
uint64_t zobristHash(uint64_t black_bb, uint64_t white_bb, Color turn) {
  uint64_t hash = 0;
  while (black_bb) {
    int pos = std::countr_zero(
        black_bb);  // Get the index of the least significant bit
    hash ^= zobrist_table[pos][0];  // XOR with the black piece hash
    black_bb &= (black_bb - 1);     // Clear the least significant bit
  }
  while (white_bb) {
    int pos = std::countr_zero(
        white_bb);  // Get the index of the least significant bit
    hash ^= zobrist_table[pos][1];  // XOR with the white piece hash
    white_bb &= (white_bb - 1);     // Clear the least significant bit
  }
  if (turn == Color::BLACK) {
    hash ^= zobrist_black_turn;  // Add turn information
  }
  return hash;
}

}  // namespace othello

