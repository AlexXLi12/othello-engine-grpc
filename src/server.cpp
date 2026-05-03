// Copyright (c) 2026 Alex Li
// Simple gRPC server entry point for the Othello engine.

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "engine.grpc.pb.h"
#include "othello/Engine.hpp"
#include "othello/GameBoard.hpp"
#include "othello/evaluator/Evaluator.hpp"

namespace {

constexpr uint8_t kDefaultDepthLimit = 8;
constexpr int kDefaultTimeLimitMs = 1000;
constexpr int kSearchThreads = 4;

std::string serverAddress() {
  const char *address = std::getenv("OTHELLO_SERVER_ADDRESS");
  if (address == nullptr || std::string(address).empty()) {
    return "0.0.0.0:50051";
  }
  return address;
}

uint8_t depthLimit(const engine::FindBestMoveRequest &request) {
  const uint32_t requested = request.depth_limit();
  if (requested == 0) {
    return kDefaultDepthLimit;
  }
  return static_cast<uint8_t>(std::min<uint32_t>(requested, 60));
}

int timeLimitMs(const engine::FindBestMoveRequest &request) {
  if (request.time_limit_ms() == 0) {
    return kDefaultTimeLimitMs;
  }
  return static_cast<int>(request.time_limit_ms());
}

class EngineService final : public engine::EngineService::Service {
 public:
  grpc::Status FindBestMove(grpc::ServerContext *context,
                            const engine::FindBestMoveRequest *request,
                            engine::FindBestMoveResponse *response) override {
    (void)context;

    const othello::Color color = request->game_state().black_to_move()
                                     ? othello::Color::BLACK
                                     : othello::Color::WHITE;
    const auto board = othello::GameBoard(
        request->game_state().black_bb(), request->game_state().white_bb(),
        othello::zobristHash(request->game_state().black_bb(),
                             request->game_state().white_bb(), color),
        color);

    const int best_move =
        engine_.findBestMove(board, depthLimit(*request), color,
                             timeLimitMs(*request));

    response->set_best_move(best_move);
    response->set_eval_score(evaluationAfterMove(board, best_move, color));
    return grpc::Status::OK;
  }

 private:
  int evaluationAfterMove(const othello::GameBoard &board, int best_move,
                          othello::Color color) const {
    if (best_move < 0) {
      return evaluator_.evaluate(board);
    }
    return evaluator_.evaluate(othello::applyMove(board, best_move, color));
  }

  othello::MobilityEvaluator evaluator_;
  utils::ThreadPool thread_pool_{kSearchThreads};
  othello::Engine engine_{evaluator_, thread_pool_};
};

}  // namespace

int main() {
  othello::initializeZobrist();

  const std::string address = serverAddress();
  EngineService service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (server == nullptr) {
    std::cerr << "Failed to start Othello server on " << address << std::endl;
    return 1;
  }

  std::cout << "Othello gRPC server listening on " << address << std::endl;
  server->Wait();
  return 0;
}
