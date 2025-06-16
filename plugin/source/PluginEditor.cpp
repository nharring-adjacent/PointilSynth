#include "Pointilsynth/PluginEditor.h"
#include "Pointilsynth/PluginProcessor.h"  // Ensure this path is correct based on include directories
#include "Pointilsynth/ConfigManager.h"

namespace audio_plugin {

PointillisticSynthAudioProcessorEditor::PointillisticSynthAudioProcessorEditor(
    audio_plugin::AudioPluginAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
      processorRef(p),
      debugUIPanel(processorRef.getConfigManager()) {
  addAndMakeVisible(pitchPod);
  addAndMakeVisible(densityPod);
  addAndMakeVisible(durationPod);
  addAndMakeVisible(panPod);
  addAndMakeVisible(visualization);

  setSize(600, 400);  // Example size, can be adjusted
}

PointillisticSynthAudioProcessorEditor::
    ~PointillisticSynthAudioProcessorEditor() {}

void PointillisticSynthAudioProcessorEditor::paint(juce::Graphics& g) {
  // Fill the background
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  // Visualization is handled by the VisualizationComponent.
}

void PointillisticSynthAudioProcessorEditor::resized() {
  auto bounds = getLocalBounds();

  auto visArea = bounds.removeFromTop(getHeight() * 2 / 3);
  visualization.setBounds(visArea);

  // Bottom third for the pods
  auto podArea = bounds;
  auto podWidth = podArea.getWidth() / 4;

  pitchPod.setBounds(podArea.removeFromLeft(podWidth));
  densityPod.setBounds(podArea.removeFromLeft(podWidth));
  durationPod.setBounds(podArea.removeFromLeft(podWidth));
  panPod.setBounds(podArea);
}

}  // namespace audio_plugin
