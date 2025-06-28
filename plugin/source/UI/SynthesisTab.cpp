#include "UI/SynthesisTab.h"
#include "UI/CustomKnob.h"
#include "UI/DetailedKnobEditor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace audio_plugin {

SynthesisTab::SynthesisTab(juce::AudioProcessorValueTreeState& apvts)
    : apvts_(apvts) {
    
    // Setup sections
    addAndMakeVisible(grainControls_);
    addAndMakeVisible(envelopeControls_);
    addAndMakeVisible(pitchControls_);
    addAndMakeVisible(filterControls_);
    
    // Setup controls
    setupControls();
    
    // Set up parameter listeners
    for (auto* param : apvts_.getParameters()) {
        param->addListener(this);
    }
    
    // Set initial size
    setSize(800, 400);
}

SynthesisTab::~SynthesisTab() {
    for (auto* param : apvts_.getParameters()) {
        param->removeListener(this);
    }
}

void SynthesisTab::paint(juce::Graphics& g) {
    // Draw section backgrounds
    auto bounds = getLocalBounds();
    
    auto drawSectionBackground = [&](juce::Component& section, const juce::String& name) {
        auto sectionBounds = section.getBounds().toFloat();
        
        // Background
        g.setColour(juce::Colour(0x20ffffff));
        g.fillRoundedRectangle(sectionBounds, 4.0f);
        
        // Border
        g.setColour(juce::Colour(0x40ffffff));
        g.drawRoundedRectangle(sectionBounds, 4.0f, 1.0f);
        
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(name, sectionBounds.getX() + 8, sectionBounds.getY() + 4, 200, 20, 
                  juce::Justification::left, false);
    };
    
    drawSectionBackground(grainControls_, "Grain");
    drawSectionBackground(envelopeControls_, "Envelope");
    drawSectionBackground(pitchControls_, "Pitch");
    drawSectionBackground(filterControls_, "Filter");
}

void SynthesisTab::resized() {
    auto bounds = getLocalBounds();
    const int padding = 8;
    const int sectionHeight = bounds.getHeight() / 2 - padding * 2;
    const int sectionWidth = bounds.getWidth() / 2 - padding * 2;
    
    // Position sections
    grainControls_.setBounds(0, 0, sectionWidth, sectionHeight);
    envelopeControls_.setBounds(sectionWidth + padding * 2, 0, sectionWidth, sectionHeight);
    pitchControls_.setBounds(0, sectionHeight + padding * 2, sectionWidth, sectionHeight);
    filterControls_.setBounds(sectionWidth + padding * 2, sectionHeight + padding * 2, sectionWidth, sectionHeight);
    
    // Position controls within sections
    auto positionKnobs = [](juce::Component& parent, std::vector<CustomKnob*> knobs) {
        auto bounds = parent.getLocalBounds().reduced(8, 24);
        const int knobWidth = bounds.getWidth() / knobs.size();
        
        for (size_t i = 0; i < knobs.size(); ++i) {
            knobs[i]->setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
        }
    };
    
    // Position knobs in each section
    positionKnobs(grainControls_, {densityKnob_.get(), durationKnob_.get(), spreadKnob_.get()});
    positionKnobs(envelopeControls_, {attackKnob_.get(), decayKnob_.get(), sustainKnob_.get(), releaseKnob_.get()});
    positionKnobs(pitchControls_, {transposeKnob_.get(), detuneKnob_.get(), randomPitchKnob_.get()});
    positionKnobs(filterControls_, {filterCutoffKnob_.get(), filterResonanceKnob_.get(), filterEnvAmountKnob_.get()});
}

void SynthesisTab::setupControls() {
    // Helper function to create and setup a knob
    auto createKnob = [this](const juce::String& id, const juce::String& name, 
                            float min, float max, float defaultValue, 
                            const juce::String& suffix = "") -> std::unique_ptr<CustomKnob> {
        auto knob = std::make_unique<CustomKnob>(name);
        knob->setRange(min, max);
        knob->setValue(defaultValue);
        knob->setTextValueSuffix(suffix);
        
        // Attach to APVTS parameter
        attachments_.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts_, id, *knob));
        
        knob->onClick = [this, id]() {
            if (detailedEditor_) {
                removeChildComponent(detailedEditor_.get());
                detailedEditor_.reset();
            }
            detailedEditor_ = std::make_unique<DetailedKnobEditor>(apvts_, id);
            addAndMakeVisible(detailedEditor_.get());
            detailedEditor_->setBounds(getLocalBounds());
            detailedEditor_->toFront(true);
        };
        
        return knob;
    };
    
    // Grain Controls
    densityKnob_ = createKnob("DENSITY", "Density", 0.1f, 100.0f, 10.0f, " Hz");
    durationKnob_ = createKnob("DURATION", "Duration", 1.0f, 1000.0f, 100.0f, " ms");
    spreadKnob_ = createKnob("SPREAD", "Spread", 0.0f, 1.0f, 0.2f, " %");
    
    // Envelope Controls
    attackKnob_ = createKnob("ATTACK", "Attack", 0.0f, 5.0f, 0.1f, " s");
    decayKnob_ = createKnob("DECAY", "Decay", 0.0f, 5.0f, 0.3f, " s");
    sustainKnob_ = createKnob("SUSTAIN", "Sustain", 0.0f, 1.0f, 0.7f, "");
    releaseKnob_ = createKnob("RELEASE", "Release", 0.0f, 5.0f, 0.2f, " s");
    
    // Pitch Controls
    transposeKnob_ = createKnob("TRANSPOSE", "Transpose", -24.0f, 24.0f, 0.0f, " st");
    detuneKnob_ = createKnob("DETUNE", "Detune", -50.0f, 50.0f, 0.0f, " ct");
    randomPitchKnob_ = createKnob("RAND_PITCH", "Random", 0.0f, 1.0f, 0.1f, " st");
    
    // Filter Controls
    filterCutoffKnob_ = createKnob("FILTER_CUTOFF", "Cutoff", 20.0f, 20000.0f, 10000.0f, " Hz");
    filterResonanceKnob_ = createKnob("FILTER_RESO", "Resonance", 0.1f, 10.0f, 1.0f, "");
    filterEnvAmountKnob_ = createKnob("FILTER_ENV_AMT", "Env Amt", -1.0f, 1.0f, 0.0f, "");
    
    // Add controls to sections
    for (auto* knob : {densityKnob_.get(), durationKnob_.get(), spreadKnob_.get()}) {
        grainControls_.addAndMakeVisible(knob);
    }
    
    for (auto* knob : {attackKnob_.get(), decayKnob_.get(), sustainKnob_.get(), releaseKnob_.get()}) {
        envelopeControls_.addAndMakeVisible(knob);
    }
    
    for (auto* knob : {transposeKnob_.get(), detuneKnob_.get(), randomPitchKnob_.get()}) {
        pitchControls_.addAndMakeVisible(knob);
    }
    
    for (auto* knob : {filterCutoffKnob_.get(), filterResonanceKnob_.get(), filterEnvAmountKnob_.get()}) {
        filterControls_.addAndMakeVisible(knob);
    }
}

void SynthesisTab::parameterChanged(const juce::String& parameterID, float newValue) {
    // Handle parameter changes if needed
}

void SynthesisTab::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    // Handle value tree changes if needed
}

} // namespace audio_plugin
