# Othello Engine (C++23)

A high-performance Othello/Reversi engine written in modern C++ with a focus on search performance, compact board representation, and clear systems-oriented design.

This repository currently contains a working single-node engine with parallel root search, a benchmark target, unit tests, and an early gRPC contract. The distributed/service side is still a work in progress.

## What is implemented today

### Engine core

- **Bitboard board representation** using `uint64_t` for black and white pieces
- **Negamax + alpha-beta pruning**
- **Principal Variation Search (PVS)-style search**
- **Move ordering** with corners first, then edges, plus transposition-table move promotion
- **Zobrist hashing** for board state keys
- **Transposition table** using `std::unordered_map<uint64_t, TTEntry>`
- **Parallel root search** using a custom thread pool and shared atomic alpha
- **Evaluation layer** with positional and mobility evaluators

### Tooling

- **Benchmark executable** for measuring search throughput
- **GoogleTest-based unit tests**
- **Dockerized build, test, runtime, and benchmark targets**
- **Proto definition** for an eventual gRPC engine service

## What is not finished yet

- The **gRPC server/client implementation** is not wired up yet
- The engine is still effectively a **single-node process**, not a distributed system
- There is no real **request queue, load balancer, worker orchestration, or service discovery** yet
- The current transposition-table design is practical, but not yet a custom cache-tuned or distributed TT

## Why this project exists

The goal of this project is not just to build a strong Othello engine.
It is also a vehicle for exploring systems problems that show up in real infrastructure work:

- latency vs throughput tradeoffs
- concurrency and coordination
- caching behavior
- performance profiling
- service boundaries
- eventually, distributed execution

Othello is a good domain for this because the engine core is compact and deterministic, which makes systems changes easier to reason about and benchmark.

## Repository layout

```text
.
├── include/
│   ├── othello/
│   │   ├── Constants.hpp
│   │   ├── Controller.hpp
│   │   ├── Engine.hpp
│   │   ├── GameBoard.hpp
│   │   ├── OthelloRules.hpp
│   │   └── evaluator/
│   └── utils/
├── proto/
│   └── engine.proto
├── src/
│   ├── Controller.cpp
│   ├── Engine.cpp
│   ├── GameBoard.cpp
│   ├── MobilityEvaluator.cpp
│   ├── OthelloRules.cpp
│   ├── PositionalEvaluator.cpp
│   ├── benchmark.cpp
│   ├── main.cpp
│   └── utils/
├── tests/
│   ├── CMakeLists.txt
│   └── test_OthelloRules.cpp
├── CMakeLists.txt
├── Dockerfile
├── README.Docker.md
└── compose.yaml
```

## Core design

### 1. Bitboards

The engine uses a pair of 64-bit bitboards to represent board state. This keeps move generation and flipping operations compact and efficient, and avoids per-cell board structures in hot paths.

Relevant files:
- `include/othello/GameBoard.hpp`
- `src/GameBoard.cpp`
- `include/othello/Constants.hpp`
- `src/OthelloRules.cpp`

### 2. Search

The main search algorithm is negamax with alpha-beta pruning. The implementation also uses a PVS-style approach where the first move is searched with a full window and later moves are first probed with a narrow scout window.

Relevant files:
- `include/othello/Engine.hpp`
- `src/Engine.cpp`

### 3. Move ordering

Legal moves are ordered with a simple but effective heuristic:
- corners first
- then edges
- then remaining moves
- if a move exists in the transposition table, it is promoted to the front

This helps pruning efficiency without overcomplicating the implementation.

### 4. Parallel root search

At the root, the first move is searched to seed alpha, then sibling moves are searched in parallel using a custom thread pool. The implementation uses a shared `std::atomic<int>` for alpha updates.

This is the main concurrency mechanism in the repo today.

Relevant files:
- `include/utils/ThreadPool.hpp`
- `src/utils/ThreadPool.cpp`
- `src/Engine.cpp`

### 5. Transposition table

The engine stores search results in a hash table keyed by Zobrist hashes. Entries include:
- score
- depth
- bound type
- best move index

The current implementation uses `std::unordered_map<uint64_t, TTEntry>`. At the root, the search uses one TT per root move to reduce contention during parallel search.

This is a good practical baseline, though there is still room to push it much further from a cache-layout and systems perspective.

## Build

This project uses **CMake** internally, targets **C++23**, and is intentionally
Docker-only. Do not configure or run it directly on the host.

Build and run the engine:

```bash
just run
```

Run the test image:

```bash
just test
```

Run the benchmark image:

```bash
just benchmark
```

Profiling support is built into the Docker images, but profile collection is off
unless `OTHELLO_PROFILE=1` is set.

Targets include:
- `othello_exec`
- `othello_benchmark`
- test targets under `tests/`

## Current caveats

A few things are worth calling out explicitly:

- The README used to describe some future-facing service/distributed ideas more strongly than the current code justified.
- The gRPC layer is currently just a **proto contract**, not a finished service.
- `main.cpp` still looks more like an interactive local executable than a service entrypoint.
- The current system is best described as a **fast local engine with some parallel search**, not yet a distributed engine platform.

## gRPC status

The repository already includes a protobuf contract:

- `proto/engine.proto`

It defines:
- `EngineService.FindBestMove`
- `FindBestMoveRequest`
- `FindBestMoveResponse`
- `GameState`

What is missing is the actual implementation layer:
- generated stubs wired into the build
- server implementation
- request translation into engine calls
- client or load generator
- timeout/depth control semantics at the service boundary

## Where this project can go next

This project becomes much more compelling if it evolves from “strong engine” into “strong engine turned into a systems artifact.”

### High-leverage next steps

1. **Finish the gRPC service**
   - implement server + request handling
   - expose depth/time controls cleanly
   - return structured search metadata

2. **Add a real benchmark/service harness**
   - request latency
   - throughput under concurrency
   - queueing behavior
   - tail latency under load

3. **Turn it into a multi-process/containerized system**
   - one frontend service
   - worker processes for search jobs
   - containerized deployment with Compose/Kubernetes later if needed

4. **Make the project observable**
   - request metrics
   - profiling hooks
   - cache hit rate
   - nodes searched
   - time spent in move generation vs search vs evaluation

5. **Upgrade the TT / search infrastructure**
   - more cache-conscious storage
   - capacity/replacement policy experiments
   - contention-aware shared-state design
   - possibly explore distributed result caching only after local performance is solid

6. **Document the engineering tradeoffs**
   - why this concurrency model
   - why this board representation
   - where the bottlenecks are
   - what changed after profiling

## Why those next steps matter

If the goal is eventually to work on systems-heavy teams — the kind of places doing serious database, infra, and query/data-engine work — then the strongest version of this repo is not just:

> “I built a game engine.”

It is:

> “I built a performant engine core, exposed it as a service, measured it under load, and used that to explore concurrency, caching, latency, and distributed-system tradeoffs.”

That story is much closer to real infrastructure work.

## Summary

Today, this repo is already a solid engine project with real technical substance:
- compact representation
- serious search implementation
- parallelism
- benchmarking mindset

The next leap is to make it a systems project, not just an engine project.
