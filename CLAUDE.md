# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PointilSynth is a JUCE-based audio plugin implementing pointillistic stochastic granular synthesis. It generates complex textures from internal oscillators or user samples using probability distributions to control grain properties rather than fixed values.

## Build System & Commands

### Building the Project
```bash
# Configure with default preset (uses Ninja)
cmake --preset default

# Build with default preset
cmake --build --preset default

# Alternative: manual configuration
cmake -B build -S .
cmake --build build
```

### Running Tests
```bash
# Run all tests after building
ctest --preset default

# Note: Do NOT use -C flag with single-config generators like Ninja
```

### Plugin Formats
The build produces VST3, AU, and AAX formats. Currently VST3 is commented out in CMake for development.

## Core Architecture

### Main Components

**Audio Processing Chain:**
- `AudioPluginAudioProcessor` (plugin/include/Pointilsynth/PluginProcessor.h) - Main JUCE processor
- `AudioEngine` (plugin/source/AudioEngine.cpp) - Core synthesis engine managing grain lifecycle
- `StochasticModel` - Generates grain parameters using probability distributions
- `ConfigManager` - Centralized parameter management using JUCE APVTS

**UI Architecture:**
- `PluginEditor` - Main editor window
- `VisualizationComponent` - Real-time 2D grain visualization (X=pan, Y=pitch)
- `ProbabilityWaveComponent` - Visual representation of parameter distributions
- `PresetBrowserComponent` - Preset management interface

**Parameter System:**
The `ConfigManager` class acts as a singleton wrapper around JUCE's `AudioProcessorValueTreeState`. All parameter access goes through this centralized system:
- Parameter IDs defined in `ConfigManager::ParamID`
- Thread-safe parameter updates via callbacks
- Automatic UI attachment management

### Key Design Patterns

**Grain Management:**
- Grains are short audio segments with stochastic properties (pitch, pan, duration, etc.)
- `AudioEngine` maintains a vector of active grains, processing them each audio block
- Grain parameters determined by probability distributions rather than fixed values

**Visualization:**
- Real-time 2D visualization shows grain positions (pan vs pitch)
- `InertialHistoryManager` provides smooth visual transitions
- Grain visual properties can reflect audio characteristics

**Configuration:**
- All parameters managed through `ConfigManager` singleton
- UI components create attachments via `createAttachedSlider()`/`createAttachedComboBox()`
- Non-UI classes register callbacks via `addListener()` for parameter changes

## Development Workflow

### Code Organization
- **plugin/include/Pointilsynth/**: Core headers (audio engine, processor, managers)
- **plugin/include/UI/**: UI component headers
- **plugin/source/**: All implementation files
- **test/**: Unit tests using Catch2 framework

### Dependencies
- **JUCE 8.0.6**: Audio framework (via CPM)
- **nlohmann_json**: JSON handling for presets
- **Catch2**: Testing framework
- **GoogleTest**: Additional testing utilities

### Testing
- Comprehensive test suite covering all major components
- Tests include audio processing, UI components, and parameter management
- Use `melatonin_test_helpers` for JUCE-specific testing utilities

### Key Files for Understanding
- `plugin/include/Pointilsynth/PointilismInterfaces.h`: Core interfaces and data structures
- `plugin/source/StochasticModel.cpp`: Probability distribution logic
- `plugin/include/Pointilsynth/ConfigManager.h`: Parameter management system