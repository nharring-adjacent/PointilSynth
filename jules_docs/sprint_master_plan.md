Pointilistic Synth Sprint Plan
### **Sprint 0: Project Foundation & Scaffolding (1 Week)**

This preliminary sprint is executed by senior staff to create a stable and scalable foundation, enabling parallel work for the entire team from Sprint 1 onwards.

* **Primary Goal:** Establish the core project structure, build system, and continuous integration pipeline.
* **Key Activities:**
    * [cite_start]**Project Setup:** Initialize the C++ project using the JUCE framework, configuring it to build VST3, AU, and AAX plugin targets[cite: 41, 48].
    * [cite_start]**Dependency Integration:** Integrate and validate core third-party libraries: Skia for graphics [cite: 60] [cite_start]and nlohmann/json for presets[cite: 68].
    * **Architectural Skeleton:** Define the primary interfaces and data structures. [cite_start]This includes creating placeholder classes for `AudioEngine`, `StochasticModel`, and the `Grain` structure[cite: 13]. This is the most critical step for enabling parallel work.
    * **CI/CD Pipeline:** Configure a continuous integration server to automatically build and run tests on every code submission.
    * **Test Framework:** Set up the unit testing environment.

***

### **EPIC 1 & 2: Core Sound Engine & Stochastic Controls**

These sprints focus on creating a "headless" version of the instrument. All work can be validated through audio output and unit tests, without requiring a functional UI.

#### 📢 **Dependency Deadline: Start of Sprint 1**
* [cite_start]**Sound Design Team:** Provide a small collection of diverse WAV and AIFF files for testing the sample loader[cite: 11].

#### **Sprint 1: Core Grain Generation (2 Weeks)**

* **Primary Goal:** To generate silent "grains" from both internal and external audio sources.
* **Parallel Workstreams:**
    * [cite_start]**Stream A: Sample Loading:** Implement the mechanism to load user-owned WAV and AIFF samples using the JUCE Audio Format Readers[cite: 11, 72]. The output is a class that provides audio data to the rest of the engine.
    * [cite_start]**Stream B: Oscillator Source:** Implement the internal waveform generator, producing basic sine, saw, square, and noise signals that can be used as a grain source[cite: 12].
    * [cite_start]**Stream C: Grain Scheduler:** Develop the high-performance scheduler capable of managing a target of 256 simultaneous grains[cite: 13]. At this stage, it will just instantiate and track grain objects with default properties.

#### **Sprint 2: Stochastic Parameter Logic (2 Weeks)**

* **Primary Goal:** Implement the mathematical models that will control grain properties.
* **Parallel Workstreams:**
    * [cite_start]**Stream A: Pitch & Pan:** Using the C++ `<random>` library, develop the logic to control the central **Pitch** with a statistical **Dispersion** [cite: 18, 57] [cite_start]and the stereo **Position** with a **Spread** amount[cite: 20].
    * [cite_start]**Stream B: Duration & Rhythm:** Implement the controls for average grain **Duration** with **Variation**[cite: 19]. [cite_start]Implement the selectable **Temporal Distribution** models (e.g., Uniform, Poisson) to control the **Density** of grain creation[cite: 17, 21].
    * [cite_start]**Stream C: Grain Enveloping:** Implement the selectable amplitude envelopes (e.g., Hann, Trapezoid) that will shape the attack and decay of each grain[cite: 15].

#### **Sprint 3: Audio Synthesis & Integration (2 Weeks)**

* **Primary Goal:** Combine the sources from Sprint 1 with the logic from Sprint 2 to produce the final audio output.
* **Parallel Workstreams:**
    * [cite_start]**Stream A: Pitch Shifting:** Implement the high-quality resampling algorithm (e.g., Windowed-Sinc) to pitch-shift grains with professional audio quality and minimal artifacts[cite: 40, 54]. This is a critical, focused task.
    * **Stream B: Engine Integration:** Connect the stochastic parameter models (Pitch, Density, etc.) to the grain scheduler and the new resampler. The engine should now produce audible, controllable sound.
    * **Stream C: Test Harness:** Build a simple, internal debugging interface to change parameters at runtime and validate the audio output, ensuring all controls work as expected.

#### **Review Sprint A: Engine Performance & Test Coverage (1 Week)**

* **Primary Goal:** To harden and optimize the core engine.
* **All-Hands Activities:**
    * [cite_start]**Profiling:** rigorously test the engine to ensure CPU usage remains below 15% on a reference machine with a typical load of 128 grains[cite: 39].
    * **Unit Testing:** Write comprehensive tests for all DSP and stochastic components, verifying mathematical correctness and checking for edge cases.
    * **Refactoring:** Optimize code based on profiler results and fix all discovered bugs.

***

### **EPIC 3 & 5: UI, Visualization & Preset Management**

These sprints run parallel to the engine development, starting after Sprint 0. They rely on the interfaces defined in the scaffolding phase, allowing UI to be built with mock data until final integration.

#### 📢 **Dependency Deadline: Start of Sprint 4**
* [cite_start]**UI/UX Design Team:** Deliver final wireframes, high-fidelity mockups of the "Pod" controls, visual asset files (images, icons), and specifications for the real-time visualization's appearance[cite: 23, 25].

#### **Sprint 4: UI Scaffolding & Visualization (3 Weeks)**

* **Primary Goal:** Build the main UI window and the complex real-time visualization.
* **Parallel Workstreams:**
    * **Stream A: UI Layout:** Construct the main plugin window and layout using JUCE components. [cite_start]Create placeholder components for the large interactive "Pods"[cite: 26].
    * **Stream B: Visualization Backend:** Using Skia, implement the rendering canvas. [cite_start]Develop the logic to draw and animate hundreds of grain visuals at a target of 60fps[cite: 42, 60]. [cite_start]This stream will use a *mock data generator* to simulate grain information (pan, pitch, size)[cite: 24].
    * [cite_start]**Stream C: Pod Component v1:** Develop the foundational custom JUCE component for the "Pod" controls, focusing on visual styling and basic interaction without its detailed editor panel yet[cite: 64].

#### **Sprint 5: Interactive Controls & Presets (3 Weeks)**

* **Primary Goal:** Make the UI fully interactive and implement the preset system logic.
* **Parallel Workstreams:**
    * [cite_start]**Stream A: Pod Interaction:** Extend the "Pod" components to include the "click-to-edit" functionality, revealing a more detailed parameter editor view for each control[cite: 27, 65].
    * [cite_start]**Stream B: Preset Serialization:** Using `nlohmann/json`, implement the logic to save the entire state of the instrument to a human-readable JSON file[cite: 35, 67, 68].
    * [cite_start]**Stream C: Preset Browser:** Develop the complete UI for the preset browser, including loading presets, Browse factory content, and filtering based on tags[cite: 36, 37, 38]. This can be tested with hand-written JSON files.

#### **Sprint 6: Full Integration (2 Weeks)**

* **Primary Goal:** Connect the audio engine to the user interface to create the complete MVP.
* **All-Hands Activities:**
    * **Engine-UI Connection:** Remove the mock data generator from the visualization and feed it with live data from the audio engine.
    * **Control Hookup:** Connect the UI "Pod" controls to the corresponding stochastic parameters in the C++ engine.
    * [cite_start]**Feedback Loop:** Ensure that changing a parameter on the UI is immediately and accurately reflected in the real-time visualization and the audio output[cite: 28].
    * **Preset Finalization:** Verify that saving and loading presets correctly restores the complete state of both the audio engine and the UI.

#### **Review Sprint B: MVP Stability & Finalization (2 Weeks)**

* **Primary Goal:** Ensure the complete MVP is stable, performant, and ready for users.
* **All-Hands Activities:**
    * [cite_start]**DAW Testing:** Perform rigorous stability testing in all target DAWs (Ableton Live, Logic Pro X, Pro Tools, etc.) to identify and fix compatibility issues or crashes[cite: 42].
    * **E2E Testing:** Conduct full end-to-end testing, simulating user workflows from sound generation to preset saving.
    * [cite_start]**Performance Review:** Profile the entire plugin (engine + UI) to confirm that UI responsiveness and CPU usage meet the non-functional requirements[cite: 39, 42].
    * [cite_start]**Sound Design:** The **Sound Design Team's** primary task this sprint is to create a comprehensive suite of factory presets that showcase the instrument's capabilities[cite: 37].

***

### **EPIC 4: Modulation System (V2)**

This epic builds upon the stable MVP to add dynamic, evolving textures.

#### **Sprint 7: Modulation Sources (2 Weeks)**

* **Primary Goal:** Create the core modulation sources (LFOs and Envelopes).
* **Parallel Workstreams:**
    * [cite_start]**Stream A: LFOs:** Develop a flexible LFO generator with standard waveshapes (sine, saw, random, etc.)[cite: 30].
    * [cite_start]**Stream B: ADSR Envelopes:** Implement a MIDI-triggerable ADSR envelope generator[cite: 31].
    * **Stream C: Modulation Architecture:** Design the underlying `ModulationTarget` and `ModulationSource` system that will allow parameters to be animated.

#### **Sprint 8: Modulation Routing & UI (2 Weeks)**

* **Primary Goal:** Allow the user to connect modulation sources to their destinations.
* **Parallel Workstreams:**
    * [cite_start]**Stream A: Routing Logic:** Implement the modulation matrix or drag-and-drop system that applies a modulation source to a destination parameter with a controllable amount[cite: 32, 33].
    * **Stream B: UI Implementation:** Design and build the user interface components for assigning and visualizing modulation.
    * **Stream C: Integration:** Connect the routing logic to the UI and the audio engine's parameters.

#### **Review Sprint C: V2 Polish & Final Testing (2 Weeks)**

* **Primary Goal:** Finalize the V2 product with a full feature set.
* **All-Hands Activities:**
    * **Regression Testing:** Test the entire application to ensure the new modulation features have not negatively impacted performance or stability.
    * **Sound Design:** The **Sound Design Team** creates new presets that specifically highlight the LFO and envelope capabilities.
    * **Final Review:** Code cleanup, final bug fixes, and preparation for release.
