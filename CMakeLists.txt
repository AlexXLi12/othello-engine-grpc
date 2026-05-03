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

add_executable(othello_server src/server.cpp)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)

pkg_check_modules(GPERF REQUIRED IMPORTED_TARGET libprofiler)
pkg_check_modules(GRPCPP REQUIRED IMPORTED_TARGET grpc++)

find_program(PROTOC protoc REQUIRED)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

set(PROTO_FILE ${PROJECT_SOURCE_DIR}/proto/engine.proto)
set(PROTO_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(PROTO_SRCS ${PROTO_GENERATED_DIR}/engine.pb.cc)
set(PROTO_HDRS ${PROTO_GENERATED_DIR}/engine.pb.h)
set(GRPC_SRCS ${PROTO_GENERATED_DIR}/engine.grpc.pb.cc)
set(GRPC_HDRS ${PROTO_GENERATED_DIR}/engine.grpc.pb.h)

add_custom_command(
  OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROTO_GENERATED_DIR}
  COMMAND ${PROTOC}
    --proto_path=${PROJECT_SOURCE_DIR}/proto
    --cpp_out=${PROTO_GENERATED_DIR}
    --grpc_out=${PROTO_GENERATED_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
    ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
)

add_library(engine_proto ${PROTO_SRCS} ${GRPC_SRCS})

target_include_directories(engine_proto PUBLIC ${PROTO_GENERATED_DIR})
target_link_libraries(engine_proto PUBLIC protobuf::libprotobuf PkgConfig::GRPCPP)

target_link_libraries(othello_lib PUBLIC PkgConfig::GPERF Threads::Threads)
target_include_directories(othello_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(othello_exec PRIVATE othello_lib)

target_link_libraries(othello_benchmark PRIVATE othello_lib)

target_link_libraries(othello_server PRIVATE othello_lib engine_proto)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
