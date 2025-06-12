#pragma once


#include "PluginProcessor.h" // Adjusted path
#include "DebugUIPanel.h"       // Added for DebugUIPanel
// PointilismInterfaces.h is included by PluginProcessor.h, which is included above.
// If direct use of StochasticModel type was needed here, an include would be good:
// #include "../../source/PointilismInterfaces.h"

namespace audio_plugin {

class PointillisticSynthAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit PointillisticSynthAudioProcessorEditor(audio_plugin::AudioPluginAudioProcessor&);
    ~PointillisticSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    audio_plugin::AudioPluginAudioProcessor& processorRef;

    DebugUIPanel debugUIPanel; // Added DebugUIPanel member
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PointillisticSynthAudioProcessorEditor)

};

} // namespace audio_plugin
