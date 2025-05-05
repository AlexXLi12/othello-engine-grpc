// Copyright (c) 2025 Alex Li
// test_OthelloRules.cpp
// Test cases for OthelloRules functions

#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "utils/Visualize.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class OthelloRulesTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup code that will be called before each test
    board = std::make_shared<othello::GameBoard>();
  }
  
  std::shared_ptr<othello::GameBoard> board;
};

// Test that the initial board is set up correctly
TEST_F(OthelloRulesTest, InitialBoardSetup) {
  EXPECT_EQ(board->black_bb, othello::INITIAL_BLACK);
  EXPECT_EQ(board->white_bb, othello::INITIAL_WHITE);
}

// Test valid moves for the initial board
TEST_F(OthelloRulesTest, PossibleMovesInitialBoard) {
  std::vector<int> possible_moves = othello::get_possible_moves(*board, othello::Color::BLACK);
  std::vector<int> expected_moves = {19, 26, 37, 44};
  
  EXPECT_THAT(possible_moves, ::testing::UnorderedElementsAreArray(expected_moves));
}

// Add more test cases here

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

