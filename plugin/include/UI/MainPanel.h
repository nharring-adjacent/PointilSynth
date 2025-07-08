#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "VisualizationComponent.h"
#include "PresetManager.h"
#include "PresetBrowserComponent.h"

namespace audio_plugin {

class MainPanel : public juce::Component,
                  private juce::ValueTree::Listener {
public:
    MainPanel(juce::AudioProcessorValueTreeState& apvts, 
              juce::AudioProcessorEditor& editor);
    ~MainPanel() override;
    
    void resized() override;
    void paint(juce::Graphics&) override;
    
    // Access to visualization component for the processor
    VisualizationComponent* getVisualizationComponent() { return &visualizationComponent_; }
    
private:
    void setupTabs();
    void setupControls();
    void parameterChanged(const juce::String& parameterID, float newValue);
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    
    juce::AudioProcessorValueTreeState& apvts_;
    juce::AudioProcessorEditor& editor_;
    
    // UI Components
    juce::TabbedComponent tabbedComponent_;
    
    // Tabs
    std::unique_ptr<juce::Component> synthTab_;
    std::unique_ptr<juce::Component> fxTab_;
    std::unique_ptr<juce::Component> modTab_;
    std::unique_ptr<juce::Component> visTab_;
    
    // Visualization
    VisualizationComponent visualizationComponent_;
    
    // Presets
    std::unique_ptr<juce::ComboBox> presetSelector_;
    std::unique_ptr<PresetManager> presetManager_;
    std::unique_ptr<PresetBrowserComponent> presetBrowser_;
    
    // Look and feel
    std::unique_ptr<juce::LookAndFeel> lnf_;
    
    // Dummy data for visualization (temporary)
    juce::AbstractFifo dummyFifo_{1024};
    std::array<float, 1024> dummyBuffer_{};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};

} // namespace audio_plugin
