#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h> // For DBG, juce::File
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

void AudioEngine::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) // Uncommented sampleRate
{
    currentSampleRate = sampleRate; // Store sampleRate
    stochasticModel.setSampleRate(sampleRate); // Inform StochasticModel
    grains.reserve(1024); // Existing logic
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
} // End of processBlock

// Implementation of loadAudioSample - MOVED OUTSIDE processBlock
void AudioEngine::loadAudioSample(const juce::File& audioFile)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats(); // Register WAV and AIFF

    // Create a reader for the audio file
    // Note: juce::AudioFormatReader is an abstract class, store as a pointer.
    // Using std::unique_ptr for automatic memory management is a good practice.
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));

    if (reader == nullptr)
    {
        DBG("Error loading audio file: " + audioFile.getFullPathName());
        // Optionally, clear the sourceAudio buffer or handle the error in a more specific way
        sourceAudio.setSize(0, 0); // Clear buffer on error
        return;
    }

    // Resize the sourceAudio buffer to match the file's properties
    // reader->numChannels gives the number of channels
    // reader->lengthInSamples gives the total number of samples in the file
    sourceAudio.setSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));

    // Read the audio data from the file into the sourceAudio buffer
    // Parameters for reader->read:
    // - targetBuffer: pointer to the buffer to fill (&sourceAudio)
    // - startSampleInTargetBuffer: sample offset in the target buffer to start writing to (0)
    // - numSamplesToRead: how many samples to read from the source (reader->lengthInSamples)
    // - sourceSampleOffset: sample offset in the source file to start reading from (0)
    // - fillLeftoversWithSilence: if true, fills remaining buffer with silence if source is shorter (true)
    // - useStereoToMonoConversionIfNecessary: if true, converts stereo to mono if target is mono (true)
    reader->read(
        &sourceAudio,                               // Target buffer
        0,                                          // Start sample in target buffer
        static_cast<int>(reader->lengthInSamples),  // Number of samples to read
        0,                                          // Start sample in source file
        true,                                       // Fill leftovers with silence
        true                                        // Use stereo to mono if necessary
    );

    // The std::unique_ptr will automatically delete the reader when it goes out of scope.
    // No need for `delete reader;` if using std::unique_ptr.

    DBG("Loaded audio file: " + audioFile.getFullPathName() +
        ", Channels: " + juce::String(sourceAudio.getNumChannels()) +
        ", Samples: " + juce::String(sourceAudio.getNumSamples()));
}
