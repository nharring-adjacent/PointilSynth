#include "PodComponent.h"
#include "Pointilsynth/ConfigManager.h"

namespace audio_plugin {

PodComponent::PodComponent(const juce::String& paramID,
                           const juce::String& displayName)
    : displayName_(displayName) {
  slider_ = ConfigManager::getInstance()->createAttachedSlider(paramID);
  addAndMakeVisible(*slider_);
}

PodComponent::~PodComponent() {
  if (slider_)
    ConfigManager::getInstance()->releaseAttachment(slider_.get());
}

void PodComponent::resized() {
  if (slider_)
    slider_->setBounds(getLocalBounds());
}

}  // namespace audio_plugin
