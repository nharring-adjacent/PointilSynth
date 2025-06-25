#include "UI/PresetManager.h"
#include <juce_core/juce_core.h>

namespace audio_plugin {

const juce::Identifier PresetManager::presetNameProperty_ { "presetName" };

const juce::File& PresetManager::getPresetDirectory() {
    static juce::File dir = [] {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("PointilSynth/Presets");
        
        if (!dir.exists()) {
            dir.createDirectory();
        }
        
        return dir;
    }();
    
    return dir;
}

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvts_(apvts)
    , presetDirectory_(getPresetDirectory()) {
    
    // Load default state if no state is present
    if (apvts_.state.getNumChildren() == 0) {
        setDefaultState();
    }
}

PresetManager::~PresetManager() {
    // Clean up
}

bool PresetManager::savePreset(const juce::String& presetName) {
    if (presetName.isEmpty()) {
        return false;
    }
    
    auto presetFile = getPresetFile(presetName);
    auto presetState = createPresetValueTree(presetName);
    
    // Save the preset
    juce::FileOutputStream stream(presetFile);
    
    if (stream.openedOk()) {
        stream.setPosition(0);
        stream.truncate();
        
        std::unique_ptr<juce::XmlElement> xml(presetState.createXml());
        if (xml && xml->writeTo(stream, {})) {
            changeBroadcaster_.sendChangeMessage();
            return true;
        }
    }
    
    return false;
}

bool PresetManager::loadPreset(const juce::String& presetName) {
    auto presetFile = getPresetFile(presetName);
    
    if (!presetFile.existsAsFile()) {
        return false;
    }
    
    juce::FileInputStream stream(presetFile);
    
    if (stream.openedOk()) {
        auto xml = juce::XmlDocument::parse(stream);
        
        if (xml) {
            auto presetState = juce::ValueTree::fromXml(*xml);
            
            if (isValidPreset(presetState)) {
                // Copy parameter values from the preset
                for (auto& param : apvts_.getParameters()) {
                    if (auto* paramWithID = dynamic_cast<juce::RangedAudioParameter*>(param)) {
                        auto paramID = paramWithID->paramID;
                        auto value = presetState.getProperty(paramID, param->getValue());
                        param->setValueNotifyingHost(static_cast<float>(value));
                    }
                }
                
                changeBroadcaster_.sendChangeMessage();
                return true;
            }
        }
    }
    
    return false;
}

bool PresetManager::deletePreset(const juce::String& presetName) {
    auto presetFile = getPresetFile(presetName);
    
    if (presetFile.existsAsFile() && presetFile.deleteFile()) {
        changeBroadcaster_.sendChangeMessage();
        return true;
    }
    
    return false;
}

juce::StringArray PresetManager::getPresetList() const {
    juce::StringArray presets;
    auto presetFiles = presetDirectory_.findChildFiles(
        juce::File::findFiles, false, "*.xml");
    
    for (const auto& file : presetFiles) {
        presets.add(file.getFileNameWithoutExtension());
    }
    
    return presets;
}

juce::String PresetManager::getCurrentPresetName() const {
    return apvts_.state.getProperty(presetNameProperty_).toString();
}

void PresetManager::setDefaultState() {
    // Reset all parameters to their default values
    for (auto* param : apvts_.getParameters()) {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            param->setValueNotifyingHost(rangedParam->getDefaultValue());
        }
    }
    
    apvts_.state.removeProperty(presetNameProperty_, nullptr);
    changeBroadcaster_.sendChangeMessage();
}

void PresetManager::addChangeListener(juce::ChangeListener* listener) {
    changeBroadcaster_.addChangeListener(listener);
}

void PresetManager::removeChangeListener(juce::ChangeListener* listener) {
    changeBroadcaster_.removeChangeListener(listener);
}

juce::ValueTree PresetManager::createPresetValueTree(const juce::String& name) const {
    juce::ValueTree preset("PRESET");
    preset.setProperty(presetNameProperty_, name, nullptr);
    
    // Save all parameter values
    for (auto* param : apvts_.getParameters()) {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            preset.setProperty(rangedParam->paramID, param->getValue(), nullptr);
        }
    }
    
    return preset;
}

juce::File PresetManager::getPresetFile(const juce::String& presetName) const {
    return presetDirectory_.getChildFile(presetName + ".xml");
}

bool PresetManager::isValidPreset(const juce::ValueTree& v) const {
    return v.hasType("PRESET") && v.hasProperty(presetNameProperty_);
}

} // namespace audio_plugin
