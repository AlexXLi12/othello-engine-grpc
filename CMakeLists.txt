cmake_minimum_required(VERSION 3.20)
project(OthelloEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(othello_lib
  src/OthelloRules.cpp
  src/Engine.cpp
  src/GameBoard.cpp
  src/PositionalEvaluator.cpp
  src/Controller.cpp   # uses gperftools
  src/utils/Visualize.cpp
  src/utils/ThreadPool.cpp
)

add_executable(othello_exec src/main.cpp)

find_package(Threads REQUIRED)

# Help CMake/pkg-config find Homebrew stuff
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

find_package(PkgConfig REQUIRED)
# Use an imported target so include dirs, link dirs, and libs propagate correctly
pkg_check_modules(GPERF REQUIRED IMPORTED_TARGET libprofiler)

# Controller.cpp includes profiler.h, so the lib needs the headers/flags.
# PUBLIC ensures othello_exec also links profiler.
target_link_libraries(othello_lib PUBLIC PkgConfig::GPERF Threads::Threads)
target_include_directories(othello_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Executable just links the library; profiler comes via PUBLIC propagation
target_link_libraries(othello_exec PRIVATE othello_lib)

# Ensure runtime can find libprofiler.dylib (no DYLD_* hacks)
if(APPLE)
  execute_process(
    COMMAND brew --prefix gperftools
    OUTPUT_VARIABLE GPERF_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (GPERF_PREFIX)
    # Put rpath on the final binary
    set_target_properties(othello_exec PROPERTIES BUILD_RPATH "${GPERF_PREFIX}/lib")
  endif()
endif()

enable_testing()
add_subdirectory(tests)

