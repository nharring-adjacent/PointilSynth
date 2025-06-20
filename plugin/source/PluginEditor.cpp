#include "Pointilsynth/PluginEditor.h"
#include "Pointilsynth/PluginProcessor.h"  // Ensure this path is correct based on include directories
#include "Pointilsynth/ConfigManager.h"
#include "Pointilsynth/DebugWindow.h"

namespace audio_plugin {

PointillisticSynthAudioProcessorEditor::PointillisticSynthAudioProcessorEditor(
    audio_plugin::AudioPluginAudioProcessor& p,
    juce::AbstractFifo& fifo,
    GrainInfoForVis* buffer)
    : juce::AudioProcessorEditor(&p),
      processorRef(p),
      pitchPod(ConfigManager::ParamID::pitch, "Pitch"),
      densityPod(ConfigManager::ParamID::density, "Density"),
      durationPod(ConfigManager::ParamID::avgDuration, "Duration"),
      panPod(ConfigManager::ParamID::pan, "Pan"),
      visualizationComponent(fifo, buffer) {
  addAndMakeVisible(visualizationComponent);
  addAndMakeVisible(debugButton);
  addAndMakeVisible(pitchPod);
  addAndMakeVisible(densityPod);
  addAndMakeVisible(durationPod);
  addAndMakeVisible(panPod);

  debugButton.onClick = [this] {
    if (!debugWindow)
      debugWindow =
          std::make_unique<DebugWindow>(processorRef.getConfigManager());
    debugWindow->setVisible(true);
    debugWindow->toFront(true);
  };

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

  auto buttonArea = area.removeFromTop(30);
  debugButton.setBounds(buttonArea.reduced(5));

  auto podArea = area;
  auto podWidth = podArea.getWidth() / 4;

  pitchPod.setBounds(podArea.removeFromLeft(podWidth));
  densityPod.setBounds(podArea.removeFromLeft(podWidth));
  durationPod.setBounds(podArea.removeFromLeft(podWidth));
  panPod.setBounds(podArea);
}

}  // namespace audio_plugin
