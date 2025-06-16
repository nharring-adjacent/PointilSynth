#include "UI/PresetBrowserComponent.h"
#include "nlohmann/json.hpp"

namespace audio_plugin {

PresetBrowserComponent::PresetBrowserComponent(
    Pointilism::PresetManager& manager)
    : presetManager(manager) {
  addAndMakeVisible(presetList);
  presetList.setModel(this);

  addAndMakeVisible(loadButton);
  addAndMakeVisible(saveButton);
  addAndMakeVisible(presetNameEditor);
  addAndMakeVisible(categoryCombo);

  categoryCombo.addItem("All", 1);
  categoryCombo.addItem("Texture", 2);
  categoryCombo.addItem("Rhythmic", 3);
  categoryCombo.addItem("FX", 4);
  categoryCombo.onChange = [this] { scanPresetDirectory(); };

  loadButton.onClick = [this] { loadSelectedPreset(); };
  saveButton.onClick = [this] { savePreset(); };

  presetDirectory =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile("PointilSynthPresets");
  presetDirectory.createDirectory();

  scanPresetDirectory();
}

void PresetBrowserComponent::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PresetBrowserComponent::resized() {
  auto area = getLocalBounds().reduced(5);
  categoryCombo.setBounds(area.removeFromTop(25));
  presetList.setBounds(area.removeFromTop(getHeight() - 90));
  loadButton.setBounds(area.removeFromTop(25).removeFromLeft(60));
  presetNameEditor.setBounds(area.removeFromTop(25).removeFromLeft(150));
  saveButton.setBounds(area.removeFromTop(25).removeFromLeft(60));
}

int PresetBrowserComponent::getNumRows() {
  return static_cast<int>(presets.size());
}

void PresetBrowserComponent::paintListBoxItem(int rowNumber,
                                              juce::Graphics& g,
                                              int width,
                                              int height,
                                              bool rowIsSelected) {
  if (rowNumber < 0 || rowNumber >= getNumRows())
    return;

  if (rowIsSelected) {
    g.fillAll(juce::Colours::lightblue);
  }
  g.setColour(juce::Colours::white);
  g.drawText(presets[static_cast<size_t>(rowNumber)]
                 .file.getFileNameWithoutExtension(),
             0, 0, width, height, juce::Justification::centredLeft, true);
}

void PresetBrowserComponent::scanPresetDirectory() {
  presets.clear();
  juce::String selected = categoryCombo.getText();
  if (selected.isEmpty())
    selected = "All";
  auto files =
      presetDirectory.findChildFiles(juce::File::findFiles, false, "*.json");
  for (auto& f : files) {
    juce::String category;
    juce::FileInputStream in(f);
    if (in.openedOk()) {
      auto text = in.readEntireStreamAsString();
      try {
        auto j = nlohmann::json::parse(text.toStdString());
        if (j.contains("category"))
          category = j["category"].get<std::string>();
      } catch (...) {
      }
    }
    if (selected == "All" || category == selected)
      presets.push_back({f, category});
  }
  presetList.updateContent();
  presetList.repaint();
}

void PresetBrowserComponent::loadSelectedPreset() {
  int row = presetList.getSelectedRow();
  if (row >= 0 && row < getNumRows())
    presetManager.loadPreset(presets[static_cast<size_t>(row)].file);
}

void PresetBrowserComponent::savePreset() {
  auto name = presetNameEditor.getText();
  if (name.isEmpty())
    return;
  juce::File file =
      presetDirectory.getChildFile(name).withFileExtension(".json");
  if (presetManager.savePreset(file)) {
    juce::String category = categoryCombo.getText();
    if (category == "All")
      category = "Uncategorized";
    juce::FileInputStream in(file);
    nlohmann::json j;
    if (in.openedOk()) {
      auto text = in.readEntireStreamAsString();
      try {
        j = nlohmann::json::parse(text.toStdString());
      } catch (...) {
        j = nlohmann::json{};
      }
    }
    j["category"] = category.toStdString();
    juce::FileOutputStream out(file);
    std::string data = j.dump(4);
    out.write(data.c_str(), data.size());
    scanPresetDirectory();
  }
}

}  // namespace audio_plugin
