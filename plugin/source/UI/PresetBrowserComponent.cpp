#include "UI/PresetBrowserComponent.h"
#include "UI/PresetManager.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "nlohmann/json.hpp"

namespace audio_plugin {

PresetBrowserComponent::~PresetBrowserComponent() {
    presetList.setModel(nullptr);
}

void PresetBrowserComponent::listBoxItemClicked(int rowNumber, const juce::MouseEvent&) { (void)rowNumber; }
void PresetBrowserComponent::listBoxItemDoubleClicked(int rowNumber, const juce::MouseEvent&) { (void)rowNumber; }
void PresetBrowserComponent::changeListenerCallback(juce::ChangeBroadcaster*) {}

PresetBrowserComponent::PresetBrowserComponent(PresetManager& manager)
    : presetManager(manager) {
  addAndMakeVisible(presetList);
  presetList.setModel(this);
  presetList.setRowHeight(24);
  presetList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff2d2d30));
  presetList.setColour(juce::ListBox::textColourId, juce::Colours::white);
  presetList.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff3f3f41));

  // Load button
  addAndMakeVisible(loadButton);
  loadButton.onClick = [this] { loadSelectedPreset(); };
  loadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0e639c));
  loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

  // Save button
  addAndMakeVisible(saveButton);
  saveButton.onClick = [this] { savePreset(); };
  saveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0e9c5e));
  saveButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

  // Preset name editor
  addAndMakeVisible(presetNameEditor);
  presetNameEditor.setMultiLine(false);
  presetNameEditor.setReturnKeyStartsNewLine(false);
  presetNameEditor.setReadOnly(false);
  presetNameEditor.setScrollbarsShown(false);
  presetNameEditor.setCaretVisible(true);
  presetNameEditor.setPopupMenuEnabled(true);
  presetNameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff3f3f41));
  presetNameEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
  presetNameEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff5d5d5f));
  presetNameEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff0e639c));
  presetNameEditor.setColour(juce::TextEditor::highlightColourId, juce::Colour(0xff0e639c).withAlpha(0.4f));
  presetNameEditor.setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::white);

  // Category combo
  addAndMakeVisible(categoryCombo);
  categoryCombo.addItem("All", 1);
  categoryCombo.setSelectedId(1);
  categoryCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff3f3f41));
  categoryCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
  categoryCombo.setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
  categoryCombo.setColour(juce::ComboBox::buttonColourId, juce::Colour(0xff3f3f41));
  categoryCombo.setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff3f3f41));
  categoryCombo.setColour(juce::PopupMenu::textColourId, juce::Colours::white);
  categoryCombo.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff0e639c));

  // Set up preset directory and scan for presets
  scanPresetDirectory();
  
  // Set initial size
  setSize(300, 400);
}

void PresetBrowserComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colour(0xff252526));
  
  // Draw header
  auto headerBounds = getLocalBounds().removeFromTop(30);
  g.setColour(juce::Colour(0xff2d2d30));
  g.fillRect(headerBounds);
  
  g.setColour(juce::Colours::white);
  g.setFont(juce::Font(juce::FontOptions{}.withHeight(16.0f)).boldened());
  g.drawText("PRESETS", headerBounds.reduced(8, 0), juce::Justification::left, false);
}

void PresetBrowserComponent::resized() {
  auto bounds = getLocalBounds();
  
  // Header is already handled in paint
  bounds.removeFromTop(30);
  
  // Bottom controls
  auto bottomSection = bounds.removeFromBottom(40).reduced(4);
  loadButton.setBounds(bottomSection.removeFromLeft(80).reduced(2));
  saveButton.setBounds(bottomSection.removeFromLeft(80).reduced(2));
  
  // Top controls
  auto topSection = bounds.removeFromTop(40).reduced(4);
  categoryCombo.setBounds(topSection.removeFromLeft(100).reduced(2));
  presetNameEditor.setBounds(topSection.removeFromLeft(150).reduced(2));
  
  // Preset list takes the rest
  presetList.setBounds(bounds.reduced(2));
}

int PresetBrowserComponent::getNumRows() {
  return static_cast<int>(presets.size());
}

void PresetBrowserComponent::paintListBoxItem(int rowNumber,
                                             juce::Graphics& g,
                                             int width,
                                             int height,
                                             bool rowIsSelected) {
  if (rowIsSelected) {
    g.setColour(juce::Colour(0xff0e639c));
    g.fillRect(0, 0, width, height);
    g.setColour(juce::Colours::white);
  } else if (rowNumber % 2) {
    g.setColour(juce::Colour(0x10ffffff));
    g.fillRect(0, 0, width, height);
    g.setColour(juce::Colours::white);
  } else {
    g.setColour(juce::Colours::white.withAlpha(0.9f));
  }

  if (rowNumber < static_cast<int>(presets.size())) {
    auto text = presets[static_cast<size_t>(rowNumber)].file.getFileNameWithoutExtension();
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(14.0f)));
    g.drawText(text, 10, 0, width - 10, height, juce::Justification::centredLeft, true);
  }
}

void PresetBrowserComponent::scanPresetDirectory() {
  presets.clear();
  
  // Get all preset files
  auto presetFiles = presetManager.getPresetDirectory().findChildFiles(
      juce::File::findFiles, false, "*.xml");
  
  // Sort by modification time (newest first)
  std::vector<juce::File> files;
  for (const auto& file : presetFiles) {
    files.push_back(file);
  }
  
  std::sort(files.begin(), files.end(),
           [](const juce::File& a, const juce::File& b) {
             return a.getLastModificationTime() > b.getLastModificationTime();
           });
  
  // Add to presets list
  for (const auto& file : files) {
    presets.push_back({file, "Default"});
  }
  
  // Update the list
  presetList.updateContent();
  
  // Select the first preset if available
  if (!presets.empty()) {
    presetList.selectRow(0);
  }
}

void PresetBrowserComponent::loadSelectedPreset() {
  auto selectedRow = presetList.getSelectedRow();
  if (selectedRow >= 0 && selectedRow < static_cast<int>(presets.size())) {
    auto presetName = presets[static_cast<size_t>(selectedRow)].file.getFileNameWithoutExtension();
    if (presetManager.loadPreset(presetName)) {
      // Update the preset name editor
      presetNameEditor.setText(presetName, false);
      
      // Show a brief highlight on the selected row
      presetList.repaintRow(selectedRow);
      juce::Timer::callAfterDelay(200, [this, selectedRow] {
        presetList.repaintRow(selectedRow);
      });
    }
  }
}

void PresetBrowserComponent::savePreset() {
  auto presetName = presetNameEditor.getText().trim();
  if (presetName.isNotEmpty()) {
    if (presetManager.savePreset(presetName)) {
      // Rescan to update the list with the new preset
      scanPresetDirectory();
      
      // Select the newly saved preset
      for (size_t i = 0; i < presets.size(); ++i) {
        if (presets[i].file.getFileNameWithoutExtension() == presetName) {
          presetList.selectRow(static_cast<int>(i));
          break;
        }
      }
    }
  }
}

}  // namespace audio_plugin
