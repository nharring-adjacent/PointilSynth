Core Framework & Language
 * Technology: C++ (C++17 or newer)
   * Contribution: This is the industry-standard language for high-performance audio processing. It provides the low-level control necessary to meet the strict CPU efficiency and real-time performance goals required by the core sound engine.
 * Technology: JUCE Framework
   * Contribution: JUCE is a comprehensive C++ framework that will serve as the foundation of the application. It provides the essential boilerplate for creating stable VST3, AU, and AAX plugins, handling audio/MIDI I/O, and creating a basic application window, which dramatically reduces development time and mitigates stability risks across different DAWs.
DSP & Sound Engine
 * Technology: Custom Granular Synthesis Engine
   * Contribution: A custom-built C++ engine is necessary to achieve the specific "pointillistic" behavior. This allows for tight integration between the stochastic parameter controls (Density, Pitch Dispersion, etc.) and the per-grain property assignment (pitch, pan, duration, amplitude), ensuring maximum performance.
 * Algorithm: High-Quality Windowed-Sinc Resampling
   * Contribution: To ensure professional audio quality when pitching grains from the source audio file, a high-quality resampling algorithm is critical. A windowed-sinc (or polyphase FIR) implementation provides excellent results, minimizing aliasing and other artifacts as required.
 * Library: C++ Standard <random> Library
   * Contribution: This standard library will be used to implement the stochastic engine. It provides robust and efficient tools for generating numbers according to various probability distributions, which is essential for the Pitch Dispersion, Duration Variation, and particularly the selectable temporal models like Poisson distribution.
Preset Management & Utilities
 * Library: nlohmann/json
   * Contribution: To meet the preset management requirements, the entire state of the instrument must be saved and loaded. Using JSON as the format makes presets human-readable and easy to debug. The nlohmann/json library is a header-only, dependency-free, and extremely easy-to-use C++ library for serializing and deserializing the instrument's state, including preset tags for the browser.
 * Library: JUCE Audio Format Readers
   * Contribution: The engine must load user-provided audio samples in WAV and AIFF format. The JUCE framework has built-in, stable, and efficient classes for reading these formats, eliminating the need for an external dependency.

