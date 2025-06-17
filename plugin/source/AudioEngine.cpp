#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>  // For DBG, juce::File
#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/ConfigManager.h"
#include <vector>
#include <algorithm>                 // Required for std::remove_if
#include <cmath>                     // For std::pow, std::cos, std::sin
#include "Pointilsynth/Resampler.h"  // For Resampler::getSample

AudioEngine::AudioEngine(std::shared_ptr<ConfigManager> cfg)
    : stochasticModel(std::move(cfg)),
      config_(std::move(cfg)),
      currentLimiterPersonality(LimiterPersonality::Subtle) {}

void AudioEngine::prepareToPlay(double sampleRate, int samplesPerBlock) {
  currentSampleRate = sampleRate;
  oscillator_.setSampleRate(sampleRate);
  stochasticModel.setSampleRate(sampleRate);  // Inform StochasticModel

  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
  // Assuming output is always stereo. If your plugin can change output channel count,
  // this might need to be more dynamic, possibly based on processor's channel layout.
  // For now, hardcoding to 2 as per typical synth plugins.
  spec.numChannels = static_cast<juce::uint32>(2);

  switch (currentLimiterPersonality.load()) {
      case LimiterPersonality::Subtle:
          limiter.setThreshold(-1.0f);
          limiter.setRelease(100.0f);
          break;
      case LimiterPersonality::Aggressive:
          limiter.setThreshold(-3.0f);
          limiter.setRelease(50.0f);
          break;
      case LimiterPersonality::Pumping:
          limiter.setThreshold(-6.0f);
          limiter.setRelease(300.0f);
          break;
      default: // Fallback, though currentLimiterPersonality should always be valid
          limiter.setThreshold(-1.0f);
          limiter.setRelease(100.0f);
          break;
  }

  limiter.prepare(spec);

  samplesUntilNextGrain = stochasticModel.getSamplesUntilNextEvent();
  grains.reserve(1024);  // Keep existing functionality
}

// Add the following method:
void AudioEngine::triggerNewGrain() {
  Grain newGrain;
  stochasticModel.generateNewGrain(newGrain);  // Populate grain properties

  newGrain.id = grainIdCounter++;  // Assign unique ID and increment counter
  newGrain.isAlive = true;         // New grains are always initially alive
  newGrain.ageInSamples = 0;       // New grains start with zero age

  // It's assumed that stochasticModel.generateNewGrain(newGrain) handles:
  // - newGrain.pitch
  // - newGrain.pan
  // - newGrain.amplitude
  // - newGrain.durationInSamples
  // - newGrain.sourceSamplePosition (if applicable for the current source type)

  grains.push_back(newGrain);
}

// Add the following method:
void AudioEngine::processBlock(juce::AudioBuffer<float>& buffer,
                               juce::MidiBuffer& /*midiMessages*/) {
  const int numSamples = buffer.getNumSamples();

  // Update the countdown for the next grain event
  samplesUntilNextGrain -= numSamples;

  // Trigger new grains if the countdown has elapsed
  while (samplesUntilNextGrain <= 0) {
    triggerNewGrain();
    // Add the full duration for the next event, plus any "overshoot" from the
    // current block. This maintains more accurate timing for grain generation.
    samplesUntilNextGrain += stochasticModel.getSamplesUntilNextEvent();
    // Note: StochasticModel::getSamplesUntilNextEvent() must return a positive
    // value to prevent potential infinite loops if it could return 0 or
    // negative.
  }

  // Clear the buffer at the start of the block, after triggering new grains
  buffer.clear();

  // const int numSamples = buffer.getNumSamples(); // This line is already
  // above the triggering logic

  for (int s = 0; s < numSamples;
       ++s)  // Outer loop: iterate through each sample in the block
  {
    float outputLeft = 0.0f;
    float outputRight = 0.0f;

    for (auto& grain : grains)  // Inner loop: iterate through each grain
    {
      if (!grain.isAlive)
        continue;

      // Check if grain's lifetime has just ended in this sample
      if (grain.ageInSamples >= grain.durationInSamples) {
        grain.isAlive = false;  // Mark for cleanup after the main sample loop
        continue;
      }

      float sourceSample = 0.0f;

      // A. Fetch source sample based on grain's source type
      if (currentSourceType_.load() == GrainSourceType::Oscillator) {
        double frequency = juce::MidiMessage::getMidiNoteInHertz(
            static_cast<int>(std::round(grain.pitch)));
        oscillator_.setFrequency(
            static_cast<float>(frequency));  // Tune the shared oscillator
        sourceSample =
            oscillator_.getNextSample();  // Process and advance oscillator
      } else if (currentSourceType_.load() == GrainSourceType::AudioSample) {
        // Ensure sourceAudio is valid and has channels/samples
        if (sourceAudio.getNumSamples() > 0 &&
            sourceAudio.getNumChannels() > 0) {
          int sourceChannelToRead =
              0;  // Default to reading from channel 0
                  // (Resampler expects a specific channel from source)
          sourceSample = Resampler::getSample(sourceAudio, sourceChannelToRead,
                                              grain.sourceSamplePosition);

          // Advance grain's internal playback position for the audio sample,
          // adjusted by pitch
          float baseMidiNote = 60.0f;  // MIDI note 60 is normal speed
          float pitchRatio =
              std::pow(2.0f, (grain.pitch - baseMidiNote) / 12.0f);
          grain.sourceSamplePosition += static_cast<double>(pitchRatio);

          // Note: Resampler::getSample should handle grain.sourceSamplePosition
          // going out of bounds.
        }
      }

      // B. Calculate envelope value using GrainEnvelope
      // grainEnvelope_ is a member of AudioEngine.
      float envelopeValue = grainEnvelope_.getAmplitude(
          grain.ageInSamples, grain.durationInSamples);

      // C. Multiply source sample by envelope value and grain's overall
      // amplitude
      float processedSample = sourceSample * envelopeValue * grain.amplitude;

      // D. Apply panning (Constant Power Panning)
      float panPosition = grain.pan;  // Expected range: -1.0 (L) to 1.0 (R)
      // Map pan position to an angle: -1.0 (L) -> 0, 0.0 (C) -> PI/4, 1.0 (R)
      // -> PI/2
      float panAngle =
          (panPosition * 0.5f + 0.5f) * (juce::MathConstants<float>::pi * 0.5f);
      float panGainLeft = std::cos(panAngle);
      float panGainRight = std::sin(panAngle);

      outputLeft += processedSample * panGainLeft;
      outputRight += processedSample * panGainRight;

      // E. Increment grain's ageInSamples (as it has been processed for one
      // sample)
      grain.ageInSamples++;
    }  // End of inner grain loop

    // Write accumulated stereo signal to the output buffer for the current
    // sample 's'
    if (buffer.getNumChannels() >
        0) {  // Check if buffer has at least one channel
      buffer.setSample(
          0, s, outputLeft);  // Left channel (or mono if numChannels == 1)
    }
    if (buffer.getNumChannels() >
        1) {  // Check if buffer has at least two channels
      buffer.setSample(1, s, outputRight);  // Right channel
    }

    // Optional: Fill any additional channels (e.g., for surround sound setups)
    for (int channel = 2; channel < buffer.getNumChannels(); ++channel) {
      // Example: Mix down stereo to additional channels or set to zero
      buffer.setSample(channel, s, (outputLeft + outputRight) * 0.5f);
    }
  }  // End of outer sample loop

  // --- Apply the limiter ---
  // Wrap the main output buffer as an AudioBlock
  juce::dsp::AudioBlock<float> block(buffer);

  // Create a processing context from this block
  juce::dsp::ProcessContextReplacing<float> context(block);

  // Process the context with the limiter
  limiter.process(context);
  // --- Limiter processing done ---

  // Cleanup dead grains (this part remains from existing code)
  grains.erase(
      std::remove_if(grains.begin(), grains.end(),
                     [](const Grain& grain) { return !grain.isAlive; }),
      grains.end());
}  // End of processBlock

// Implementation of loadAudioSample - MOVED OUTSIDE processBlock
void AudioEngine::loadAudioSample(const juce::File& audioFile) {
  juce::AudioFormatManager formatManager;
  formatManager.registerBasicFormats();  // Register WAV and AIFF

  // Create a reader for the audio file
  // Note: juce::AudioFormatReader is an abstract class, store as a pointer.
  // Using std::unique_ptr for automatic memory management is a good practice.
  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(audioFile));

  if (reader == nullptr) {
    DBG("Error loading audio file: " + audioFile.getFullPathName());
    // Optionally, clear the sourceAudio buffer or handle the error in a more
    // specific way
    sourceAudio.setSize(0, 0);  // Clear buffer on error
    return;
  }

  // Resize the sourceAudio buffer to match the file's properties
  // reader->numChannels gives the number of channels
  // reader->lengthInSamples gives the total number of samples in the file
  sourceAudio.setSize(static_cast<int>(reader->numChannels),
                      static_cast<int>(reader->lengthInSamples));

  // Read the audio data from the file into the sourceAudio buffer
  // Parameters for reader->read:
  // - targetBuffer: pointer to the buffer to fill (&sourceAudio)
  // - startSampleInTargetBuffer: sample offset in the target buffer to start
  // writing to (0)
  // - numSamplesToRead: how many samples to read from the source
  // (reader->lengthInSamples)
  // - sourceSampleOffset: sample offset in the source file to start reading
  // from (0)
  // - fillLeftoversWithSilence: if true, fills remaining buffer with silence if
  // source is shorter (true)
  // - useStereoToMonoConversionIfNecessary: if true, converts stereo to mono if
  // target is mono (true)
  reader->read(
      &sourceAudio,  // Target buffer
      0,             // Start sample in target buffer
      static_cast<int>(reader->lengthInSamples),  // Number of samples to read
      0,                                          // Start sample in source file
      true,                                       // Fill leftovers with silence
      true  // Use stereo to mono if necessary
  );

  // The std::unique_ptr will automatically delete the reader when it goes out
  // of scope. No need for `delete reader;` if using std::unique_ptr.

  DBG("Loaded audio file: " + audioFile.getFullPathName() +
      ", Channels: " + juce::String(sourceAudio.getNumChannels()) +
      ", Samples: " + juce::String(sourceAudio.getNumSamples()));
}

void AudioEngine::setGrainSource(int internalWaveformId) {
  currentSourceType_.store(
      AudioEngine::GrainSourceType::Oscillator);  // Set source type to
                                                  // Oscillator

  Pointilsynth::Oscillator::Waveform selectedWaveform;

  switch (internalWaveformId) {
    case 0:
      selectedWaveform = Pointilsynth::Oscillator::Waveform::Sine;
      break;
    case 1:
      selectedWaveform = Pointilsynth::Oscillator::Waveform::Saw;
      break;
    case 2:
      selectedWaveform = Pointilsynth::Oscillator::Waveform::Square;
      break;
    case 3:
      selectedWaveform = Pointilsynth::Oscillator::Waveform::Noise;
      break;
    default:
      // Default to Sine for unrecognized IDs
      selectedWaveform = Pointilsynth::Oscillator::Waveform::Sine;
      // Consider adding DBG("Unknown internalWaveformId..."); for debugging
      break;
  }
  oscillator_.setWaveform(selectedWaveform);
}

void AudioEngine::applyMidiInfluence(int noteNumber, float normalizedVelocity) {
  stochasticModel.setMidiInfluence(noteNumber, normalizedVelocity);
}

void AudioEngine::setLimiterPersonality(LimiterPersonality personality) {
    currentLimiterPersonality.store(personality);
}
