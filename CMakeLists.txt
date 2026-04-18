cmake_minimum_required(VERSION 3.20)
project(OthelloEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OTHELLO_DOCKER_BUILD "Permit configuring this Docker-only project" OFF)

if (NOT OTHELLO_DOCKER_BUILD)
  message(FATAL_ERROR "This project is Docker-only. Use `docker compose up --build engine` or build through the Dockerfile.")
endif()

add_library(othello_lib
  src/OthelloRules.cpp
  src/Engine.cpp
  src/GameBoard.cpp
  src/PositionalEvaluator.cpp
  src/MobilityEvaluator.cpp
  src/Controller.cpp   # uses gperftools
  src/utils/Visualize.cpp
  src/utils/ThreadPool.cpp
)

add_executable(othello_exec src/main.cpp)

add_executable(othello_benchmark src/benchmark.cpp)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GPERF REQUIRED IMPORTED_TARGET libprofiler)

target_link_libraries(othello_lib PUBLIC PkgConfig::GPERF Threads::Threads)
target_include_directories(othello_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(othello_exec PRIVATE othello_lib)

target_link_libraries(othello_benchmark PRIVATE othello_lib)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
