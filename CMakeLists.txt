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
  src/MobilityEvaluator.cpp
  src/Controller.cpp   # uses gperftools
  src/utils/Visualize.cpp
  src/utils/ThreadPool.cpp
)

add_executable(othello_exec src/main.cpp)

add_executable(othello_benchmark src/benchmark.cpp)

find_package(Threads REQUIRED)

# Help CMake/pkg-config find Homebrew stuff
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

set(LIBPROFILER_TARGET "")

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
  # Use an imported target so include dirs, link dirs, and libs propagate correctly
  pkg_check_modules(GPERF IMPORTED_TARGET libprofiler)
  if (TARGET PkgConfig::GPERF)
    set(LIBPROFILER_TARGET PkgConfig::GPERF)
  endif()
endif()

if (NOT LIBPROFILER_TARGET)
  include(FetchContent)
  include(ExternalProject)

  set(GPERFTOOLS_VERSION "2.15")
  FetchContent_Declare(gperftools
    URL https://github.com/gperftools/gperftools/releases/download/gperftools-${GPERFTOOLS_VERSION}/gperftools-${GPERFTOOLS_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_GetProperties(gperftools)
  if (NOT gperftools_POPULATED)
    FetchContent_Populate(gperftools)
  endif()

  find_program(LIBPROFILER_MAKE NAMES gmake make)
  if (NOT LIBPROFILER_MAKE)
    message(FATAL_ERROR "make is required to build libprofiler via FetchContent, but it was not found")
  endif()

  set(GPERF_INSTALL_DIR ${CMAKE_BINARY_DIR}/gperftools-install)
  file(MAKE_DIRECTORY ${GPERF_INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${GPERF_INSTALL_DIR}/lib)

  # Autotools configure script references headers via "src/..." paths,
  # so point CPPFLAGS at the source tree.
  set(GPERF_CONFIGURE_ENV "CPPFLAGS=-I${gperftools_SOURCE_DIR}")
  ExternalProject_Add(libprofiler_build
    SOURCE_DIR ${gperftools_SOURCE_DIR}
    BINARY_DIR ${gperftools_BINARY_DIR}
    INSTALL_DIR ${GPERF_INSTALL_DIR}
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${GPERF_CONFIGURE_ENV} ${gperftools_SOURCE_DIR}/configure --prefix=${GPERF_INSTALL_DIR} --disable-shared --enable-static
    BUILD_COMMAND ${LIBPROFILER_MAKE}
    INSTALL_COMMAND ${LIBPROFILER_MAKE} install
    BUILD_BYPRODUCTS ${GPERF_INSTALL_DIR}/lib/libprofiler.a
  )

  add_library(libprofiler_fetch STATIC IMPORTED GLOBAL)
  add_dependencies(libprofiler_fetch libprofiler_build)
  set_target_properties(libprofiler_fetch PROPERTIES
    IMPORTED_LOCATION ${GPERF_INSTALL_DIR}/lib/libprofiler.a
    INTERFACE_INCLUDE_DIRECTORIES ${GPERF_INSTALL_DIR}/include
  )

  set(LIBPROFILER_TARGET libprofiler_fetch)
endif()

if (NOT LIBPROFILER_TARGET)
  message(FATAL_ERROR "libprofiler could not be located or built")
endif()

# Controller.cpp includes profiler.h, so the lib needs the headers/flags.
# PUBLIC ensures othello_exec also links profiler.
target_link_libraries(othello_lib PUBLIC ${LIBPROFILER_TARGET} Threads::Threads)
target_include_directories(othello_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Executable just links the library; profiler comes via PUBLIC propagation
target_link_libraries(othello_exec PRIVATE othello_lib)

target_link_libraries(othello_benchmark PRIVATE othello_lib)

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
