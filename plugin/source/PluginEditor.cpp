#include "Pointilsynth/PluginEditor.h"
#include "Pointilsynth/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), debugUIPanel(processorRef.getStochasticModel()) { // Initialize debugUIPanel
  juce::ignoreUnused(processorRef);

  addAndMakeVisible(debugUIPanel); // Add and make panel visible

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(600, 400); // Set size for DebugUIPanel
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  // g.setColour(juce::Colours::white); // Hello World text removed
  // g.setFont(15.0f);
  // g.drawFittedText("Hello World!", getLocalBounds(),
  //                  juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  debugUIPanel.setBounds(getLocalBounds()); // Make DebugUIPanel fill the editor
}
}  // namespace audio_plugin
