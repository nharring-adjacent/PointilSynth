Pointilistic Stochastic Textural Timbre Synthesis is LLM-speak for a granular synthesis engine that presents the user with controls to shape the geometry and composition of a point cloud of grains governed by stochastic probability with a beautiful and responsive interface that provides enough consistency and precision to build the kind of intuition required for confident live performance without hobbling the power of opening the geometry up wide and letting unbridled pseudorandomness flow.

## Continuous Integration (CI)

This project uses GitHub Actions for Continuous Integration. The CI pipeline is defined in `.github/workflows/cmake.yml`.

Key aspects of the CI setup:
- **Triggers**: The CI workflow runs on every `push` and `pull_request` to the `main` branch.
- **Platforms**: Builds are performed on `macos-latest` and `windows-latest`.
- **Build System**: CMake is used to configure and build the project.
- **Compilation**: The plugin and associated tests are compiled in `Debug` configuration.
- **Testing**: After a successful build, tests are executed using CTest. The test results determine the success or failure of the CI run. Test failures will be reported in the GitHub Actions logs, including output from failed tests.

## Building and Running Tests Locally

The project uses CMake for building and JUCE's unit testing framework (with GoogleTest utilities) for tests.

### Prerequisites
- A C++ compiler supporting C++20.
- CMake (version 3.22 or higher).
- Git.

### Steps to Build and Test

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd <repository-directory>
    ```

2.  **Configure CMake:**
    Create a build directory and run CMake configuration. This will download dependencies like JUCE and GoogleTest if they are not already present in the `libs` directory.
    ```bash
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    ```
    (You can replace `Debug` with `Release` for a release build).

3.  **Build:**
    Compile the project (including the test executable).
    ```bash
    cmake --build build --config Debug
    ```
    (Adjust `--config` if you used a different build type).

4.  **Run Tests:**
    The tests are run via CTest, which executes the `PointilSynthTests` test program.
    Navigate to the build directory and run CTest:
    ```bash
    cd build
    ctest -C Debug --output-on-failure
    ```
    (Adjust `-C Debug` if you used a different build type).
    A successful test run will show all tests passing. If tests fail, CTest will report a non-zero exit code, and details of the failures will be printed to the console (due to `--output-on-failure`).

    Alternatively, you can run the test executable directly after building:
    On macOS/Linux:
    ```bash
    ./test/PointilSynthTests
    ```
    On Windows:
    ```bash
    .\test\Debug\PointilSynthTests.exe
    ```
    (The exact path might vary based on your generator and build type configuration. The `ctest` command is generally more portable.)
