#pragma once

// Modern, modular JUCE includes instead of the deprecated JuceHeader.h
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "Oscillator.h"
#include "GrainEnvelope.h"
#include "InertialHistoryManager.h"
#include "ConfigManager.h"

#include <vector>
#include <random>
#include <atomic>

/**
 * @struct Grain
 * @brief A plain data structure representing a single sonic event.
 *
 * This structure holds all distinct properties for one grain of sound,
 * assigned at the moment of its creation. The AudioEngine will manage a
 * collection of these.
 */
struct Grain {
  bool isAlive = true;  // Flag to mark for cleanup when the grain is finished.
  int id = 0;           // Unique identifier for visualization purposes.

  // Core sonic properties
  float pitch = 60.0f;        // MIDI note number.
  float pan = 0.0f;           // Stereo position from -1.0 (L) to 1.0 (R).
  float amplitude = 0.0f;     // Amplitude from 0.0 to 1.0.
  int durationInSamples = 0;  // Total lifetime of the grain in audio samples.

  // Playback state
  int ageInSamples = 0;  // How many samples this grain has been playing.
  double sourceSamplePosition =
      0.0;  // The starting position within the source audio file.
};

/**
 * @struct GrainInfoForVis
 * @brief Lightweight data passed from the AudioEngine to the GUI thread.
 */
struct GrainInfoForVis {
  float pan{};
  float pitch{};
  float durationSeconds{};
};

/**
 * @class StochasticModel
 * @brief Manages the probability distributions that govern grain creation.
 *
 * This class is the core of the "pointillistic" concept. It uses the C++
 * <random> library to generate properties for new grains based on user-defined
 * parameters. The UI thread will call the 'set' methods, and the audio thread
 * will call the 'generate' methods. Parameters are atomic to ensure thread
 * safety.
 */
class ConfigManager;  // Forward declaration

class StochasticModel {
public:
  // This enum will be used by the UI to select the temporal distribution model.
  enum class TemporalDistribution {
    Uniform,
    Poisson  // Random
  };

  //==============================================================================
  // Parameter Setters (called by the UI thread)
  //==============================================================================
  explicit StochasticModel(std::shared_ptr<ConfigManager> cfg = {});

  void setPitchAndDispersion(float centralPitchValue, float dispersionValue) {
    pitch.store(centralPitchValue);
    dispersion.store(dispersionValue);
    pitchDistribution =
        std::normal_distribution<float>(centralPitchValue, dispersionValue);
  }
  void setDurationAndVariation(float averageDurationMsValue,
                               float variationValue);
  void setPanAndSpread(float centralPanValue, float spreadValue) {
    centralPan.store(centralPanValue);
    panSpread.store(spreadValue);
    panDistribution =
        std::normal_distribution<float>(centralPanValue, spreadValue);
  }
  void setGlobalDensity(float densityValue) {  // Renamed from setDensity
    globalDensity_.store(densityValue);  // Updated from grainsPerSecond_ to
                                         // globalDensity_ and parameter name
    // If Poisson distribution rate depends on this, update here.
    // e.g., if using globalDensity_ directly for poisson lambda:
    // poissonDistribution_ = std::poisson_distribution<int>(densityValue);
    // More likely, it's related to sampleRate: events_per_sample = densityValue
    // / sampleRate And then lambda = events_per_sample * block_size for poisson
    // events per block. This logic is typically in getSamplesUntilNextEvent()
    // or similar.
  }
  void setGlobalMinDistance(float minDistValue) {
    globalMinDistance_.store(minDistValue);
  }
  void setGlobalPitchOffset(int pitchOffsetValue) {
    globalPitchOffset_.store(pitchOffsetValue);
  }
  void setGlobalPanOffset(float panOffsetValue) {
    globalPanOffset_.store(panOffsetValue);
  }
  void setGlobalVelocityOffset(float velocityOffsetValue) {
    globalVelocityOffset_.store(velocityOffsetValue);
  }
  void setGlobalDurationOffset(float durationOffsetValue) {
    globalDurationOffset_.store(durationOffsetValue);
  }
  void setGlobalTempoSyncEnabled(bool tempoSyncEnabledValue) {
    globalTempoSyncEnabled_.store(tempoSyncEnabledValue);
  }
  void setGlobalNumVoices(int numVoicesValue) {
    globalNumVoices_.store(numVoicesValue);
  }
  void setGlobalNumGrains(int numGrainsValue) {
    globalNumGrains_.store(numGrainsValue);
  }
  void setGlobalTemporalDistribution(
      TemporalDistribution
          modelValue) {  // Renamed from setTemporalDistribution
    globalTemporalDistribution_.store(
        modelValue);  // Updated from temporalDistributionModel_
  }

  //==============================================================================
  // Parameter Getters (called by UI thread via DebugUIPanel)
  //==============================================================================
  float getPitch() const { return pitch.load(); }
  float getDispersion() const { return dispersion.load(); }
  float getAverageDurationMs() const { return averageDurationMs_.load(); }
  float getDurationVariation() const { return durationVariation_.load(); }
  float getCentralPan() const { return centralPan.load(); }
  float getPanSpread() const { return panSpread.load(); }
  float getGlobalDensity() const {
    return globalDensity_.load();
  }  // Renamed from getGrainsPerSecond
  float getGlobalMinDistance() const { return globalMinDistance_.load(); }
  int getGlobalPitchOffset() const { return globalPitchOffset_.load(); }
  float getGlobalPanOffset() const { return globalPanOffset_.load(); }
  float getGlobalVelocityOffset() const { return globalVelocityOffset_.load(); }
  float getGlobalDurationOffset() const { return globalDurationOffset_.load(); }
  bool isGlobalTempoSyncEnabled() const {
    return globalTempoSyncEnabled_.load();
  }  // Note: 'is' prefix for bool
  int getGlobalNumVoices() const { return globalNumVoices_.load(); }
  int getGlobalNumGrains() const { return globalNumGrains_.load(); }
  TemporalDistribution getGlobalTemporalDistribution() const {
    return globalTemporalDistribution_.load();
  }  // Renamed from getTemporalDistributionModel
  double getSampleRate() const { return sampleRate_.load(); }

  //==============================================================================
  // Value Generators (called by the AudioEngine thread)
  //==============================================================================

  /** Generates the number of samples to wait before triggering the next grain.
   */
  int getSamplesUntilNextEvent();

  /** Fills a Grain struct with new, randomized properties based on the current
   * model. */
  void generateNewGrain(Grain& newGrain);

private:
  std::shared_ptr<ConfigManager> config_;
  // Private members would include std::mt19937 for random number generation,
  // and various std::distribution objects (e.g., normal_distribution,
  // uniform_real_distribution, poisson_distribution) to model the parameters.
  // Ensure randomEngine is seeded, e.g., in constructor or a dedicated init
  // method.
  std::mt19937 randomEngine{std::random_device{}()};  // Example seeding
  std::poisson_distribution<int> poissonDistribution_{
      10.0};  // Example initialization
  std::uniform_real_distribution<float> uniformRealDistribution_{-0.5f, 0.5f};

  // Parameters controlled by the UI (using std::atomic for thread-safety)
  std::atomic<float> pitch{60.0f};
  std::atomic<float> dispersion{12.0f};           // e.g. in semitones
  std::atomic<float> averageDurationMs_{200.0f};  // e.g. in milliseconds
  std::atomic<float> durationVariation_{0.25f};   // e.g. 25% variation
  std::atomic<float> globalDensity_{10.0f};       // Formerly grainsPerSecond_
  std::atomic<float> globalMinDistance_{0.0f};    // New parameter
  std::atomic<int> globalPitchOffset_{0};         // New parameter
  std::atomic<float> globalPanOffset_{0.0f};      // New parameter
  std::atomic<float> globalVelocityOffset_{0.0f};
  std::atomic<float> globalDurationOffset_{0.0f};
  std::atomic<bool> globalTempoSyncEnabled_{false};
  std::atomic<int> globalNumVoices_{1};
  std::atomic<int> globalNumGrains_{100};
  std::atomic<TemporalDistribution> globalTemporalDistribution_{
      TemporalDistribution::Uniform};  // Formerly temporalDistributionModel_
  std::atomic<double> sampleRate_{
      44100.0};  // Should be set by prepareToPlay in AudioEngine
  std::atomic<float> centralPan{0.0f};  // -1 (L) to 1 (R)
  std::atomic<float> panSpread{0.5f};   // 0 (no spread) to 1 (full spread)

  // MIDI influence parameters
  std::atomic<float> midiTargetPitch_{60.0f};
  std::atomic<float> midiInfluence_{0.0f};

  // Distributions - these should be updated when parameters change.
  // For simplicity, the setters currently just store values. A more complete
  // implementation would update these distributions in the setters.
  std::normal_distribution<float> pitchDistribution{pitch.load(),
                                                    dispersion.load()};
  std::normal_distribution<float> panDistribution{centralPan.load(),
                                                  panSpread.load()};
  // Duration distribution might be normal or log-normal depending on desired
  // behavior
  std::normal_distribution<float> durationDistribution{
      averageDurationMs_.load(),
      averageDurationMs_.load() * durationVariation_.load()};

public:  // Public setter for sample rate, to be called by AudioEngine
  void setSampleRate(double sr);

  // Method to set MIDI influence
  void setMidiInfluence(int noteNumber, float influenceAmount);
};

/**
 * @class AudioEngine
 * @brief The high-performance heart of the instrument.
 *
 * This class is responsible for managing and rendering all active grains. It
 * will be driven by the main JUCE AudioProcessor's processBlock. Its goal is to
 * maintain high CPU efficiency while managing up to the target number of
 * grains.
 */
class AudioEngine {
public:
  enum class GrainSourceType { Oscillator, AudioSample };

  explicit AudioEngine(std::shared_ptr<ConfigManager> cfg = {},
                       juce::AbstractFifo* visFifo = nullptr,
                       GrainInfoForVis* visBuffer = nullptr);

  /** Called by the host to prepare the engine for playback. */
  void prepareToPlay(double sampleRate, int samplesPerBlock);

  /**
   * Processes a block of audio. This is where all DSP happens.
   * This method will be called repeatedly on the real-time audio thread.
   */
  void processBlock(juce::AudioBuffer<float>& buffer,
                    juce::MidiBuffer& midiMessages,
                    const juce::AudioPlayHead::PositionInfo& pos);

  /** Loads a user-provided audio file to be used as a grain source. */
  void loadAudioSample(const juce::File& audioFile);

  /** Selects an internal waveform to be used as a grain source. */
  void setGrainSource(int internalWaveformId);

  /** Provides a non-owning pointer to the model for the UI to control. */
  StochasticModel* getStochasticModel() { return &stochasticModel; }

  /** Applies MIDI note and velocity influence to the stochastic model. */
  void applyMidiInfluence(int noteNumber, float normalizedVelocity);

private:
  double currentSampleRate = 44100.0;
  int grainIdCounter = 0;

  // The model that dictates the properties of new grains.
  StochasticModel stochasticModel;

  std::shared_ptr<ConfigManager> config_;

  // A pre-allocated vector to hold all active grains. We reserve to avoid
  // real-time memory allocation.
  std::vector<Grain> grains;

  // A counter to determine when to ask the StochasticModel for a new grain.
  int samplesUntilNextGrain = 0;

  // Placeholder for the loaded audio file data.
  juce::AudioBuffer<float> sourceAudio;

  Pointilsynth::Oscillator oscillator_;
  GrainEnvelope grainEnvelope_;
  std::atomic<GrainSourceType> currentSourceType_{GrainSourceType::Oscillator};

  juce::AbstractFifo* visualizationFifo_{};
  GrainInfoForVis* visualizationBuffer_{};

  InertialHistoryManager inertialHistoryManager_;

  void triggerNewGrain();
};
