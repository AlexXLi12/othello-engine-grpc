// Copyright (c) 2026, Alex Li
// benchmark.cpp
// Source code for benchmarking engine performance under repeatable scenarios.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "othello/Engine.hpp"
#include "othello/GameBoard.hpp"
#include "othello/OthelloRules.hpp"
#include "othello/evaluator/Evaluator.hpp"
#include "utils/BitboardUtils.hpp"
#include "utils/Profiler.hpp"
#include "utils/ThreadPool.hpp"

namespace {

enum class OutputFormat { Text, Csv, Json };

struct Config {
  std::vector<int> depths{1, 5, 10, 13};
  int positions = 10;
  int plies = 20;
  int threads = 5;
  int time_limit_ms = std::numeric_limits<int>::max();
  uint64_t seed = 1738;
  OutputFormat format = OutputFormat::Text;
  std::string profile_file = "cpu_profile.prof";
};

struct Result {
  int depth = 0;
  int position_index = 0;
  int plies = 0;
  int threads = 0;
  uint64_t seed = 0;
  int best_move = -1;
  int score = 0;
  double elapsed_ms = 0.0;
  int nodes_searched = 0;
  int cache_hits = 0;
  double nodes_per_sec = 0.0;
  int completed_depth = 0;
  bool time_limit_hit = false;
};

int parsePositiveInt(const std::string &value, const std::string &name) {
  size_t parsed = 0;
  int result = std::stoi(value, &parsed);
  if (parsed != value.size() || result <= 0) {
    throw std::invalid_argument(name + " must be a positive integer");
  }
  return result;
}

int parseNonNegativeInt(const std::string &value, const std::string &name) {
  size_t parsed = 0;
  int result = std::stoi(value, &parsed);
  if (parsed != value.size() || result < 0) {
    throw std::invalid_argument(name + " must be a non-negative integer");
  }
  return result;
}

uint64_t parseSeed(const std::string &value) {
  size_t parsed = 0;
  unsigned long long result = std::stoull(value, &parsed);
  if (parsed != value.size()) {
    throw std::invalid_argument("seed must be an unsigned integer");
  }
  return static_cast<uint64_t>(result);
}

std::vector<int> parseDepths(const std::string &value) {
  std::vector<int> depths;
  std::stringstream stream(value);
  std::string item;
  while (std::getline(stream, item, ',')) {
    if (item.empty()) {
      throw std::invalid_argument("depth list contains an empty item");
    }
    int depth = parsePositiveInt(item, "depth");
    if (depth > std::numeric_limits<uint8_t>::max()) {
      throw std::invalid_argument("depth must fit in uint8_t");
    }
    depths.push_back(depth);
  }
  if (depths.empty()) {
    throw std::invalid_argument("depth list must not be empty");
  }
  return depths;
}

OutputFormat parseFormat(const std::string &value) {
  if (value == "text") {
    return OutputFormat::Text;
  }
  if (value == "csv") {
    return OutputFormat::Csv;
  }
  if (value == "json") {
    return OutputFormat::Json;
  }
  throw std::invalid_argument("format must be one of: text, csv, json");
}

void printUsage(const char *program) {
  std::cout
      << "Usage: " << program << " [options]\n"
      << "\n"
      << "Options:\n"
      << "  --depth N              Run one search depth\n"
      << "  --depths A,B,C         Run multiple search depths\n"
      << "  --positions N          Number of deterministic positions per depth\n"
      << "  --plies N              Random legal plies used to create each position\n"
      << "  --threads N            Thread pool size\n"
      << "  --time-limit-ms N      Per-search time limit\n"
      << "  --seed N               Deterministic board-generation seed\n"
      << "  --format text|csv|json Output format\n"
      << "  --profile-file NAME    Profile filename under OTHELLO_PROFILE_DIR\n"
      << "  --help                 Show this help\n";
}

Config parseArgs(int argc, char **argv) {
  Config config;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    auto requireValue = [&](const std::string &name) -> std::string {
      if (i + 1 >= argc) {
        throw std::invalid_argument(name + " requires a value");
      }
      return argv[++i];
    };

    if (arg == "--help" || arg == "-h") {
      printUsage(argv[0]);
      std::exit(0);
    } else if (arg == "--depth") {
      config.depths = parseDepths(requireValue(arg));
    } else if (arg == "--depths") {
      config.depths = parseDepths(requireValue(arg));
    } else if (arg == "--positions") {
      config.positions = parsePositiveInt(requireValue(arg), "positions");
    } else if (arg == "--plies") {
      config.plies = parseNonNegativeInt(requireValue(arg), "plies");
    } else if (arg == "--threads") {
      config.threads = parsePositiveInt(requireValue(arg), "threads");
    } else if (arg == "--time-limit-ms") {
      config.time_limit_ms =
          parsePositiveInt(requireValue(arg), "time-limit-ms");
    } else if (arg == "--seed") {
      config.seed = parseSeed(requireValue(arg));
    } else if (arg == "--format") {
      config.format = parseFormat(requireValue(arg));
    } else if (arg == "--profile-file") {
      config.profile_file = requireValue(arg);
      if (config.profile_file.empty()) {
        throw std::invalid_argument("profile-file must not be empty");
      }
    } else {
      throw std::invalid_argument("unknown option: " + arg);
    }
  }

  return config;
}

std::vector<othello::GameBoard> getRandomBoards(int num_boards,
                                                int moves_per_board,
                                                uint64_t seed) {
  std::vector<othello::GameBoard> boards;
  boards.reserve(static_cast<size_t>(num_boards));
  std::mt19937_64 rng(seed);
  for (int i = 0; i < num_boards; ++i) {
    othello::GameBoard board = othello::createInitialBoard();
    othello::Color color = othello::Color::BLACK;
    for (int j = 0; j < moves_per_board; ++j) {
      std::vector<int> moves =
          othello::bitboard_to_positions(othello::getPossibleMoves(board, color));
      if (moves.empty()) {
        color = othello::opponent(color);
        moves =
            othello::bitboard_to_positions(othello::getPossibleMoves(board, color));
        if (moves.empty()) {
          break;
        }
      }
      int move = moves[rng() % moves.size()];
      board = othello::applyMove(board, move, color);
      color = othello::opponent(color);
    }
    boards.push_back(board);
  }
  return boards;
}

std::vector<Result> runBenchmark(const Config &config) {
  othello::initializeZobrist();

  othello::MobilityEvaluator evaluator;
  utils::ThreadPool thread_pool(static_cast<size_t>(config.threads));
  othello::Engine engine(evaluator, thread_pool);
  engine.setVerbose(false);

  const auto boards = getRandomBoards(config.positions, config.plies, config.seed);
  std::vector<Result> results;
  results.reserve(config.depths.size() * boards.size());

  utils::profiler::start(config.profile_file.c_str());
  for (int depth : config.depths) {
    for (size_t position = 0; position < boards.size(); ++position) {
      auto start = std::chrono::steady_clock::now();
      int best_move = engine.findBestMove(
          boards[position], static_cast<uint8_t>(depth), othello::Color::BLACK,
          config.time_limit_ms);
      auto end = std::chrono::steady_clock::now();

      const std::chrono::duration<double, std::milli> elapsed = end - start;
      const othello::SearchStats stats = engine.lastSearchStats();
      const double elapsed_seconds = elapsed.count() / 1000.0;

      results.push_back(Result{
          .depth = depth,
          .position_index = static_cast<int>(position),
          .plies = config.plies,
          .threads = config.threads,
          .seed = config.seed,
          .best_move = best_move,
          .score = stats.score,
          .elapsed_ms = elapsed.count(),
          .nodes_searched = stats.nodes_searched,
          .cache_hits = stats.cache_hits,
          .nodes_per_sec = elapsed_seconds > 0.0
                               ? stats.nodes_searched / elapsed_seconds
                               : 0.0,
          .completed_depth = stats.completed_depth,
          .time_limit_hit = stats.time_limit_hit,
      });
    }
  }
  utils::profiler::stop();

  return results;
}

void printCsv(const std::vector<Result> &results) {
  std::cout << "depth,position_index,plies,threads,seed,best_move,score,"
               "elapsed_ms,nodes_searched,cache_hits,nodes_per_sec,"
               "completed_depth,time_limit_hit\n";
  std::cout << std::fixed << std::setprecision(3);
  for (const Result &result : results) {
    std::cout << result.depth << ',' << result.position_index << ','
              << result.plies << ',' << result.threads << ',' << result.seed
              << ',' << result.best_move << ',' << result.score << ','
              << result.elapsed_ms << ',' << result.nodes_searched << ','
              << result.cache_hits << ',' << result.nodes_per_sec << ','
              << result.completed_depth << ','
              << (result.time_limit_hit ? "true" : "false") << '\n';
  }
}

void printJson(const std::vector<Result> &results) {
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "[\n";
  for (size_t i = 0; i < results.size(); ++i) {
    const Result &result = results[i];
    std::cout << "  {"
              << "\"depth\":" << result.depth << ","
              << "\"position_index\":" << result.position_index << ","
              << "\"plies\":" << result.plies << ","
              << "\"threads\":" << result.threads << ","
              << "\"seed\":" << result.seed << ","
              << "\"best_move\":" << result.best_move << ","
              << "\"score\":" << result.score << ","
              << "\"elapsed_ms\":" << result.elapsed_ms << ","
              << "\"nodes_searched\":" << result.nodes_searched << ","
              << "\"cache_hits\":" << result.cache_hits << ","
              << "\"nodes_per_sec\":" << result.nodes_per_sec << ","
              << "\"completed_depth\":" << result.completed_depth << ","
              << "\"time_limit_hit\":"
              << (result.time_limit_hit ? "true" : "false") << "}";
    if (i + 1 < results.size()) {
      std::cout << ',';
    }
    std::cout << '\n';
  }
  std::cout << "]\n";
}

void printText(const std::vector<Result> &results) {
  std::cout << std::fixed << std::setprecision(3);
  for (const Result &result : results) {
    std::cout << "depth=" << result.depth
              << " position=" << result.position_index
              << " elapsed_ms=" << result.elapsed_ms
              << " nodes=" << result.nodes_searched
              << " nodes_per_sec=" << result.nodes_per_sec
              << " cache_hits=" << result.cache_hits
              << " best_move=" << result.best_move
              << " score=" << result.score
              << " completed_depth=" << result.completed_depth
              << " time_limit_hit="
              << (result.time_limit_hit ? "true" : "false") << '\n';
  }

  std::vector<int> depths;
  for (const Result &result : results) {
    if (std::find(depths.begin(), depths.end(), result.depth) == depths.end()) {
      depths.push_back(result.depth);
    }
  }

  std::cout << "\nsummary\n";
  for (int depth : depths) {
    double total_ms = 0.0;
    double total_nodes = 0.0;
    int count = 0;
    for (const Result &result : results) {
      if (result.depth == depth) {
        total_ms += result.elapsed_ms;
        total_nodes += result.nodes_searched;
        ++count;
      }
    }
    const double avg_ms = count > 0 ? total_ms / count : 0.0;
    const double avg_nodes = count > 0 ? total_nodes / count : 0.0;
    const double nodes_per_sec =
        total_ms > 0.0 ? (total_nodes / (total_ms / 1000.0)) : 0.0;
    std::cout << "depth=" << depth << " runs=" << count
              << " avg_ms=" << avg_ms
              << " avg_nodes=" << avg_nodes
              << " aggregate_nodes_per_sec=" << nodes_per_sec << '\n';
  }
}

void printResults(const Config &config, const std::vector<Result> &results) {
  switch (config.format) {
  case OutputFormat::Text:
    printText(results);
    break;
  case OutputFormat::Csv:
    printCsv(results);
    break;
  case OutputFormat::Json:
    printJson(results);
    break;
  }
}

} // namespace

int main(int argc, char **argv) {
  try {
    const Config config = parseArgs(argc, argv);
    const std::vector<Result> results = runBenchmark(config);
    printResults(config, results);
  } catch (const std::exception &error) {
    std::cerr << "benchmark error: " << error.what() << '\n';
    std::cerr << "Run with --help for usage.\n";
    return 1;
  }

  return 0;
}
