set dotenv-load := true

default:
    just --list

config:
    docker compose --profile test --profile benchmark config

build:
    docker compose build engine

lsp:
    mkdir -p .clangd-deps/usr/include .clangd-deps/generated
    docker compose --profile test run --rm --build --entrypoint /bin/bash -v "{{justfile_directory()}}/.clangd-deps:/clangd-deps" test -lc 'set -euo pipefail; cp -a /usr/include/{gperftools,gtest,gmock,grpc,grpc++,grpcpp} /clangd-deps/usr/include/; mkdir -p /clangd-deps/usr/include/google; cp -a /usr/include/google/protobuf /clangd-deps/usr/include/google/; cp -a /workspace/build/generated/*.pb.h /clangd-deps/generated/'

run depth="15" time_ms="2000":
    docker compose run --rm --build engine {{depth}} {{time_ms}}

server:
    #!/usr/bin/env bash
    set -euo pipefail
    node web/server.mjs &
    web_pid=$!
    trap 'kill "$web_pid" 2>/dev/null || true' EXIT INT TERM
    echo "Othello web UI: http://localhost:${OTHELLO_WEB_PORT:-8080}"
    COMPOSE_MENU=false docker compose up --build server

frontend:
    node web/server.mjs

test:
    docker compose --profile test up --build --abort-on-container-exit --exit-code-from test test

benchmark:
    docker compose --profile benchmark up --build --abort-on-container-exit --exit-code-from benchmark benchmark

profile:
    mkdir -p profiles
    OTHELLO_PROFILE=1 docker compose --profile benchmark up --build --abort-on-container-exit --exit-code-from benchmark benchmark
    @echo "CPU profile: profiles/cpu_5_thread.prof"

profile-svg: profile
    docker compose --profile benchmark run --rm --no-deps --entrypoint /bin/bash benchmark -lc 'set -euo pipefail; google-pprof --svg /usr/local/bin/othello_benchmark /engine/profiles/cpu_5_thread.prof > /engine/profiles/cpu_5_thread.svg; ls -lh /engine/profiles/cpu_5_thread.svg'
    @echo "Flamegraph SVG: profiles/cpu_5_thread.svg"

profile-web port="8081": profile
    @echo "pprof UI: http://localhost:{{port}}"
    docker compose --profile benchmark run --rm --no-deps --service-ports --publish "{{port}}:{{port}}" --entrypoint /usr/local/bin/pprof benchmark -http=0.0.0.0:{{port}} /usr/local/bin/othello_benchmark /engine/profiles/cpu_5_thread.prof

shell:
    docker compose run --rm --build --entrypoint /bin/bash engine

clean:
    docker compose down --volumes --remove-orphans
