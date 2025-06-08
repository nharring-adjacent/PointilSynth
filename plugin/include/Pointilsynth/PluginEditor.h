#pragma once

#include "PluginProcessor.h"
#include "../../source/DebugUIPanel.h"       // Added for DebugUIPanel
// PointilismInterfaces.h is included by PluginProcessor.h, which is included above.
// If direct use of StochasticModel type was needed here, an include would be good:
// #include "../../source/PointilismInterfaces.h"

namespace audio_plugin {

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef;

  DebugUIPanel debugUIPanel; // Added DebugUIPanel member

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
}  // namespace audio_plugin
