#include "Pointilsynth/PointilismInterfaces.h"  // Assuming this is the correct path relative to the cpp file
#include "Pointilsynth/ConfigManager.h"
#include <random>  // For distributions if needed directly in setters, though likely just for storage here
#include <cmath>   // For std::abs, std::max etc. if needed
#include <limits>  // For std::numeric_limits<double>::epsilon(), INT_MAX
#include <algorithm>  // Will be needed for std::clamp in other methods

StochasticModel::StochasticModel(std::shared_ptr<ConfigManager> cfg)
    : config_(std::move(cfg)) {
  if (config_) {
    config_->addListener(ConfigManager::ParamID::pitch,
                         [this](float v) { pitch = v; });
    config_->addListener(ConfigManager::ParamID::dispersion,
                         [this](float v) { dispersion = v; });
    config_->addListener(ConfigManager::ParamID::avgDuration,
                         [this](float v) { averageDurationMs_ = v; });
    config_->addListener(ConfigManager::ParamID::durationVariation,
                         [this](float v) { durationVariation_ = v; });
    config_->addListener(ConfigManager::ParamID::pan,
                         [this](float v) { centralPan = v; });
    config_->addListener(ConfigManager::ParamID::panSpread,
                         [this](float v) { panSpread = v; });
    config_->addListener(ConfigManager::ParamID::density,
                         [this](float v) { globalDensity_ = v; });
    config_->addListener(
        ConfigManager::ParamID::temporalDistribution, [this](float v) {
          globalTemporalDistribution_ =
              static_cast<TemporalDistribution>(static_cast<int>(v));
        });
  }
}

// Forward declaration or ensure StochasticModel is fully defined via header
// class StochasticModel; // Not needed if PointilismInterfaces.h includes full
// definition

void StochasticModel::setDurationAndVariation(float averageDurationMs,
                                              float variation) {
  averageDurationMs_ = averageDurationMs;
  durationVariation_ = variation;
}

void StochasticModel::setSampleRate(double newSampleRate) {
  sampleRate_ = newSampleRate;
}

void StochasticModel::generateNewGrain(Grain& newGrain) {
  // 1. Get atomic values
  float avgDurationMs = averageDurationMs_.load(std::memory_order_relaxed);
  float variation = durationVariation_.load(
      std::memory_order_relaxed);  // This is the 'variation' parameter, e.g.,
                                   // 0.1 for 10%
  double currentSampleRate = sampleRate_.load(std::memory_order_relaxed);

  // 2. Calculate randomized duration factor
  // uniformRealDistribution_ is [-0.5f, 0.5f].
  // randomPercentDeviation will be in range [-variation, +variation]
  // e.g. if variation = 0.1, then randomPercentDeviation is in [-0.1, 0.1]
  float randomPercentDeviation =
      uniformRealDistribution_(randomEngine) * 2.0f * variation;

  // 3. Randomized duration in ms
  float randomizedDurationMs = avgDurationMs * (1.0f + randomPercentDeviation);

  // 4. Ensure duration is not negative
  randomizedDurationMs = std::max(0.0f, randomizedDurationMs);

  // 5. Convert to samples
  // Ensure sampleRate is not zero to prevent division by zero or NaN issues.
  if (currentSampleRate <= 0) {
    // Fallback to a default sample rate, or log an error.
    // For now, using a common default.
    currentSampleRate = 44100.0;
  }
  newGrain.durationInSamples = static_cast<int>(
      (static_cast<double>(randomizedDurationMs) / 1000.0) * currentSampleRate);

  // Other members of Grain (pitch, pan, amplitude, etc.) are not set here
  // as per the current task. They would be set by other parts of
  // StochasticModel or have default values. Pitch

  // Retrieve base pitch, MIDI target pitch, and MIDI influence
  float basePitch = pitch.load(std::memory_order_relaxed);
  float targetPitch = midiTargetPitch_.load(std::memory_order_relaxed);
  float influence = midiInfluence_.load(std::memory_order_relaxed);

  // Calculate effective pitch based on MIDI influence
  float effectivePitch = (basePitch * (1.0f - influence)) + (targetPitch * influence);

  if (influence > 0.0f) {
    DBG("StochasticModel::generateNewGrain - MIDI Influence Active:");
    DBG("  Base Pitch: " + juce::String(basePitch));
    DBG("  Target Pitch: " + juce::String(targetPitch));
    DBG("  Influence: " + juce::String(influence));
    DBG("  Effective Pitch: " + juce::String(effectivePitch));
  }

  using PitchDistributionParams = std::normal_distribution<float>::param_type;
  pitchDistribution.param(
      PitchDistributionParams(effectivePitch, dispersion.load(std::memory_order_relaxed)));
  newGrain.pitch = pitchDistribution(randomEngine);

  // Pan
  using PanDistributionParams = std::normal_distribution<float>::param_type;
  panDistribution.param(
      PanDistributionParams(centralPan.load(), panSpread.load()));
  float generatedPan = panDistribution(randomEngine);
  newGrain.pan = std::clamp(generatedPan, -1.0f, 1.0f);

  // Set a default amplitude so grains are audible. This could be
  // parameterized in a future update.
  newGrain.amplitude = 0.2f;

  // Start reading from the beginning of the source sample when applicable.
  newGrain.sourceSamplePosition = 0.0;

  // Other grain properties will be set elsewhere or in future tasks.
  // For now, ensure isAlive is true and assign a temporary ID or leave as
  // default.
  newGrain.isAlive = true;
  // newGrain.id = ...; // Not part of this task's core requirements for
  // pitch/pan newGrain.amplitude = ...; newGrain.durationInSamples = ...;
  // newGrain.ageInSamples = 0;
  // newGrain.sourceSamplePosition = ...;
}

int StochasticModel::getSamplesUntilNextEvent() {
  // 1. Get atomic values
  float currentGrainsPerSecond = globalDensity_.load(std::memory_order_relaxed);
  double currentSampleRate = sampleRate_.load(std::memory_order_relaxed);
  TemporalDistribution currentModel =
      globalTemporalDistribution_.load(std::memory_order_relaxed);

  // 2. Validate inputs and calculate average samples per grain
  if (currentGrainsPerSecond <= 0.0f || currentSampleRate <= 0.0) {
    // Cannot compute a meaningful event interval, return a very large number of
    // samples to effectively pause event generation.
    return INT_MAX;
  }

  double averageSamplesPerGrain =
      currentSampleRate / static_cast<double>(currentGrainsPerSecond);

  // Check for potential issues with averageSamplesPerGrain before using it,
  // especially with the Poisson distribution.
  if (averageSamplesPerGrain <= 0.0 || std::isinf(averageSamplesPerGrain) ||
      std::isnan(averageSamplesPerGrain)) {
    return INT_MAX;  // Not a valid mean for Poisson or for timing.
  }

  // 3. Handle TemporalDistribution models
  if (currentModel == TemporalDistribution::Uniform) {
    return static_cast<int>(averageSamplesPerGrain);
  } else if (currentModel == TemporalDistribution::Poisson) {
    // Ensure the mean for the Poisson distribution is positive.
    // Clamping to a very small positive number if averageSamplesPerGrain is too
    // small, as Poisson distribution mean must be > 0.
    double poissonMean = std::max(std::numeric_limits<double>::epsilon(),
                                  averageSamplesPerGrain);

    // Update the Poisson distribution's mean parameter.
    // This is technically not thread-safe if multiple threads call this
    // concurrently AND randomEngine is shared without locks. However,
    // randomEngine itself is not thread-safe by default. Assuming single audio
    // thread access for generation, or external synchronization for
    // randomEngine if shared. For typical JUCE audio processing, this method
    // and generateNewGrain would be called from the same audio thread.
    poissonDistribution_.param(
        std::poisson_distribution<int>::param_type(poissonMean));
    return poissonDistribution_(randomEngine);
  }

  // Fallback, though ideally all enum values should be handled.
  return INT_MAX;
}

void StochasticModel::setMidiInfluence(int noteNumber, float influenceAmount) {
  midiTargetPitch_.store(static_cast<float>(noteNumber));
  midiInfluence_.store(influenceAmount);
}
