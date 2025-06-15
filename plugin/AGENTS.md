# Agent Instructions for the /plugin/ Directory

This directory contains the core source code of the PointilSynth audio plugin.

## 1. Key Classes

- **`PluginProcessor.h`/`.cpp`**: The main entry point for the DAW. It owns the `AudioEngine`.
- **`PluginEditor.h`/`.cpp`**: The main UI window for the plugin. It gets data from the `PluginProcessor`.
- **`AudioEngine.h`/`.cpp`**: The heart of the synthesizer. It manages all active sound "grains" and contains the main `processBlock` where all audio is generated. If you are changing the sound output, you will edit this file.
- **`StochasticModel.h`/`.cpp`**: The brain of the synthesizer. It manages the probability distributions that control grain properties (pitch, pan, etc.). If you are changing how parameters are generated, you will edit this file.
- **`PodComponent.h`**: A placeholder for the main UI control pods. You may need to implement its functionality.

## 2. Adding New Source Files

If you add a new `.cpp` or `.h` file:
1.  Place `.h` files in `plugin/include/Pointilsynth/`.
2.  Place `.cpp` files in `plugin/source/`.
3.  **You MUST update `plugin/CMakeLists.txt`** to include the new file in the `target_sources()` command. Add `.cpp` files to the `SOURCE_FILES` list and `.h` files to the `HEADER_FILES` list. Failure to do so will cause a build failure.
