#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

class PodComponent : public juce::Component {
public:
  PodComponent() = default;
  ~PodComponent() override = default;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PodComponent)
};

}  // namespace audio_plugin
