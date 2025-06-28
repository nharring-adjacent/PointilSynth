#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace audio_plugin {

class DetailedKnobEditor : public juce::Component {
public:
    DetailedKnobEditor(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);
    ~DetailedKnobEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts_;
    juce::String paramID_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetailedKnobEditor)
};

} // namespace audio_plugin
