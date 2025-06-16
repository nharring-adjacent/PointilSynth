#include "PodComponent.h"
#include "Pointilsynth/ConfigManager.h"

namespace audio_plugin {

PodComponent::PodComponent(const juce::String& paramID,
                           const juce::String& displayName)
    : displayName_(displayName) {
  slider_ = ConfigManager::getInstance()->createAttachedSlider(paramID);
  addAndMakeVisible(*slider_);
  label_.setText(displayName_, juce::dontSendNotification);
  label_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(label_);
}

PodComponent::~PodComponent() {
  if (slider_)
    ConfigManager::getInstance()->releaseAttachment(slider_.get());
}

void PodComponent::resized() {
  auto area = getLocalBounds();
  auto labelArea = area.removeFromBottom(20);
  if (slider_)
    slider_->setBounds(area);
  label_.setBounds(labelArea);
}

}  // namespace audio_plugin
