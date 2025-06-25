#include "UI/PluginEditor.h"
#include "UI/MainPanel.h"
#include "UI/TooltipManager.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace audio_plugin {

PluginEditor::PluginEditor(juce::AudioProcessor& processor)
    : AudioProcessorEditor(processor)
    , mainPanel_(std::make_unique<MainPanel>(
          dynamic_cast<juce::AudioProcessorValueTreeState&>(processor.getAPVTS()),
          *this)) {
    
    // Set the main panel as the content component
    addAndMakeVisible(mainPanel_.get());
    
    // Initialize tooltip manager
    auto& tooltipManager = TooltipManager::getInstance();
    tooltipManager.setMillisecondsBeforeTipAppears(700);
    
    // Set the initial size of the editor
    setResizable(true, true);
    setResizeLimits(800, 600, 1600, 1200);
    
    // Set the initial size (this will trigger resized())
    setSize(1000, 800);
    
    // Set window title
    setLookAndFeel(&getLookAndFeel());
}

PluginEditor::~PluginEditor() {
    setLookAndFeel(nullptr);
}

void PluginEditor::paint(juce::Graphics& g) {
    // Background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
}

void PluginEditor::resized() {
    if (mainPanel_) {
        mainPanel_->setBounds(getLocalBounds());
    }
}

} // namespace audio_plugin
