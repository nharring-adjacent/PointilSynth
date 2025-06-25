#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "UI/MainPanel.h"

namespace audio_plugin {

class PluginEditor : public juce::AudioProcessorEditor {
public:
    explicit PluginEditor(juce::AudioProcessor& processor);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    // Access to the main panel for the processor
    MainPanel* getMainPanel() { return mainPanel_.get(); }

private:
    std::unique_ptr<MainPanel> mainPanel_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace audio_plugin
