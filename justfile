set dotenv-load := true

default:
    just --list

config:
    docker compose --profile test --profile benchmark config

build:
    docker compose build engine

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
