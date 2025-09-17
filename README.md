# Othello Engine (C++23) — gRPC WIP

High‑performance Othello/Reversi engine written in modern C++ with a focus on speed, memory efficiency, and clean architecture. The engine uses bitboards for constant‑time move operations (get possible moves, apply move), a Zobrist‑keyed transposition table, cache‑conscious data layouts, and parallel search.

Note: The gRPC service is not yet completed. The protobuf contract lives in `proto/engine.proto`; the C++ gRPC server/client implementation is still in progress.


## Highlights

- Bitboards: Compact 64‑bit representation enables fast, branch‑light move generation and flipping.
- Transposition Table + Zobrist: Caches search results with compact entries keyed by Zobrist hashes.
- Cache‑optimized Layouts: Small, tightly‑packed structs for hot data; minimized indirection and allocations in inner loops.
- Parallel Search: Work is split across a thread pool with PVS/YBW‑style coordination and lock‑free alpha updates.
- Modern C++23: Clean, strong‑typed interfaces; standard atomics and utilities; portable and testable.


## Performance Deep Dive

### Bitboards and Constant‑Time Move Checks

- Representation: Each side’s pieces live in a `uint64_t` bitboard. Masking and bit operations (shift, AND, OR) naturally encode Othello’s 8 directions on an 8×8 board.
- Move Gen: Directional scans use shift operators with edge masks to accumulate flanked runs in O(1) per direction without per‑square loops:
  - `getDirectionalMoves` computes legal destinations by iteratively extending through opponent discs and stopping at empties.
  - `getDirectionalFlips` finds the exact pieces to flip for a specific move, again via directional shifts and masks.
- Popcount‑centric: Counting mobility and features relies on `std::popcount`, which maps to efficient instructions on modern CPUs.
- Heuristics by Masks: Corners, edges, X/C/A/B squares are pre‑encoded as masks, enabling fast static evaluation without per‑cell branching.

Files of interest:
- `include/othello/OthelloRules.hpp` + `src/OthelloRules.cpp`
- `include/othello/Constants.hpp`


### Transposition Table with Zobrist Hashing

- Zobrist Keys: Each (square, color) pair has a random 64‑bit value, XOR‑combined to key positions; side‑to‑move is included via a side key.
- Incremental Hashing: Applying a move updates the hash by XOR‑ing the placed piece and each flipped piece (constant‑time per flipped bit).
- Compact Entries: `TTEntry` is intentionally small to improve cache residency:
  - `score` (int), `depth` (uint8_t), `bound_type` (enum), `move_index` (int8_t) → 8 bytes total in practice.
- Bound‑aware Hits: Entries store EXACT/LOWER/UPPER bounds; probes can tighten alpha/beta or return exact scores to prune aggressively.
- Replacement/Partitioning: Simple replacement on store with depth awareness via probe logic; a table is reserved per root move to reduce contention in parallel search.

Files of interest:
- `include/othello/GameBoard.hpp`, `src/GameBoard.cpp` (Zobrist init/hash/incremental updates)
- `include/othello/Engine.hpp`, `src/Engine.cpp` (`TTEntry`, probing, storing)


### Cache‑Optimized Structs and Data Flow

- `GameBoard` packs hot fields contiguously: `black_bb`, `white_bb`, `zobrist_hash`, `current_turn`.
- `TTEntry` is kept tiny to maximize entries per cache line, improving hit rate during deep searches.
- Avoided indirection/allocation in inner loops; vectors are reserved when sizes are known; per‑depth/per‑root structures reduce churn.
- Move ordering minimizes work: corners first, then edges, then the rest; the transposition move, if present, is promoted to the front.

Files of interest:
- `include/othello/GameBoard.hpp`
- `src/Engine.cpp` (move ordering and layout‑sensitive loops)


### Multithreaded Search (Thread Pool + PVS/YBW)

- Parallelization Strategy: Iterative deepening at the root; seed the first child to establish a useful alpha, then search sibling moves in parallel using a work queue.
- Thread Pool: A compact, generic pool with futures for result collection. Work is enqueued as lambdas capturing minimal state.
- Coordination:
  - Alpha is shared via an `std::atomic<int>`; workers perform scout (zero‑window) probes first and only re‑search on fail‑high.
  - This mirrors Principal Variation Search (PVS) and Young Brothers Wait (YBW) ideas to keep parallel work productive and minimize wasted full‑window searches.
- Contention‑Aware TT: Each root move uses its own `unordered_map<uint64_t, TTEntry>`, avoiding heavy synchronization on a single global table during parallel search.

Files of interest:
- `include/utils/ThreadPool.hpp`, `src/utils/ThreadPool.cpp`
- `src/Engine.cpp` (parallel root, zero‑window probes, atomic alpha)


## Architecture Overview

- Core library (`othello_lib`)
  - Rules: `OthelloRules` (move gen/validation, terminal checks)
  - State: `GameBoard` (bitboards + Zobrist), `Constants` (masks)
  - Engine: `Engine` (negamax + alpha‑beta, PVS, TT)
  - Evaluation: `PositionalEvaluator`, `MobilityEvaluator`
  - Utils: `ThreadPool`, `Visualize`, `BitboardUtils`
- Executables
  - `othello_exec`: Interactive/CLI loop via `Controller`
  - `othello_benchmark`: Deterministic micro‑benchmarks across depths
- Tests: `tests/` with GoogleTest; discovered via CTest
- Proto (gRPC WIP): `proto/engine.proto` defines `EngineService.FindBestMove`


## Metrics

All measurements are from repeatable runs of the included benchmark harness on an Apple M4 2025 MacBook Air (10‑core), 24 GB RAM. Boards are generated deterministically from midgame positions, and the engine runs iterative deepening with parallelized root search.

- Depth 10: 18.50M nodes/s on average, about 149.8 ms per position.
- Depth 13: 8.93M nodes/s on average, about 7.87 s per position.

Key optimizations reflected in the numbers:
- Principal Variation Search (PVS) with bitboard‑native move ordering (corners → edges, TT‑seeded) cut per‑position latency by roughly **90%** versus a naïve alpha–beta baseline.
- Sharded TT per thread reduced contention and **doubled** deep‑ply throughput in practice.
- Move generation was rewritten to eliminate STL overhead in hot paths; combined with TT sharding, d13 latency improved from **8.7 s → 7.8 s** in our setup.


## Repository Layout

```
.
├── include/
│   ├── othello/
│   │   ├── Engine.hpp, GameBoard.hpp, OthelloRules.hpp, Constants.hpp
│   │   └── evaluator/Evaluator.hpp
│   └── utils/BitboardUtils.hpp, ThreadPool.hpp, Visualize.hpp
├── src/
│   ├── Engine.cpp, GameBoard.cpp, OthelloRules.cpp
│   ├── MobilityEvaluator.cpp, PositionalEvaluator.cpp
│   ├── benchmark.cpp, main.cpp, Controller.cpp
│   └── utils/ThreadPool.cpp, Visualize.cpp
├── tests/
│   ├── test_OthelloRules.cpp, CMakeLists.txt
├── proto/engine.proto   # gRPC WIP
├── CMakeLists.txt
├── Dockerfile, compose.yaml, README.Docker.md
└── README.md
```


## gRPC Status (WIP)

- Contract: `proto/engine.proto` defines `EngineService.FindBestMove` and a compact `GameState` using bitboards.
- Status: The C++ gRPC server/client plumbing has not been implemented yet. This README will be updated once the service is wired into the engine with request time/depth controls and a stable response schema.
