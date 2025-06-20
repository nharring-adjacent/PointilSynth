FROM docker.io/eyalamirmusic/juce_dev_machine:latest AS base

# Install dependencies
# Combine all apt-get install commands into a single RUN instruction to reduce layer count
# Add libasound2-dev and libgtk-3-dev from the GitHub workflow
# Remove duplicate libxcursor-dev entry
RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    ca-certificates \
    gpg \
    wget \
    git \
    clang \
    llvm \
    ninja-build \
    pkg-config \
    openssh-client \
    libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev \
    libfreetype6-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxext-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxrender-dev \
    libwebkit2gtk-4.1-dev \
    libglu1-mesa-dev \
    mesa-common-dev \
    libasound2-dev \
    libgtk-3-dev \
    x11-xserver-utils \
    libx11-dev \
    xserver-xorg-dev \
    xorg-dev

# Stage 2: Builder image
FROM base AS builder

# Set the working directory
WORKDIR /app

# Copy the project source code
COPY .  .

# Configure the CMake project using the linux-container preset
RUN cmake --preset default 

# Build the project
RUN cmake --build build

FROM builder AS tester
WORKDIR /app

COPY --from=builder . .

# Run tests
RUN ctest --preset default --output-on-failure
