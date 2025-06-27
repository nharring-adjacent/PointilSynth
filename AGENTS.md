# Agent Instructions for the PointilSynth Repository

Hello, Agent. This file contains essential information for working in this repository. Please read it carefully before making changes.

## 1. Codebase Structure

This is a C++ project using the JUCE framework to build an audio plugin. The structure is as follows:

- **/plugin/**: **This is the main directory.** All C++ source code for the synthesizer plugin lives here.
  - `plugin/include/Pointilsynth/`: Contains the public C++ header files.
  - `plugin/source/`: Contains the C++ implementation (.cpp) files.
- **/test/**: Contains all C++ unit tests for the code in the `plugin/` directory.
- **/cmake/**: Contains helper scripts for the CMake build system. You will likely not need to edit these.
- **/jules_docs/**: Contains product requirements and planning documents. Refer to these for context on features.
- **CMakeLists.txt**: The root file that defines the entire build process.

**Key Architectural Note**: The core audio logic is in [plugin/source/AudioEngine.cpp](cci:7://file:///Users/nicholasharring/PointilSynth/plugin/source/AudioEngine.cpp:0:0-0:0). The logic for generating sound parameters is in `plugin/source/StochasticModel.cpp`. The main UI code is in `plugin/source/PluginEditor.cpp`.

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

## 4. Real-time Audio Development Best Practices

1. **No Allocations in Audio Thread**: Never allocate memory in audio processing methods. All buffers should be pre-allocated in `prepareToPlay()`.

2. **Thread Safety**: Always ensure thread safety when communicating between the audio thread and other threads (like UI).

3. **Sample Rate Awareness**: All time-based calculations must account for the current sample rate. Store temporal values in samples, not milliseconds.

4. **DSP Component Design**: Keep DSP components focused on single responsibilities with clear interfaces for testing and reuse.

5. **Audio Parameter Handling**: All audio parameters should support thread-safe reads/writes and smooth value transitions.

## 5. JUCE-Specific Guidelines

1. **Module Imports**: Only import the specific JUCE modules you need, not the entire framework.

2. **Component Ownership**: Follow JUCE's ownership model where parent components own and destroy their children.

3. **Custom Component Structure**: For UI components:
   - Override only necessary paint/resize methods
   - Use JUCE's LookAndFeel system for styling
   - Support both light and dark themes

4. **Visualization Data Flow**: Use lock-free FIFOs for passing data from audio thread to UI thread.

## 6. Testing Requirements

1. **Unit Test Coverage**: Each DSP algorithm must have tests verifying:
   - Correctness of processing logic
   - Performance benchmarks
   - Boundary conditions
   - Numerical stability

2. **Reference Output Testing**: Use golden file comparisons for complex audio algorithms.

## 7. Key Patterns to Follow

1. **Stochastic Parameter Control**: When implementing stochastic features, encapsulate probability distributions with clear interfaces rather than using scattered random number generation.

2. **Parameter Encapsulation**: All plugin parameters must have appropriate range, default, and step values with proper metadata.

3. **Preset State Management**: Use structured JSON serialization for all preset data with backwards/forwards compatibility.

4. **Performance Critical Code**: Use JUCE's DSP module and SIMD operations for performance-critical vector operations.

Please consult the `.windsurfrules` file in the repository root for a comprehensive set of development rules and best practices specific to this project.