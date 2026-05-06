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
    OTHELLO_PROFILE=1 docker compose --profile benchmark up --build --abort-on-container-exit --exit-code-from benchmark benchmark

shell:
    docker compose run --rm --build --entrypoint /bin/bash engine

clean:
    docker compose down --volumes --remove-orphans
