# ============ Build stage ============
FROM ubuntu:22.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

# Toolchain + gRPC/Protobuf dev pkgs
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake pkg-config git curl ca-certificates \
    protobuf-compiler protobuf-compiler-grpc libprotobuf-dev libprotoc-dev \
    libgrpc++-dev grpc-proto \
    google-perftools libgoogle-perftools-dev \
 && rm -rf /var/lib/apt/lists/*

# Copy your source (adjust path as needed)
WORKDIR /engine
# If your repo has a top-level CMakeLists.txt, this is enough:
COPY . .

# Configure + build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
 && cmake --build build -j \
 && cp build/othello_exec /engine/ \
 && cp build/othello_benchmark /engine/

# ============ Runtime stage ============
FROM ubuntu:22.04 AS runtime

# Minimal runtime libs (protobuf/grpc runtime shared objects)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libprotobuf23 libprotoc23 libabsl20210324 \
    libgrpc++1 libgrpc10 ca-certificates \
    google-perftools \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /engine
COPY --from=build /engine/othello_exec /engine/
COPY --from=build /engine/othello_benchmark /engine/

# EXPOSE 50051
ENTRYPOINT ["/engine/othello_exec"]

# =========== Benchmark stage ============
FROM runtime AS benchmark

# Run othello_benchmark
ENTRYPOINT ["/engine/othello_benchmark"]
