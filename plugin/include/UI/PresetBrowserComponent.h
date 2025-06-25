#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/PresetManager.h"

namespace audio_plugin {

class PresetBrowserComponent : public juce::Component,
                               private juce::ListBoxModel,
                               private juce::ChangeListener {
public:
    explicit PresetBrowserComponent(PresetManager& manager);
    ~PresetBrowserComponent() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics&, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
    
    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
private:
    struct PresetEntry {
        juce::File file;
        juce::String category;
    };
    
    void scanPresetDirectory();
    void loadSelectedPreset();
    void savePreset();
    
    PresetManager& presetManager;
    
    // UI Components
    juce::ListBox presetList;
    juce::TextButton loadButton;
    juce::TextButton saveButton;
    juce::TextEditor presetNameEditor;
    juce::ComboBox categoryCombo;
    
    // Data
    juce::File presetDirectory;
    std::vector<PresetEntry> presets;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserComponent)
};

} // namespace audio_plugin
