#include "Pointilsynth/AudioEngine.h"
#include "Pointilsynth/InertialHistoryManager.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>

using namespace juce;
namespace audio_plugin {

class AudioEngineTestFixture {
protected:
    AudioEngineTestFixture()
        : engine(std::make_shared<ConfigManager>(), &fifo, buffer.data()) {
        // Set up test audio buffer
        audioBuffer.setSize(2, 512);
        audioBuffer.clear();
        
        // Set up test MIDI buffer
        midiBuffer.clear();
        
        // Set up position info
        positionInfo.setIsPlaying(true);
        positionInfo.setPpqPosition(0.0);
        positionInfo.setBpm(120.0);
        positionInfo.setTimeSignature(TimeSignature{4, 4});
    }
    
    void sendMidiNote(int note, bool isNoteOn, int sampleOffset = 0) {
        MidiMessage msg;
        if (isNoteOn) {
            msg = MidiMessage::noteOn(1, note, 1.0f);
        } else {
            msg = MidiMessage::noteOff(1, note);
        }
        midiBuffer.addEvent(msg, sampleOffset);
    }
    
    void advanceTime(double beats) {
        double bpm = positionInfo.getBpm().orFallback(120.0);
        double seconds = (beats * 60.0) / bpm;
        double sampleRate = 44100.0;
        int samples = static_cast<int>(seconds * sampleRate);
        
        // Process in chunks of 512 samples
        while (samples > 0) {
            int chunkSize = jmin(512, samples);
            audioBuffer.setSize(2, chunkSize, false, false, true);
            audioBuffer.clear();
            
            engine.processBlock(audioBuffer, midiBuffer, positionInfo);
            
            // Update position
            double ppqPerSample = (bpm / 60.0) / sampleRate;
            positionInfo.setPpqPosition(
                positionInfo.getPpqPosition().orFallback(0.0) + 
                (chunkSize * ppqPerSample)
            );
            
            samples -= chunkSize;
        }
        
        // Clear any processed MIDI
        midiBuffer.clear();
    }
    
    // Test components
    AbstractFifo fifo{8};
    std::array<GrainInfoForVis, 8> buffer{};
    AudioBuffer<float> audioBuffer;
    MidiBuffer midiBuffer;
    AudioPlayHead::PositionInfo positionInfo;
    AudioEngine engine;
};

TEST_CASE_METHOD(AudioEngineTestFixture, "MIDI Note Processing") {
    // Set up audio engine
    engine.prepareToPlay(44100.0, 512);
    
    // Send a note on
    sendMidiNote(60, true);
    advanceTime(0.5); // Process half a beat
    
    // Should have active note in history
    const auto& history = engine.getInertialHistoryManager();
    REQUIRE(history.getNumNotes() > 0);
    
    // Should have generated some grains
    REQUIRE_FALSE(engine.getNumActiveGrains() == 0);
    
    // Advance time - note should decay
    advanceTime(4.0); // 1 bar at 120 BPM
    REQUIRE(history.getNumNotes() > 0);
    
    // After more time, note should be removed
    advanceTime(16.0); // 4 more bars
    REQUIRE(history.getNumNotes() == 0);
}

TEST_CASE_METHOD(AudioEngineTestFixture, "Grain Generation") {
    engine.prepareToPlay(44100.0, 512);
    
    // No grains initially
    REQUIRE(engine.getNumActiveGrains() == 0);
    
    // Send a note
    sendMidiNote(64, true);
    advanceTime(0.1);
    
    // Should have grains now
    REQUIRE(engine.getNumActiveGrains() > 0);
    
    // Grains should age and be removed
    advanceTime(2.0);
    
    // Check that old grains are cleaned up
    int initialCount = engine.getNumActiveGrains();
    advanceTime(0.5);
    REQUIRE(engine.getNumActiveGrains() < initialCount);
}

TEST_CASE_METHOD(AudioEngineTestFixture, "Audio Output") {
    engine.prepareToPlay(44100.0, 512);
    
    // Should be silent with no notes
    advanceTime(0.5);
    for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch) {
        auto* channelData = audioBuffer.getReadPointer(ch);
        float rms = 0;
        for (int i = 0; i < audioBuffer.getNumSamples(); ++i) {
            rms += channelData[i] * channelData[i];
        }
        rms = std::sqrt(rms / audioBuffer.getNumSamples());
        REQUIRE(rms < 1e-5f);
    }
    
    // Play a note
    sendMidiNote(60, true);
    advanceTime(0.5);
    
    // Should have audio output
    bool hasAudio = false;
    for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch) {
        auto* channelData = audioBuffer.getReadPointer(ch);
        for (int i = 0; i < audioBuffer.getNumSamples(); ++i) {
            if (std::abs(channelData[i]) > 1e-5f) {
                hasAudio = true;
                break;
            }
        }
        if (hasAudio) break;
    }
    REQUIRE(hasAudio);
}

} // namespace audio_plugin
