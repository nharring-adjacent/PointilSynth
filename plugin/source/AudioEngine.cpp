#include "PointilismInterfaces.h"
#include <vector>
#include <algorithm> // Required for std::remove_if

// Forward declaration of AudioEngine if not fully defined in PointilismInterfaces.h
// Or ensure PointilismInterfaces.h has full class definition before this point.
// Assuming PointilismInterfaces.h contains:
// class AudioEngine {
// public:
//     void prepareToPlay(double sampleRate, int samplesPerBlock);
//     // ... other members
// private:
//     std::vector<Grain> grains;
//     // ... other members
// };

void AudioEngine::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    grains.reserve(1024);
}

// Add the following method:
void AudioEngine::triggerNewGrain()
{
    Grain newGrain;
    newGrain.durationInSamples = 44100; // Default test value
    newGrain.isAlive = true;
    newGrain.ageInSamples = 0;
    grains.push_back(newGrain);
}

// Add the following method:
void AudioEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const int numSamples = buffer.getNumSamples();

    for (auto& grain : grains)
    {
        if (!grain.isAlive)
            continue;

        grain.ageInSamples += numSamples;

        if (grain.ageInSamples >= grain.durationInSamples)
        {
            grain.isAlive = false;
        }
    }

    grains.erase(
        std::remove_if(grains.begin(), grains.end(), [](const Grain& grain) {
            return !grain.isAlive;
        }),
        grains.end()
    );
}
