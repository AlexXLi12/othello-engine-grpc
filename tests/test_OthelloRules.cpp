// Copyright (c) 2025 Alex Li
// test_OthelloRules.cpp
// Test cases for OthelloRules functions

#include "GameBoard.hpp"
#include "OthelloRules.hpp"
#include <gtest/gtest.h>

class OthelloRulesTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup code that will be called before each test
    board = std::make_shared<GameBoard>();
  }
  
  std::shared_ptr<GameBoard> board;
};

// Test that the initial board is set up correctly
TEST_F(OthelloRulesTest, InitialBoardSetup) {
  EXPECT_EQ(board->getPiece(3, 3), GameBoard::WHITE);
  EXPECT_EQ(board->getPiece(3, 4), GameBoard::BLACK);
  EXPECT_EQ(board->getPiece(4, 3), GameBoard::BLACK);
  EXPECT_EQ(board->getPiece(4, 4), GameBoard::WHITE);
  
  // Check that the corners are empty
  EXPECT_EQ(board->getPiece(0, 0), GameBoard::EMPTY);
  EXPECT_EQ(board->getPiece(0, 7), GameBoard::EMPTY);
  EXPECT_EQ(board->getPiece(7, 0), GameBoard::EMPTY);
  EXPECT_EQ(board->getPiece(7, 7), GameBoard::EMPTY);
}

// Test valid moves for the initial board
TEST_F(OthelloRulesTest, ValidMovesInitialBoard) {
  // In standard Othello, BLACK goes first and has 4 valid moves
  std::vector<std::pair<int, int>> validMoves = rules->getValidMoves(*board, GameBoard::BLACK);
  EXPECT_EQ(validMoves.size(), 4);
  
  // TODO: Check the specific valid moves
}

// Add more test cases here

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

