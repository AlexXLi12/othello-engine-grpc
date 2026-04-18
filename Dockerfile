# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=22.04

FROM ubuntu:${UBUNTU_VERSION} AS base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

FROM base AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    libgmock-dev \
    libgoogle-perftools-dev \
    libgtest-dev \
    pkg-config \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests
COPY proto ./proto

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
    -DOTHELLO_DOCKER_BUILD=ON \
 && cmake --build build --parallel

FROM build AS test

RUN ctest --test-dir build --output-on-failure

CMD ["ctest", "--test-dir", "/workspace/build", "--output-on-failure"]

FROM base AS runtime

WORKDIR /engine

RUN apt-get update && apt-get install -y --no-install-recommends \
    google-perftools \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --system --create-home --home-dir /engine othello \
 && mkdir -p /engine/profiles \
 && chown -R othello:othello /engine

COPY --from=build /workspace/build/othello_exec /usr/local/bin/othello_exec
COPY --from=build /workspace/build/othello_benchmark /usr/local/bin/othello_benchmark

ENV OTHELLO_PROFILE_DIR=/engine/profiles

USER othello

VOLUME ["/engine/profiles"]

ENTRYPOINT ["othello_exec"]
CMD ["15", "2000"]

FROM runtime AS benchmark

ENTRYPOINT ["othello_benchmark"]
CMD []
