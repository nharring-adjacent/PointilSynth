#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

class PodComponent : public juce::Component {
public:
  PodComponent(const juce::String& paramID, const juce::String& displayName);
  ~PodComponent() override;

  void resized() override;

private:
  std::unique_ptr<juce::Slider> slider_;
  juce::String displayName_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PodComponent)
};

}  // namespace audio_plugin
