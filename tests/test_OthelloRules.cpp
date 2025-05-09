// Copyright (c) 2025 Alex Li
// test_OthelloRules.cpp
// Test cases for OthelloRules functions

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/Visualize.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class StartingBoardState : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup code that will be called before each test
    board = std::make_shared<othello::GameBoard>();
  }
  
  std::shared_ptr<othello::GameBoard> board;
};

class IntermediateBoardState : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup code that will be called before each test
    board = std::make_shared<othello::GameBoard>();
    // Set up a specific board state for testing
    board->black_bb = 0x10100C000000ULL;
    board->white_bb = 0x80830000000ULL;
  }
  
  std::shared_ptr<othello::GameBoard> board;
};

// Test that the initial board is set up correctly
TEST_F(StartingBoardState, InitialBoardSetup) {
  EXPECT_EQ(board->black_bb, othello::INITIAL_BLACK);
  EXPECT_EQ(board->white_bb, othello::INITIAL_WHITE);
}

// Test valid moves for the initial board
TEST_F(StartingBoardState, PossibleMovesInitialBoard) {
  std::vector<int> possible_moves = othello::get_possible_moves(*board, othello::Color::BLACK);
  std::vector<int> expected_moves = {19, 26, 37, 44};
  
  EXPECT_THAT(possible_moves, ::testing::UnorderedElementsAreArray(expected_moves));
}

TEST_F(IntermediateBoardState, PossibleMovesIntermediateBoard) {
  std::vector<int> possible_moves = othello::get_possible_moves(*board, othello::Color::BLACK);
  std::vector<int> expected_moves = {20, 22, 30, 34, 42, 50, 51};
  EXPECT_THAT(possible_moves, ::testing::UnorderedElementsAreArray(expected_moves));

  std::vector<int> possible_moves_white = othello::get_possible_moves(*board, othello::Color::WHITE);
  std::vector<int> expected_moves_white = {17, 19, 25, 37, 45, 52, 53};
  EXPECT_THAT(possible_moves_white, ::testing::UnorderedElementsAreArray(expected_moves_white));
}

TEST_F(IntermediateBoardState, ValidMove) {
  EXPECT_TRUE(othello::is_valid_move(*board, 20, othello::Color::BLACK));
  EXPECT_FALSE(othello::is_valid_move(*board, 17, othello::Color::BLACK));
  EXPECT_TRUE(othello::is_valid_move(*board, 19, othello::Color::WHITE));
  EXPECT_FALSE(othello::is_valid_move(*board, 20, othello::Color::WHITE));
  EXPECT_TRUE(othello::is_valid_move(*board, 25, othello::Color::WHITE));
  EXPECT_FALSE(othello::is_valid_move(*board, 22, othello::Color::WHITE));
  EXPECT_FALSE(othello::is_valid_move(*board, 60, othello::Color::BLACK));
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

