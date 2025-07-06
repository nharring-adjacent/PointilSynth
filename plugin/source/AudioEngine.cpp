#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>  // For DBG, juce::File
#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/ConfigManager.h"
#include "Pointilsynth/InertialHistoryManager.h"
#include <vector>
#include <algorithm>                 // Required for std::remove_if
#include <cmath>                     // For std::pow, std::cos, std::sin
#include "Pointilsynth/Resampler.h"  // For Resampler::getSample
#include "UI/VisualizationComponent.h"  // For VisualizationComponent definition

AudioEngine::AudioEngine(std::shared_ptr<ConfigManager> cfg)
    : stochasticModel(cfg),
      config_(cfg)
{
    // Initialize without visualization component for now
}

void AudioEngine::initializeVisualization() {
    // Visualization initialization placeholder
    // TODO: Add visualization component support back later
}

void AudioEngine::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
  currentSampleRate = sampleRate;
  stochasticModel.setSampleRate(sampleRate);  // Inform StochasticModel
  samplesUntilNextGrain = stochasticModel.getSamplesUntilNextEvent();
  grains.reserve(1024);  // Pre-allocate memory for grains
  
  // Initialize grain envelope with default settings
  grainEnvelope_.setAttackTime(static_cast<double>(0.1f));  // 100ms attack
  grainEnvelope_.setDecayTime(static_cast<double>(0.2f));   // 200ms decay
  grainEnvelope_.setSustainLevel(static_cast<double>(0.7f)); // 70% sustain
  grainEnvelope_.setReleaseTime(static_cast<double>(0.1f));  // 100ms release
  grainEnvelope_.setShape(GrainEnvelope::Shape::ADSR);
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

  grains.push_back(std::move(newGrain));

  // TODO: Add visualization grain info when visualization component is available
}

void AudioEngine::processBlock(juce::AudioBuffer<float>& buffer,
                               juce::MidiBuffer& midiMessages,
                               const juce::AudioPlayHead::PositionInfo& pos) {
    // TODO: Add visualization update when visualization component is available
    
    // Update inertial history with current position
    currentPpq = pos.getPpqPosition().orFallback(0.0);
    ppqPerBar = 4.0;
    if (auto sig = pos.getTimeSignature()) {
        ppqPerBar = sig->numerator * (4.0 / sig->denominator);
    }
    inertialHistoryManager_.update(currentPpq, ppqPerBar);
    
    // Process MIDI messages
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            float velocity = static_cast<float>(msg.getVelocity()) / 127.0f;
            inertialHistoryManager_.addNote(msg.getNoteNumber(), velocity, currentPpq);
        }
    }
  const int numSamples = buffer.getNumSamples();
  const int numChannels = buffer.getNumChannels();
  
  // Clear the output buffer
  for (int channel = 0; channel < numChannels; ++channel) {
    buffer.clear(channel, 0, numSamples);
  }

  // Update time position and inertial history
  currentPpq = pos.getPpqPosition().orFallback(0.0);
  ppqPerBar = 4.0;
  if (auto sig = pos.getTimeSignature())
    ppqPerBar = sig->numerator * (4.0 / sig->denominator);
  inertialHistoryManager_.update(currentPpq, ppqPerBar);

  // Process MIDI messages
  for (const auto metadata : midiMessages) {
    const auto msg = metadata.getMessage();
    if (msg.isNoteOn()) {
      float velocity = static_cast<float>(msg.getVelocity()) / 127.0f;
      inertialHistoryManager_.addNote(msg.getNoteNumber(), velocity,
                                      currentPpq);
    }
  }

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

  for (int s = 0; s < numSamples; ++s) {
    float outputLeft = 0.0f;
    float outputRight = 0.0f;

    // Process each grain for this sample
    for (auto& grain : grains) {
      if (!grain.isAlive) continue;

      // Check if grain has exceeded its lifetime
      if (grain.ageInSamples >= grain.durationInSamples) {
        grain.isAlive = false;
        continue;
      }

      // Apply any real-time modulation to grain parameters
      float modulatedPitch = grain.pitch;
      float modulatedPan = grain.pan;
      float modulatedAmp = grain.amplitude;
      
      // Apply global volume and pan
      modulatedAmp *= volume_;
      modulatedPan = juce::jlimit(-1.0f, 1.0f, grain.pan + pan_);
      
      // Get sample from source (oscillator or audio file)
      float sampleValue = 0.0f;
      float panPosition = modulatedPan;
      
      if (currentSourceType_.load() == GrainSourceType::Oscillator) {
        // Set oscillator frequency based on modulated grain pitch
        float frequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(static_cast<int>(modulatedPitch + 0.5f)));
        grain.oscillator.setFrequency(frequency);
        sampleValue = grain.oscillator.getNextSample() * modulatedAmp;
      } 
      else if (currentSourceType_.load() == GrainSourceType::AudioSample && 
               sourceAudio.getNumSamples() > 0) {
        // Calculate playback rate considering pitch and global rate
        float effectivePitch = modulatedPitch + (samplePlaybackRate_ - 1.0f) * 12.0f;
        float sourceSampleRate = 44100.0f; // Should be set from loaded sample
        float targetSampleRate = static_cast<float>(currentSampleRate) * std::pow(2.0f, (effectivePitch - 60.0f) / 12.0f);
        float increment = (sourceSampleRate / targetSampleRate) * samplePlaybackRate_;
        
        // Calculate sample position with bounds checking
        float samplePos = static_cast<float>(grain.sourceSamplePosition);
        if (sampleEndPos_ > 0 && samplePos >= sampleEndPos_ * sourceAudio.getNumSamples()) {
          if (loopSample_) {
            samplePos = sampleStartPos_ * static_cast<float>(sourceAudio.getNumSamples());
          } else {
            grain.isAlive = false;
            continue;
          }
        }
        
        // Linear interpolation for smooth playback
        int pos0 = static_cast<int>(std::floor(samplePos));
        float alpha = static_cast<float>(samplePos - pos0);
        
        if (pos0 + 1 < sourceAudio.getNumSamples()) {
          float s1 = sourceAudio.getSample(0, pos0);
          float s2 = sourceAudio.getSample(0, pos0 + 1);
          sampleValue = s1 + alpha * (s2 - s1);
        } else {
          sampleValue = sourceAudio.getSample(0, pos0 % sourceAudio.getNumSamples());
        }
        
        grain.sourceSamplePosition += static_cast<double>(increment);
        sampleValue *= modulatedAmp;
      }

      // Apply grain envelope with optional envelope modulation
      float envelope = grainEnvelope_.getAmplitude(
          grain.ageInSamples, grain.durationInSamples);
      
      // Apply envelope to sample with amplitude modulation
      float processedSample = sampleValue * envelope;

      // Apply width control to stereo image
      float mid = (processedSample * 0.5f) * (1.0f - width_);
      float side = processedSample * 0.5f * width_;
      
      // Apply panning with equal power law
      float panPositionClamped = juce::jlimit(-1.0f, 1.0f, panPosition);
      float panAngle = (panPositionClamped * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;
      float panGainLeft = std::cos(panAngle);
      float panGainRight = std::sin(panAngle);
      
      // Combine mid/side with panning
      outputLeft += (mid + side) * panGainLeft;
      outputRight += (mid - side) * panGainRight;

      // Increment grain's age
      grain.ageInSamples++;
    }

    
    // Write to output buffer (with clipping protection)
    if (numChannels > 0) {
      buffer.addSample(0, s, outputLeft);
      if (numChannels > 1) {
        buffer.addSample(1, s, outputRight);
      }
    }
    
    // Fill any additional channels with stereo mix
    for (int channel = 2; channel < numChannels; ++channel) {
      buffer.addSample(channel, s, (outputLeft + outputRight) * 0.5f);
    }
  }  // End of outer sample loop

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

// Grain Source Controls
void AudioEngine::setGrainSource() {
  currentSourceType_.store(AudioEngine::GrainSourceType::Oscillator);
}

void AudioEngine::setGrainEnvelope(GrainEnvelope::Shape shape, 
                                 float attackMs, float decayMs, 
                                 float sustainLevel, float releaseMs) {
  grainEnvelope_.setShape(shape);
  grainEnvelope_.setAttackTime(attackMs);
  grainEnvelope_.setDecayTime(decayMs);
  grainEnvelope_.setSustainLevel(sustainLevel);
  grainEnvelope_.setReleaseTime(releaseMs);
}

void AudioEngine::setGrainPitch(float basePitch, float pitchVariation, bool quantize) {
    (void)quantize; // suppress unused parameter warning
  stochasticModel.setPitchAndDispersion(basePitch, pitchVariation);
  // Add quantization if needed
}

void AudioEngine::setGrainPan(float pan, float spread) {
  stochasticModel.setPanAndSpread(pan, spread);
}

void AudioEngine::setGrainDuration(float durationMs, float variation) {
  stochasticModel.setDurationAndVariation(durationMs, variation);
}

void AudioEngine::setGrainDensity(float density) {
  stochasticModel.setGlobalDensity(density);
}

void AudioEngine::setGrainVolume(float volume) {
  // Scale all grain amplitudes by this volume
  for (auto& grain : grains) {
    grain.amplitude = grain.amplitude * volume;
  }
}

// Sample Playback Controls
void AudioEngine::setSamplePlaybackParams(float playbackRate, float startPos, float endPos, bool loop) {
  samplePlaybackRate_ = playbackRate;
  sampleStartPos_ = startPos;
  sampleEndPos_ = endPos;
  loopSample_ = loop;
}

// Global Controls
void AudioEngine::setVolume(float volumeDb) {
  volume_ = juce::Decibels::decibelsToGain(volumeDb);
}

void AudioEngine::setPan(float pan) {
  pan_ = juce::jlimit(-1.0f, 1.0f, pan);
}

void AudioEngine::setWidth(float width) {
  width_ = juce::jlimit(0.0f, 1.0f, width);
}

// Modulation Routing
void AudioEngine::setModulationDepth(int source, int target, float depth) {
  // Implementation for modulation routing
  // source: 0=LFO1, 1=LFO2, 2=ENV1, etc.
  // target: 0=pitch, 1=pan, 2=volume, etc.
  modulationMatrix_[static_cast<size_t>(source)][static_cast<size_t>(target)] = depth;
}

// Real-time parameter adjustment
void AudioEngine::setParameter(int parameterId, float value) {
  switch (parameterId) {
    case 0: setGrainDensity(value * 100.0f); break;
    case 1: setGrainPitch(60.0f + value * 48.0f, 0.1f); break;
    case 2: setGrainPan(value * 2.0f - 1.0f, 0.5f); break;
    case 3: setGrainDuration(20.0f + value * 980.0f, 0.3f); break;
    case 4: setVolume(value * 60.0f - 30.0f); break;
    // Add more parameters as needed
  }
}

void AudioEngine::applyMidiInfluence(int noteNumber, float normalizedVelocity) {
  stochasticModel.setMidiInfluence(noteNumber, normalizedVelocity);
}