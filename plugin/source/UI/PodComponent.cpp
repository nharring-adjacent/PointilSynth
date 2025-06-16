#include "PodComponent.h"

namespace audio_plugin {

PodComponent::PodComponent(juce::AudioProcessorValueTreeState& apvts,
                           juce::String paramID,
                           juce::String displayName) {
  addAndMakeVisible(slider);
  slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

  addAndMakeVisible(label);
  label.setText(displayName, juce::dontSendNotification);
  label.setJustificationType(juce::Justification::centred);

  attachment =
      std::make_unique<PodComponent::SliderAttachment>(apvts, paramID, slider);
}

void PodComponent::paint(juce::Graphics& g) {
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
  g.setColour(juce::Colours::white);
  g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
}

void PodComponent::resized() {
  auto area = getLocalBounds();
  label.setBounds(area.removeFromBottom(20));
  slider.setBounds(area.reduced(10));
}

}  // namespace audio_plugin
