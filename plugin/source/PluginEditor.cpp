#include "Pointilsynth/PluginEditor.h"
#include "Pointilsynth/PluginProcessor.h"  // Ensure this path is correct based on include directories
#include "Pointilsynth/ConfigManager.h"

namespace audio_plugin {

PointillisticSynthAudioProcessorEditor::PointillisticSynthAudioProcessorEditor(
    audio_plugin::AudioPluginAudioProcessor& p,
    juce::AbstractFifo& fifo,
    GrainInfoForVis* buffer)
    : juce::AudioProcessorEditor(&p),
      processorRef(p),
      debugUIPanel(processorRef.getConfigManager()),
      pitchPod(ConfigManager::ParamID::pitch, "Pitch"),
      densityPod(ConfigManager::ParamID::density, "Density"),
      durationPod(ConfigManager::ParamID::avgDuration, "Duration"),
      panPod(ConfigManager::ParamID::pan, "Pan"),
      visualizationComponent(fifo, buffer) {
  addAndMakeVisible(visualizationComponent);
  addAndMakeVisible(debugUIPanel);
  addAndMakeVisible(pitchPod);
  addAndMakeVisible(densityPod);
  addAndMakeVisible(durationPod);
  addAndMakeVisible(panPod);
  addAndMakeVisible(openDebugButton);
  openDebugButton.onClick = [this] { openDebugWindow(); };

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
  auto buttonArea = podArea.removeFromBottom(30);
  openDebugButton.setBounds(buttonArea.reduced(4));
  auto podWidth = podArea.getWidth() / 4;

  pitchPod.setBounds(podArea.removeFromLeft(podWidth));
  densityPod.setBounds(podArea.removeFromLeft(podWidth));
  durationPod.setBounds(podArea.removeFromLeft(podWidth));
  panPod.setBounds(podArea);
}

void PointillisticSynthAudioProcessorEditor::openDebugWindow() {
  if (debugWindow)
    return;
  debugWindow = std::make_unique<DebugUIWindow>(
      processorRef.getConfigManager(), [this] { debugWindow.reset(); });
  debugWindow->setVisible(true);
}

}  // namespace audio_plugin
