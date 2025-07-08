#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/CustomKnob.h"

namespace audio_plugin {

class ModulationTab : public juce::Component,
                     private juce::ValueTree::Listener {
public:
    ModulationTab(juce::AudioProcessorValueTreeState& apvts);
    ~ModulationTab() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    void setupControls();
    void setupModulationMatrix();
    void parameterChanged(const juce::String& parameterID, float newValue);
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    
    juce::AudioProcessorValueTreeState& apvts_;
    
    // Sections
    juce::Component lfo1Section_;
    juce::Component lfo2Section_;
    juce::Component env1Section_;
    juce::Component modMatrixSection_;
    
    // LFO 1 Controls
    std::unique_ptr<CustomKnob> lfo1RateKnob_;
    std::unique_ptr<CustomKnob> lfo1AmountKnob_;
    std::unique_ptr<juce::ComboBox> lfo1WaveformCombo_;
    
    // LFO 2 Controls
    std::unique_ptr<CustomKnob> lfo2RateKnob_;
    std::unique_ptr<CustomKnob> lfo2AmountKnob_;
    std::unique_ptr<juce::ComboBox> lfo2WaveformCombo_;
    
    // Envelope 1 Controls
    std::unique_ptr<CustomKnob> env1AttackKnob_;
    std::unique_ptr<CustomKnob> env1DecayKnob_;
    std::unique_ptr<CustomKnob> env1SustainKnob_;
    std::unique_ptr<CustomKnob> env1ReleaseKnob_;
    
    // Modulation Matrix
    struct MatrixRow {
        std::unique_ptr<juce::ComboBox> sourceCombo;
        std::unique_ptr<juce::ComboBox> targetCombo;
        std::unique_ptr<CustomKnob> amountKnob;
    };
    
    std::array<MatrixRow, 8> modMatrixRows_;
    
    // Look and feel
    std::unique_ptr<juce::LookAndFeel> lnf_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationTab)
};

} // namespace audio_plugin
