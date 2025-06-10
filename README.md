## Project Overview

Pointilistic Stochastic Textural Timbre Synthesis is LLM-speak for a granular synthesis engine that presents the user with controls to shape the geometry and composition of a point cloud of grains governed by stochastic probability with a beautiful and responsive interface that provides enough consistency and precision to build the kind of intuition required for confident live performance without hobbling the power of opening the geometry up wide and letting unbridled pseudorandomness flow.

This project aims to create a next-generation software instrument that empowers musicians, composers, and sound designers to intuitively sculpt complex, evolving sonic textures using principles of pointillism and stochastic generation. It's not just another subtractive synth; it's a tool for generating organic, non-linear soundscapes.

**Key Features:**

*   **Granular Synthesis Engine:** Generates sound from internal oscillators (Sine, Saw, Square, Noise) or user-provided audio samples (WAV, AIFF). Capable of producing a high number of simultaneous grains (e.g., 256+).
*   **Stochastic Parameter Control:** Users shape probability distributions for grain properties (pitch, pan, duration, amplitude) rather than fixed values. Controls include Density, Pitch & Dispersion, Duration & Variation, Stereo Position & Spread, and Temporal Distribution models.
*   **Interactive UI & Visualization:** A visually compelling interface with real-time 2D visualization of sound grains (X-axis: pan, Y-axis: pitch). Main parameters are controlled via large "Pod" controls.
*   **Preset Management:** Save and load instrument states as presets, browse factory presets, and categorize them.

## For Users

### Installation

This instrument is available in the following plugin formats:

*   VST3
*   Audio Unit (AU)
*   AAX

Install the plugin in the appropriate folder for your Digital Audio Workstation (DAW).

### Quick Start - Understanding the Controls

The core idea is to control the *chance* of how grains of sound will behave.

*   **Grain Source:** Choose between internal waveforms or load your own audio sample.
*   **Density:** Controls how many grains are generated per second. Higher density means a thicker texture.
*   **Pitch & Dispersion:** Sets a central pitch for the grains. "Dispersion" controls how far from this central pitch individual grains can stray. More dispersion means more atonal or dissonant textures.
*   **Duration & Variation:** Defines the average length of each grain. "Variation" introduces randomness to this length.
*   **Position & Spread:** Sets the average stereo position (left to right) of the grains. "Spread" determines how wide the grains are scattered in the stereo field.
*   **Temporal Distribution:** Changes the rhythmic feel of grain generation (e.g., Uniform for steady, Poisson for more random).
*   **Grain Envelope:** Shapes the attack and decay of each individual grain (e.g., Trapezoid, Hann).

Experiment by adjusting these controls and observing the changes in the real-time visualization and the sound.

### Preset Management

*   **Saving Presets:** You can save the entire state of the instrument as a preset.
*   **Loading Presets:** A user-friendly browser allows you to load presets.
*   **Factory Presets:** Explore the included factory presets to get an idea of the instrument's capabilities. Presets can be tagged and categorized (e.g., "Texture," "Rhythmic," "Melodic," "FX").

## For Developers & Contributors

This project is built with C++ and the JUCE framework.

### Building from Source

1.  Clone the repository.
2.  Ensure you have a C++ compiler and CMake installed.
3.  Ensure you have the JUCE framework installed and correctly set up in your development environment. The project uses JUCE to handle plugin formats, audio/MIDI I/O, and basic windowing.
4.  Use CMake to generate project files for your IDE (e.g., Xcode, Visual Studio) or build system.
    ```bash
    # Example CMake configuration and build
    cmake -B build -S .
    cmake --build build
    ```
5.  The build process will produce the VST3, AU, and AAX plugin targets.

### Codebase Structure

*   `Source/`: Contains the core C++ code for the synthesis engine and UI.
    *   `Data/`: Manages presets and other data.
    *   `UI/`: JUCE components for the user interface, including the visualization.
*   `plugin/`: Platform-specific plugin wrapper code and shared plugin logic (Processor, Editor).
    *   `include/Pointilsynth/`: Public headers for the core synth parts.
    *   `source/`: Implementation files for the plugin logic.
*   `cmake/`: CMake scripts for building the project.
*   `jules_docs/`: Project documentation, including requirements and design plans.
*   `test/`: Unit tests for the project.

### Contribution Guidelines

*   Please follow the coding style defined in `.clang-format`.
*   Consider using the pre-commit hooks defined in `.pre-commit-config.yaml` to ensure code quality before committing.
    ```bash
    # Install pre-commit (if you haven't already)
    pip install pre-commit
    # Set up the git hook scripts
    pre-commit install
    ```
*   Write unit tests for new features or bug fixes.

### Running Tests

The project includes a suite of unit tests. After building the project, you can typically find and run the test executable from your build directory.

```bash
# Example: navigate to build directory and run tests
cd build/test/
./PointilSynth_Test # Or PointilSynth_Test.exe on Windows
```

## For the Curious & Sound Designers

### The Synthesis Technique: Pointillistic Stochastic Granular Synthesis

This instrument uses **granular synthesis**, where sound is constructed from tiny segments of audio called "grains." Instead of playing a continuous sound, it plays many small grains, often overlapping, to create complex textures.

The "pointillistic" aspect refers to how these grains are generated and distributed, much like dots of paint in a pointillist painting. Each grain can have unique properties.

The "stochastic" part means that these properties (like pitch, duration, pan, start time) are not fixed but are determined by probability distributions. You, the user, control the *shape* of these probabilities. For example, you don't set an exact pitch for all grains; you set a *central* pitch and a *dispersion* range, and the actual pitch of each grain is randomly chosen within that defined probabilistic range. This allows for the creation of sounds that feel organic, evolving, and unpredictable, yet controllable.

### Visualization

The central 2D visualization is key to understanding what's happening:

*   **X-axis (Horizontal):** Represents the stereo position (panning) of a grain. Grains to the left are panned left, grains to the right are panned right.
*   **Y-axis (Vertical):** Represents the pitch of a grain. Higher grains have higher pitches.
*   **Appearance (Color, Size - *Planned*):** The visual properties of a grain (e.g., its color or size) will eventually reflect other properties like its audio source or duration, giving you an immediate visual summary of the texture's composition.

By watching the visualization, you can intuitively grasp how the parameters you adjust are affecting the cloud of sound grains.

### Creative Uses

This synth excels at:

*   **Atmospheric Pads:** Create dense, evolving soundscapes with subtle internal movement.
*   **Complex Textures:** Generate rich, detailed sonic fabrics from any source audio.
*   **Rhythmic Pulses:** Use the temporal distribution controls to create intricate, non-traditional rhythmic patterns.
*   **Sound Effects:** Design unique and unusual sound effects by manipulating grain properties.
*   **Experimental Sound Design:** Explore the boundaries of granular synthesis and stochastic processes.

## Current Status & Roadmap

This project is under active development. Key development phases (based on `jules_docs/sprint_master_plan.md`):

*   **Core Engine & Stochastic Controls (Complete):** The fundamental sound generation and parameter logic are in place.
*   **UI, Visualization & Preset Management (In Progress/Nearing Completion):** The main user interface, real-time visualization, and preset system are being finalized.
*   **Modulation System (V2 - Future):** Planned features include LFOs and ADSR envelopes for animating parameters over time.

See `jules_docs/sprint_master_plan.md` for more details on past and future development sprints.

## License

This project is licensed under the terms of the LICENSE.md file.
