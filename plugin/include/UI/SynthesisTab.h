#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/CustomKnob.h"

namespace audio_plugin {

class SynthesisTab : public juce::Component,
                     private juce::ValueTree::Listener {
public:
    SynthesisTab(juce::AudioProcessorValueTreeState& apvts);
    ~SynthesisTab() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    void setupControls();
    void parameterChanged(const juce::String& parameterID, float newValue);
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    
    juce::AudioProcessorValueTreeState& apvts_;
    
    // Sections
    juce::Component grainControls_;
    juce::Component envelopeControls_;
    juce::Component pitchControls_;
    juce::Component filterControls_;
    
    // Grain Controls
    std::unique_ptr<CustomKnob> densityKnob_;
    std::unique_ptr<CustomKnob> durationKnob_;
    std::unique_ptr<CustomKnob> spreadKnob_;
    
    // Envelope Controls
    std::unique_ptr<CustomKnob> attackKnob_;
    std::unique_ptr<CustomKnob> decayKnob_;
    std::unique_ptr<CustomKnob> sustainKnob_;
    std::unique_ptr<CustomKnob> releaseKnob_;
    
    // Pitch Controls
    std::unique_ptr<CustomKnob> transposeKnob_;
    std::unique_ptr<CustomKnob> detuneKnob_;
    std::unique_ptr<CustomKnob> randomPitchKnob_;
    
    // Filter Controls
    std::unique_ptr<CustomKnob> filterCutoffKnob_;
    std::unique_ptr<CustomKnob> filterResonanceKnob_;
    std::unique_ptr<CustomKnob> filterEnvAmountKnob_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesisTab)
};

} // namespace audio_plugin
