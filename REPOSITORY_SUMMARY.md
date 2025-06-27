# PointilSynth Repository Summary

This document provides a comprehensive overview of the PointilSynth project to help agents quickly understand the repository structure, technologies, and code organization without needing to re-analyze the entire codebase.

## 1. Project Overview

PointilSynth is a granular synthesis audio plugin built using the JUCE framework. It implements a stochastic "pointillistic" approach to sound generation, where users control probability distributions rather than fixed values to create evolving, organic soundscapes.

### Key Features
- Granular synthesis engine with stochastic parameter control
- Support for both internal waveforms and user-loaded audio samples
- Real-time visualization of generated sound grains
- Preset management system

## 2. Tech Stack

### Core Technologies
- **Language**: C++20 (cross-platform support)
- **Framework**: JUCE 8.0.6 (audio plugin framework)
- **Build System**: CMake 3.22+ with CMake Presets
- **Package Manager**: CPM (embedded in the project)
- **Testing Framework**: GoogleTest 1.16.0
- **Serialization**: nlohmann/json 3.11.3
- **CI/CD**: GitHub Actions

### Development Tools
- **Linting**: clang-format (version 19)
- **Static Analysis**: CodeQL
- **Pre-commit Hooks**: Configured via .pre-commit-config.yaml

## 3. CI/CD Pipeline

The project uses GitHub Actions with multiple workflows:

1. **auto-clang-format.yml**: Automatically formats all C++ code using clang-format on pull requests.
2. **cmake.yml**: Builds the project and runs tests for every push and PR to the trunk branch.
3. **codeql.yml**: Runs CodeQL analysis for code quality and security.
4. **post-merge-builds.yml**: Runs platform-specific builds (macOS, Windows) after merges to main branch.
5. **publish-docker-image.yml**: Creates Docker images for CI environment.

The CI process uses a custom Docker container (`pointilsynth-build-env`) for consistent build environments.

## 4. Code Structure

### Key