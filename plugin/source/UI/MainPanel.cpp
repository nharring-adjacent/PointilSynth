#include "UI/MainPanel.h"
#include "UI/VisualizationComponent.h"
#include "UI/SynthesisTab.h"
#include "UI/ModulationTab.h"
#include "UI/PresetBrowserComponent.h"
#include "UI/TooltipManager.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace audio_plugin {

// Custom LookAndFeel for consistent styling
class PointilSynthLookAndFeel : public juce::LookAndFeel_V4 {
public:
    PointilSynthLookAndFeel() {
        setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff2d2d30));
        setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff3f3f41));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0e639c));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff3f3f41));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff0e639c));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff3f3f41));
    }
    
    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override {
        auto area = button.getActiveArea();
        
        if (button.getToggleState()) {
            g.setColour(findColour(juce::TabbedComponent::backgroundColourId).brighter(0.1f));
        } else if (isMouseOver) {
            g.setColour(findColour(juce::TabbedComponent::backgroundColourId).darker(0.1f));
        } else {
            g.setColour(findColour(juce::TabbedComponent::backgroundColourId));
        }
        
        g.fillRect(area);
        
        g.setColour(button.getToggleState() ? juce::Colours::white : juce::Colours::lightgrey);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(button.getButtonText(), area, juce::Justification::centred, false);
    }
};

MainPanel::MainPanel(juce::AudioProcessorValueTreeState& apvts, 
                     juce::AudioProcessorEditor& editor)
    : apvts_(apvts)
    , editor_(editor)
    , tabbedComponent_(juce::TabbedButtonBar::TabsAtTop)
    , visualizationComponent_(dummyFifo_, dummyBuffer_.data())
    , presetManager_(std::make_unique<PresetManager>(apvts)) {
    
    // Setup look and feel
    lnf_ = std::make_unique<PointilSynthLookAndFeel>();
    setLookAndFeel(lnf_.get());
    
    // Setup tabs
    setupTabs();
    
    // Setup controls
    setupControls();
    
    // Listen to parameter changes
    apvts_.addParameterListener("VIS_PRESET", this);
    
    // Add and make visible
    addAndMakeVisible(tabbedComponent_);
    addAndMakeVisible(presetSelector_.get());
    
    // Set initial size
    setSize(800, 600);
}

MainPanel::~MainPanel() {
    setLookAndFeel(nullptr);
    apvts_.removeParameterListener("VIS_PRESET", this);
}

void MainPanel::resized() {
    auto bounds = getLocalBounds();
    
    // Preset selector at top
    presetSelector_->setBounds(bounds.removeFromTop(30).reduced(5, 2));
    
    // Tabbed component takes the rest
    tabbedComponent_.setBounds(bounds);
}

void MainPanel::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainPanel::setupTabs() {
    // Create tab components with actual implementations
    synthTab_ = std::make_unique<SynthesisTab>(apvts_);
    modTab_ = std::make_unique<ModulationTab>(apvts_);
    
    // Create preset browser tab
    auto presetBrowser = std::make_unique<PresetBrowserComponent>(*presetManager_);
    
    // Create placeholder tabs for now
    fxTab_ = std::make_unique<juce::Component>("Effects");
    visTab_ = std::make_unique<juce::Component>("Visualization");
    
    // Add tabs - preset browser first
    tabbedComponent_.addTab("Presets", juce::Colours::transparentBlack, presetBrowser.get(), false);
    tabbedComponent_.addTab("Synthesis", juce::Colours::transparentBlack, synthTab_.get(), false);
    tabbedComponent_.addTab("Effects", juce::Colours::transparentBlack, fxTab_.get(), false);
    tabbedComponent_.addTab("Modulation", juce::Colours::transparentBlack, modTab_.get(), false);
    tabbedComponent_.addTab("Visualization", juce::Colours::transparentBlack, visTab_.get(), false);
    
    // Store the unique_ptr to manage the lifetime
    presetBrowser_ = std::move(presetBrowser);
    
    // Setup visualization tab
    visTab_->addAndMakeVisible(visualizationComponent_);
    visualizationComponent_.setBounds(visTab_->getLocalBounds());
    
    // Set initial tab to Synthesis (skip presets for now)
    tabbedComponent_.setCurrentTabIndex(1);
    
    // Setup tooltips
    auto& tooltipManager = TooltipManager::getInstance();
    tooltipManager.setTooltip(&tabbedComponent_, "Switch between different parameter sections");
    tooltipManager.setTooltip(presetSelector_.get(), "Select a visual preset");
}

void MainPanel::setupControls() {
    // Create preset selector
    presetSelector_ = std::make_unique<juce::ComboBox>("PresetSelector");
    presetSelector_->addItem("Default", 1);
    presetSelector_->addItem("Minimalist", 2);
    presetSelector_->addItem("Retro", 3);
    presetSelector_->addItem("Particle Wave", 4);
    presetSelector_->addItem("Spectral", 5);
    presetSelector_->setSelectedId(1);
    
    presetSelector_->onChange = [this] {
        int presetId = presetSelector_->getSelectedId();
        auto preset = static_cast<VisualizationComponent::VisualPreset>(presetId - 1);
        visualizationComponent_.setVisualPreset(preset);
    };
}

void MainPanel::parameterChanged(const juce::String& parameterID, float newValue) {
    if (parameterID == "VIS_PRESET") {
        int presetId = static_cast<int>(newValue);
        if (presetSelector_ && presetId > 0 && presetId <= presetSelector_->getNumItems()) {
            presetSelector_->setSelectedId(presetId, juce::sendNotificationSync);
        }
    }
}

void MainPanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    // Handle value tree changes if needed
}

} // namespace audio_plugin
