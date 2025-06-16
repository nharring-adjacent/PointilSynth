#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PointilismInterfaces.h"  // Added for AudioEngine and StochasticModel
#include "ConfigManager.h"

namespace audio_plugin {
class AudioPluginAudioProcessor : public juce::AudioProcessor {
public:
  AudioPluginAudioProcessor();
  // Provide access to the StochasticModel for the editor
  StochasticModel* getStochasticModel() {
    return audioEngine.getStochasticModel();
  }
  std::shared_ptr<ConfigManager> getConfigManager() { return configManager; }
  ~AudioPluginAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

private:
  std::shared_ptr<ConfigManager> configManager;
  AudioEngine audioEngine;  // Added AudioEngine member
  std::atomic<int> activeMidiNote_ {-1};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
}  // namespace audio_plugin
