#include "Pointilsynth/UI/PodComponent.h"

namespace audio_plugin {

void PodComponent::setName(const juce::String& newName) {
  if (name != newName) {
    name = newName;
    repaint();
  }
}

void PodComponent::setValue(float newValue, juce::NotificationType) {
  if (value != newValue) {
    value = newValue;
    repaint();
  }
}

void PodComponent::paint(juce::Graphics& g) {
  auto bounds = getLocalBounds().toFloat();

  auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight()) - 10.0f;
  auto radius = diameter / 2.0f;
  auto centre = bounds.getCentre();

  g.setColour(juce::Colours::darkgrey);
  g.fillEllipse(centre.x - radius, centre.y - radius, diameter, diameter);
  g.setColour(juce::Colours::white);
  g.drawEllipse(centre.x - radius, centre.y - radius, diameter, diameter, 2.0f);

  g.setColour(juce::Colours::white);
  g.setFont(14.0f);
  g.drawText(name, bounds.removeFromTop(20), juce::Justification::centred);

  g.setFont(12.0f);
  juce::String valueStr = juce::String(value, 2);
  g.drawText(valueStr, bounds.removeFromBottom(20),
             juce::Justification::centred);
}

void PodComponent::mouseDown(const juce::MouseEvent& event) {
  dragStartValue = value;
  startY = event.getPosition().getY();
}

void PodComponent::mouseDrag(const juce::MouseEvent& event) {
  int delta = startY - event.getPosition().getY();
  float newValue = dragStartValue + static_cast<float>(delta) * 0.01f;
  setValue(newValue, juce::sendNotification);
}

}  // namespace audio_plugin
