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
    docker compose --profile benchmark run --rm --no-deps benchmark --format text

benchmark-build:
    docker compose --profile benchmark build benchmark

benchmark-json:
    docker compose --profile benchmark run --rm --no-deps benchmark --format json

benchmark-csv:
    docker compose --profile benchmark run --rm --no-deps benchmark --format csv

profile-run depth="15" plies="20" threads="5" positions="1" profile_file="cpu_profile.prof":
    mkdir -p profiles
    OTHELLO_PROFILE=1 docker compose --profile benchmark run --rm --no-deps benchmark --depth {{depth}} --positions {{positions}} --plies {{plies}} --threads {{threads}} --profile-file {{profile_file}} --format text
    @echo "CPU profile: profiles/{{profile_file}}"

profile:
    just profile-run

profile-svg profile_file="" svg_file="cpu_profile.svg":
    docker compose --profile benchmark run --rm --no-deps --entrypoint /bin/bash benchmark -lc 'set -euo pipefail; profile_file="{{profile_file}}"; if [[ -n "$profile_file" ]]; then profile_path="/engine/profiles/$profile_file"; else profile_path="$(find /engine/profiles -maxdepth 1 -type f -name "*.prof" -printf "%T@ %p\n" | sort -nr | head -n 1 | cut -d" " -f2- || true)"; fi; if [[ -z "$profile_path" || ! -f "$profile_path" ]]; then echo "No profile found. Run just profile-run first, or pass a profile file." >&2; exit 1; fi; google-pprof --svg /usr/local/bin/othello_benchmark "$profile_path" > /engine/profiles/{{svg_file}}; ls -lh /engine/profiles/{{svg_file}}'
    @echo "Flamegraph SVG: profiles/{{svg_file}}"

profile-web port="8081" profile_file="":
    @echo "pprof UI: http://localhost:{{port}}"
    docker compose --profile benchmark run --rm --no-deps --publish "{{port}}:{{port}}" --entrypoint /bin/bash benchmark -lc 'set -euo pipefail; profile_file="{{profile_file}}"; if [[ -n "$profile_file" ]]; then profile_path="/engine/profiles/$profile_file"; else profile_path="$(find /engine/profiles -maxdepth 1 -type f -name "*.prof" -printf "%T@ %p\n" | sort -nr | head -n 1 | cut -d" " -f2- || true)"; fi; if [[ -z "$profile_path" || ! -f "$profile_path" ]]; then echo "No profile found. Run just profile-run first, or pass a profile file." >&2; exit 1; fi; exec /usr/local/bin/pprof -http=0.0.0.0:{{port}} /usr/local/bin/othello_benchmark "$profile_path"'

shell:
    docker compose run --rm --build --entrypoint /bin/bash engine

clean:
    docker compose down --volumes --remove-orphans
