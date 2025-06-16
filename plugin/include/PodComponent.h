#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace audio_plugin {

class PodComponent : public juce::Component {
public:
  PodComponent(juce::AudioProcessorValueTreeState& apvts,
               juce::String paramID,
               juce::String displayName);
  ~PodComponent() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  juce::Slider slider;
  juce::Label label;

  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  std::unique_ptr<SliderAttachment> attachment;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PodComponent)
};

}  // namespace audio_plugin
