find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock REQUIRED)

# Auto-detect all test_*.cpp files
file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
)

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests
  GTest::gtest
  ${GMOCK_LIBRARY}
  othello_lib
)

set_target_properties(run_tests PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

include(GoogleTest)
gtest_discover_tests(run_tests)
