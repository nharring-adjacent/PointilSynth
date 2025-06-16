#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Pointilsynth/PresetManager.h"

namespace audio_plugin {

class PresetBrowserComponent : public juce::Component,
                               private juce::ListBoxModel {
public:
  explicit PresetBrowserComponent(Pointilism::PresetManager& manager);
  ~PresetBrowserComponent() override = default;

  void paint(juce::Graphics& g) override;
  void resized() override;

  int getNumRows() override;
  void paintListBoxItem(int rowNumber,
                        juce::Graphics& g,
                        int width,
                        int height,
                        bool rowIsSelected) override;

private:
  struct PresetEntry {
    juce::File file;
    juce::String category;
  };

  void scanPresetDirectory();
  void loadSelectedPreset();
  void savePreset();

  Pointilism::PresetManager& presetManager;

  juce::ListBox presetList;
  juce::TextButton loadButton{"Load"};
  juce::TextButton saveButton{"Save"};
  juce::TextEditor presetNameEditor;
  juce::ComboBox categoryCombo;

  juce::File presetDirectory;
  std::vector<PresetEntry> presets;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserComponent)
};

}  // namespace audio_plugin
