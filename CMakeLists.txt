cmake_minimum_required(VERSION 3.14)
project(OthelloEngine)

# enable C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
add_library (othello_lib
  src/OthelloRules.cpp
  src/GameBoard.cpp
  src/GameState.cpp
)

target_include_directories(othello_lib PUBLIC src)

enable_testing()
add_subdirectory(tests)