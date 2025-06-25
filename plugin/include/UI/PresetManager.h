#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include <memory>

namespace audio_plugin {

class PresetManager : public juce::ValueTree::Listener {
public:
    struct Preset {
        juce::String name;
        juce::ValueTree state;
        
        bool isValid() const { return state.isValid(); }
    };
    
    static const juce::File& getPresetDirectory();
    
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() override;
    
    // Preset management
    bool savePreset(const juce::String& presetName);
    bool loadPreset(const juce::String& presetName);
    bool deletePreset(const juce::String& presetName);
    
    // Preset list management
    juce::StringArray getPresetList() const;
    juce::String getCurrentPresetName() const;
    
    // State management
    void setDefaultState();
    
    // Listeners
    void addChangeListener(juce::ChangeListener* listener);
    void removeChangeListener(juce::ChangeListener* listener);
    
private:
    juce::AudioProcessorValueTreeState& apvts_;
    juce::File presetDirectory_;
    juce::ChangeBroadcaster changeBroadcaster_;
    
    static const juce::Identifier presetNameProperty_;
    
    juce::ValueTree createPresetValueTree(const juce::String& name) const;
    juce::File getPresetFile(const juce::String& presetName) const;
    bool isValidPreset(const juce::ValueTree& v) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace audio_plugin
