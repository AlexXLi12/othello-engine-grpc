include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

# Auto-detect all test_*.cpp files
file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
)

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests
  gtest_main
  othello_lib
)

set_target_properties(run_tests PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

include(GoogleTest)
gtest_discover_tests(run_tests)