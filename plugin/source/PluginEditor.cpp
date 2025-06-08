#include "Pointilsynth/PluginEditor.h"
#include "Pointilsynth/PluginProcessor.h" // Ensure this path is correct based on include directories

namespace audio_plugin {

PointillisticSynthAudioProcessorEditor::PointillisticSynthAudioProcessorEditor(
    audio_plugin::AudioPluginAudioProcessor& p)
    : juce::AudioProcessorEditor(&p), processorRef(p) {
    // Add and make visible the PodComponents
    addAndMakeVisible(pitchPod);
    addAndMakeVisible(densityPod);
    addAndMakeVisible(durationPod);
    addAndMakeVisible(panPod);

    // Set the size of the editor window.
    setSize(600, 400); // Example size, can be adjusted

}

PointillisticSynthAudioProcessorEditor::~PointillisticSynthAudioProcessorEditor() {}

void PointillisticSynthAudioProcessorEditor::paint(juce::Graphics& g) {
    // Fill the background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // You can add drawing code here for the visualization area if needed,
    // or leave it blank if it's handled by another component.
    // For now, let's draw a placeholder for the visualization area.
    auto visArea = getLocalBounds().removeFromTop(getHeight() * 2 / 3);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(visArea);
    g.setColour(juce::Colours::white);
    g.drawText("Visualization Area", visArea, juce::Justification::centred, false);
}

void PointillisticSynthAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    // Top two-thirds for visualization (currently empty or placeholder)
    bounds.removeFromTop(getHeight() * 2 / 3);

    // Bottom third for the pods
    auto podArea = bounds;
    int podWidth = podArea.getWidth() / 4;

    pitchPod.setBounds(podArea.removeFromLeft(podWidth));
    densityPod.setBounds(podArea.removeFromLeft(podWidth));
    durationPod.setBounds(podArea.removeFromLeft(podWidth));
    panPod.setBounds(podArea.removeFromLeft(podWidth));

}

} // namespace audio_plugin
