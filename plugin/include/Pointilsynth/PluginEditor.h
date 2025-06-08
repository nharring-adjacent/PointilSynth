#pragma once

#include "Pointilsynth/PluginProcessor.h" // Adjusted path
#include "UI/PodComponent.h"

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

    PodComponent pitchPod;
    PodComponent densityPod;
    PodComponent durationPod;
    PodComponent panPod;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PointillisticSynthAudioProcessorEditor)
};

} // namespace audio_plugin
