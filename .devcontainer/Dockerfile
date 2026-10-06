FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

# Avoid interactive prompts during apt installation
ENV DEBIAN_FRONTEND=noninteractive

# Install C++ toolchain, CMake, Make, Ninja, Protobuf, and ZeroMQ
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gdb \
    cmake \
    make \
    ninja-build \
    pkg-config \
    libprotobuf-dev \
    protobuf-compiler \
    libzmq3-dev \
    cppzmq-dev \
    git \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
