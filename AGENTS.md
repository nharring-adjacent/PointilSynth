# Agent Instructions for the PointilSynth Repository

Hello, Agent. This file contains essential information for working in this repository. Please read it carefully before making changes.

## 1. Codebase Structure

This is a C++ project using the JUCE framework to build an audio plugin. The structure is as follows:

- **/plugin/**: **This is the main directory.** All C++ source code for the synthesizer plugin lives here. This is where you will spend most of your time.
  - `plugin/include/Pointilsynth/`: Contains the public C++ header files.
  - `plugin/source/`: Contains the C++ implementation (.cpp) files.
- **/test/**: Contains all C++ unit tests for the code in the `plugin/` directory.
- **/cmake/**: Contains helper scripts for the CMake build system. You will likely not need to edit these.
- **/jules_docs/**: Contains product requirements and planning documents. Refer to these for context on features.
- **CMakeLists.txt**: The root file that defines the entire build process.

**Key Architectural Note**: The core audio logic is in `plugin/source/AudioEngine.cpp`. The logic for generating sound parameters is in `plugin/source/StochasticModel.cpp`. The main UI code is in `plugin/source/PluginEditor.cpp`.

## 2. How to Build the Project

This project uses CMake and is best built using CMake Presets, which define all the necessary settings. Do not invoke cmake manually without presets.

1.  **Configure CMake**:
    ```bash
    cmake --preset default
    ```
2.  **Build the project**:
    ```bash
    cmake --build --preset default
    ```

## 3. How to Run Tests

This project uses GoogleTest for unit testing. Tests are discovered and run via `ctest`.

1.  **Ensure the project is built** by following the build steps above.
2.  **Run the tests**:
    ```bash
    ctest --preset default
    ```
You MUST verify that all tests pass after your changes. If you add new functionality, you MUST add a corresponding test.
