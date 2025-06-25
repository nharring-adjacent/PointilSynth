#include "UI/ModulationTab.h"
#include "UI/CustomKnob.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace audio_plugin {

ModulationTab::ModulationTab(juce::AudioProcessorValueTreeState& apvts)
    : apvts_(apvts) {
    
    // Setup sections
    addAndMakeVisible(lfo1Section_);
    addAndMakeVisible(lfo2Section_);
    addAndMakeVisible(env1Section_);
    addAndMakeVisible(modMatrixSection_);
    
    // Setup look and feel
    lnf_ = std::make_unique<juce::LookAndFeel_V4>();
    setLookAndFeel(lnf_.get());
    
    // Setup controls
    setupControls();
    setupModulationMatrix();
    
    // Set initial size
    setSize(800, 400);
}

ModulationTab::~ModulationTab() {
    setLookAndFeel(nullptr);
}

void ModulationTab::paint(juce::Graphics& g) {
    // Draw section backgrounds
    auto drawSectionBackground = [&](juce::Component& section, const juce::String& name) {
        auto bounds = section.getBounds().toFloat();
        
        // Background
        g.setColour(juce::Colour(0x20ffffff));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Border
        g.setColour(juce::Colour(0x40ffffff));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(name, bounds.getX() + 8, bounds.getY() + 4, 200, 20, 
                  juce::Justification::left, false);
    };
    
    drawSectionBackground(lfo1Section_, "LFO 1");
    drawSectionBackground(lfo2Section_, "LFO 2");
    drawSectionBackground(env1Section_, "Envelope 1");
    drawSectionBackground(modMatrixSection_, "Modulation Matrix");
}

void ModulationTab::resized() {
    auto bounds = getLocalBounds();
    const int padding = 8;
    
    // Calculate section sizes
    const int sectionHeight = bounds.getHeight() / 2 - padding * 2;
    const int lfoWidth = bounds.getWidth() / 4 - padding * 2;
    const int envWidth = bounds.getWidth() / 4 - padding * 2;
    const int matrixWidth = bounds.getWidth() / 2 - padding * 2;
    
    // Position sections
    lfo1Section_.setBounds(0, 0, lfoWidth, sectionHeight);
    lfo2Section_.setBounds(lfoWidth + padding * 2, 0, lfoWidth, sectionHeight);
    env1Section_.setBounds(0, sectionHeight + padding * 2, envWidth, sectionHeight);
    modMatrixSection_.setBounds(envWidth + padding * 2, sectionHeight + padding * 2, 
                               matrixWidth, sectionHeight);
    
    // Position controls within LFO sections
    auto positionLFOControls = [](juce::Component& parent, 
                                 CustomKnob* rateKnob,
                                 CustomKnob* amountKnob,
                                 juce::ComboBox* waveformCombo) {
        auto bounds = parent.getLocalBounds().reduced(8, 24);
        auto topRow = bounds.removeFromTop(bounds.getHeight() / 2);
        
        // Position knobs in top row
        rateKnob->setBounds(topRow.removeFromLeft(topRow.getWidth() / 2).reduced(4));
        amountKnob->setBounds(topRow.reduced(4));
        
        // Position combo box in bottom row
        waveformCombo->setBounds(bounds.reduced(4));
    };
    
    positionLFOControls(lfo1Section_, lfo1RateKnob_.get(), lfo1AmountKnob_.get(), lfo1WaveformCombo_.get());
    positionLFOControls(lfo2Section_, lfo2RateKnob_.get(), lfo2AmountKnob_.get(), lfo2WaveformCombo_.get());
    
    // Position envelope controls
    auto positionEnvelopeControls = [](juce::Component& parent, 
                                      CustomKnob* attack, CustomKnob* decay,
                                      CustomKnob* sustain, CustomKnob* release) {
        auto bounds = parent.getLocalBounds().reduced(8, 24);
        const int knobWidth = bounds.getWidth() / 4;
        
        attack->setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
        decay->setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
        sustain->setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
        release->setBounds(bounds.reduced(4));
    };
    
    positionEnvelopeControls(env1Section_, 
                            env1AttackKnob_.get(), env1DecayKnob_.get(),
                            env1SustainKnob_.get(), env1ReleaseKnob_.get());
    
    // Position modulation matrix
    auto matrixBounds = modMatrixSection_.getLocalBounds().reduced(8, 24);
    const int rowHeight = matrixBounds.getHeight() / 8;
    
    for (auto& row : modMatrixRows_) {
        auto rowBounds = matrixBounds.removeFromTop(rowHeight).reduced(2);
        
        if (row.sourceCombo && row.targetCombo && row.amountKnob) {
            const int comboWidth = rowBounds.getWidth() * 2 / 5;
            const int knobWidth = rowBounds.getWidth() / 5;
            
            row.sourceCombo->setBounds(rowBounds.removeFromLeft(comboWidth).reduced(2));
            row.targetCombo->setBounds(rowBounds.removeFromLeft(comboWidth).reduced(2));
            row.amountKnob->setBounds(rowBounds.reduced(2));
        }
    }
}

void ModulationTab::setupControls() {
    // Helper function to create and setup a knob
    auto createKnob = [this](const juce::String& id, const juce::String& name, 
                            float min, float max, float defaultValue, 
                            const juce::String& suffix = "") -> std::unique_ptr<CustomKnob> {
        auto knob = std::make_unique<CustomKnob>(name);
        knob->setRange(min, max);
        knob->setValue(defaultValue);
        knob->setTextValueSuffix(suffix);
        
        // Attach to APVTS parameter
        attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts_, id, *knob);
        
        return knob;
    };
    
    // LFO 1 Controls
    lfo1RateKnob_ = createKnob("LFO1_RATE", "Rate", 0.1f, 20.0f, 1.0f, " Hz");
    lfo1AmountKnob_ = createKnob("LFO1_AMOUNT", "Amount", 0.0f, 1.0f, 0.5f);
    
    lfo1WaveformCombo_ = std::make_unique<juce::ComboBox>("LFO1_WAVEFORM");
    lfo1WaveformCombo_->addItem("Sine", 1);
    lfo1WaveformCombo_->addItem("Triangle", 2);
    lfo1WaveformCombo_->addItem("Square", 3);
    lfo1WaveformCombo_->addItem("Saw", 4);
    lfo1WaveformCombo_->addItem("Random", 5);
    lfo1WaveformCombo_->setSelectedId(1);
    
    // LFO 2 Controls
    lfo2RateKnob_ = createKnob("LFO2_RATE", "Rate", 0.1f, 20.0f, 0.5f, " Hz");
    lfo2AmountKnob_ = createKnob("LFO2_AMOUNT", "Amount", 0.0f, 1.0f, 0.3f);
    
    lfo2WaveformCombo_ = std::make_unique<juce::ComboBox>("LFO2_WAVEFORM");
    lfo2WaveformCombo_->addItem("Sine", 1);
    lfo2WaveformCombo_->addItem("Triangle", 2);
    lfo2WaveformCombo_->addItem("Square", 3);
    lfo2WaveformCombo_->addItem("Saw", 4);
    lfo2WaveformCombo_->addItem("Random", 5);
    lfo2WaveformCombo_->setSelectedId(1);
    
    // Envelope 1 Controls
    env1AttackKnob_ = createKnob("ENV1_ATTACK", "A", 0.0f, 5.0f, 0.1f, " s");
    env1DecayKnob_ = createKnob("ENV1_DECAY", "D", 0.0f, 5.0f, 0.3f, " s");
    env1SustainKnob_ = createKnob("ENV1_SUSTAIN", "S", 0.0f, 1.0f, 0.7f);
    env1ReleaseKnob_ = createKnob("ENV1_RELEASE", "R", 0.0f, 5.0f, 0.2f, " s");
    
    // Add controls to sections
    lfo1Section_.addAndMakeVisible(lfo1RateKnob_.get());
    lfo1Section_.addAndMakeVisible(lfo1AmountKnob_.get());
    lfo1Section_.addAndMakeVisible(lfo1WaveformCombo_.get());
    
    lfo2Section_.addAndMakeVisible(lfo2RateKnob_.get());
    lfo2Section_.addAndMakeVisible(lfo2AmountKnob_.get());
    lfo2Section_.addAndMakeVisible(lfo2WaveformCombo_.get());
    
    env1Section_.addAndMakeVisible(env1AttackKnob_.get());
    env1Section_.addAndMakeVisible(env1DecayKnob_.get());
    env1Section_.addAndMakeVisible(env1SustainKnob_.get());
    env1Section_.addAndMakeVisible(env1ReleaseKnob_.get());
}

void ModulationTab::setupModulationMatrix() {
    const char* sources[] = {
        "None", "LFO 1", "LFO 2", "Envelope 1", "Velocity", "Pitch", "Mod Wheel"
    };
    
    const char* targets[] = {
        "None", 
        "Pitch", "Filter Cutoff", "Filter Resonance", "Volume", 
        "Pan", "Grain Density", "Grain Size"
    };
    
    for (auto& row : modMatrixRows_) {
        // Source combo
        row.sourceCombo = std::make_unique<juce::ComboBox>();
        for (int i = 0; i < 7; ++i) {
            row.sourceCombo->addItem(sources[i], i + 1);
        }
        row.sourceCombo->setSelectedId(1);
        
        // Target combo
        row.targetCombo = std::make_unique<juce::ComboBox>();
        for (int i = 0; i < 8; ++i) {
            row.targetCombo->addItem(targets[i], i + 1);
        }
        row.targetCombo->setSelectedId(1);
        
        // Amount knob
        row.amountKnob = std::make_unique<CustomKnob>("");
        row.amountKnob->setRange(-1.0, 1.0);
        row.amountKnob->setValue(0.0);
        
        // Add to UI
        modMatrixSection_.addAndMakeVisible(row.sourceCombo.get());
        modMatrixSection_.addAndMakeVisible(row.targetCombo.get());
        modMatrixSection_.addAndMakeVisible(row.amountKnob.get());
    }
}

void ModulationTab::parameterChanged(const juce::String& parameterID, float newValue) {
    // Handle parameter changes if needed
}

void ModulationTab::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    // Handle value tree changes if needed
}

} // namespace audio_plugin
