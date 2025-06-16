#include "Pointilsynth/PluginEditor.h"
#include "Pointilsynth/PluginProcessor.h"  // Ensure this path is correct based on include directories
#include "Pointilsynth/ConfigManager.h"

namespace audio_plugin {

PointillisticSynthAudioProcessorEditor::PointillisticSynthAudioProcessorEditor(
    audio_plugin::AudioPluginAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
      processorRef(p),
      debugUIPanel(processorRef.getConfigManager()),
      pitchPod(ConfigManager::ParamID::pitch, "Pitch"),
      densityPod(ConfigManager::ParamID::density, "Density"),
      durationPod(ConfigManager::ParamID::avgDuration, "Duration"),
      panPod(ConfigManager::ParamID::pan, "Pan") {
  addAndMakeVisible(visualizationComponent);
  addAndMakeVisible(debugUIPanel);
  addAndMakeVisible(pitchPod);
  addAndMakeVisible(densityPod);
  addAndMakeVisible(durationPod);
  addAndMakeVisible(panPod);

  setSize(600, 400);  // Example size, can be adjusted
}

PointillisticSynthAudioProcessorEditor::
    ~PointillisticSynthAudioProcessorEditor() {}

void PointillisticSynthAudioProcessorEditor::paint(juce::Graphics& g) {
  // Fill the background
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PointillisticSynthAudioProcessorEditor::resized() {
  auto area = getLocalBounds();

  auto visArea = area.removeFromTop(getHeight() / 3);
  visualizationComponent.setBounds(visArea);

  auto debugArea = area.removeFromTop(getHeight() / 3);
  debugUIPanel.setBounds(debugArea);

  auto podArea = area;
  auto podWidth = podArea.getWidth() / 4;

  pitchPod.setBounds(podArea.removeFromLeft(podWidth));
  densityPod.setBounds(podArea.removeFromLeft(podWidth));
  durationPod.setBounds(podArea.removeFromLeft(podWidth));
  panPod.setBounds(podArea);
}

}  // namespace audio_plugin
