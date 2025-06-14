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
#    openssh-client \
    libjack-jackd2-dev \
    ladspa-sdk \
#    libcurl4-openssl-dev \
#    libfreetype6-dev \
#    libx11-dev \
#    libxcomposite-dev \
#    libxcursor-dev \
#    libxext-dev \
#    libxinerama-dev \
#    libxrandr-dev \
#    libxrender-dev \
#    libwebkit2gtk-4.1-dev \
#    libglu1-mesa-dev \
#    mesa-common-dev \
    libasound2-dev \
    libgtk-3-dev

#RUN apt install -y software-properties-common lsb-release

# Purge old cmake
#RUN apt purge --auto-remove cmake gcc g++

# Add Kitware repository key
#RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
#
# Add Kitware repository
#RUN apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
#RUN  echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
#RUN apt-get install kitware-archive-keyring

# Install CMake 3.25.2
#RUN apt-get update && apt-get install -y cmake


# Stage 2: Builder image
FROM base AS builder

# Set the working directory
WORKDIR /app

# Copy the project source code
COPY .      .


# Configure the CMake project using the linux-container preset
RUN cmake --preset default 

# Build the project
RUN cmake --build build

# Run tests
RUN ctest --test-dir build --output-on-failure
