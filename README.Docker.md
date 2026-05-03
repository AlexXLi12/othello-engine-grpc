# Docker Workflow

The Docker build is the canonical way to build, test, and run this project
without installing CMake, gperftools, or test dependencies on the host. Direct
host CMake configuration is intentionally disabled.

## Build and Run

With `just`:

```bash
just run
```

Without `just`:

```bash
docker compose up --build engine
```

The default engine command is:

```bash
othello_exec 15 2000
```

Override the depth or time limit by passing arguments to the image:

```bash
docker compose run --rm engine 10 1000
```

Or:

```bash
just run 10 1000
```

## gRPC Server

```bash
just server
```

The server listens on port `50051` by default. Override the bind address with
`OTHELLO_SERVER_ADDRESS`.

## Tests

```bash
docker compose --profile test up --build test
```

Or:

```bash
just test
```

The `test` image runs `ctest` during the image build and also uses `ctest` as
its default command.

## Editor Setup

Host editors use `compile_flags.txt` for clangd. Populate the editor-only
dependency headers from Docker after cloning or after dependency changes:

```bash
just lsp
```

This creates `.clangd-deps/` from the Docker test image. The directory is ignored
by git and is only for host-side clangd parsing.

## Benchmark

```bash
docker compose --profile benchmark up --build benchmark
```

Or:

```bash
just benchmark
```

Profiling is available in every runtime image but disabled by default. Enable it
with `OTHELLO_PROFILE=1`:

```bash
OTHELLO_PROFILE=1 docker compose --profile benchmark up --build benchmark
```

Or:

```bash
just profile
```

CPU profiles are written under `/engine/profiles` inside the container when
profiling is enabled. Compose persists that directory in the `profiles` volume.

## Direct Docker Targets

```bash
docker build --target runtime -t othello-engine:local .
docker build --target test -t othello-engine-test:local .
docker build --target benchmark -t othello-engine-benchmark:local .
```
