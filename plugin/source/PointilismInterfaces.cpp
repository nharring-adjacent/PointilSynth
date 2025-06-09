#include "PointilismInterfaces.h" // Should be relative to the plugin/source directory
#include <juce_core/juce_core.h>   // For DBG if needed, or other utilities

// StochasticModel method implementations
int StochasticModel::getSamplesUntilNextEvent()
{
    // Stub implementation: return a fixed value or simple calculation
    // For Poisson distribution, this would involve -log(uniform_random) / lambda_per_sample
    // For Uniform, it would be a fixed number of samples based on density.
    // Example: 10 events per second, 44100 sample rate -> 4410 samples per event.
    if (globalDensity_.load() > 0) {
        return static_cast<int>(sampleRate_.load() / static_cast<double>(globalDensity_.load()));
    }
    return static_cast<int>(sampleRate_.load()); // Default to 1 second if density is 0 or less
}

void StochasticModel::generateNewGrain(Grain& newGrain)
{
    // Stub implementation: fill with some default or simple random values
    newGrain.isAlive = true;
    newGrain.id = 0; // This will be overridden by AudioEngine's counter
    newGrain.ageInSamples = 0;

    // Pitch: Use normal distribution around centralPitch, with dispersion
    newGrain.pitch = pitchDistribution(randomEngine) + static_cast<float>(globalPitchOffset_.load());
    newGrain.pitch = juce::jlimit(0.0f, 127.0f, newGrain.pitch); // MIDI pitch range

    // Pan: Use normal distribution around centralPan, with spread
    newGrain.pan = panDistribution(randomEngine) + globalPanOffset_.load();
    newGrain.pan = juce::jlimit(-1.0f, 1.0f, newGrain.pan); // Pan range L-R

    // Amplitude: For now, a fixed value or simple random. Could be linked to velocity later.
    newGrain.amplitude = uniformRealDistribution_(randomEngine) * 0.5f + 0.5f; // e.g. 0.5 to 1.0
    newGrain.amplitude = juce::jlimit(0.0f, 1.0f, newGrain.amplitude + globalVelocityOffset_.load());

    // Duration: Use normal distribution around averageDurationMs, with variation
    float durationMs = durationDistribution(randomEngine) + globalDurationOffset_.load();
    durationMs = juce::jmax(10.0f, durationMs); // Ensure minimum duration (e.g., 10ms)
    newGrain.durationInSamples = static_cast<int>(static_cast<double>(durationMs / 1000.0f) * sampleRate_.load());

    // sourceSamplePosition: For now, start at 0 or random if using sample source
    newGrain.sourceSamplePosition = 0.0;
    // If you had a sourceAudio buffer length:
    // if (sourceAudioBufferLength > 0) {
    //     std::uniform_real_distribution<double> sourcePosDist(0.0, static_cast<double>(sourceAudioBufferLength));
    //     newGrain.sourceSamplePosition = sourcePosDist(randomEngine);
    // }
}

// Note: AudioEngine methods are defined in AudioEngine.cpp
// No need for Grain struct methods as it's a plain data struct.
