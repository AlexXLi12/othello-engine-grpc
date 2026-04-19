set dotenv-load := true

default:
    just --list

config:
    docker compose --profile test --profile benchmark config

build:
    docker compose build engine

lsp:
    mkdir -p .clangd-deps/usr/include
    docker compose --profile test run --rm --build --entrypoint /bin/bash -v "{{justfile_directory()}}/.clangd-deps:/clangd-deps" test -lc 'set -euo pipefail; mkdir -p /clangd-deps/usr/include; cp -a /usr/include/gperftools /clangd-deps/usr/include/; cp -a /usr/include/gtest /clangd-deps/usr/include/; cp -a /usr/include/gmock /clangd-deps/usr/include/'

run depth="15" time_ms="2000":
    docker compose run --rm --build engine {{depth}} {{time_ms}}

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
