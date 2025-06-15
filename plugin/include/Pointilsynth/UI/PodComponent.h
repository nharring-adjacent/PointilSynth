#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

class PodComponent : public juce::Component {
public:
  PodComponent() = default;
  ~PodComponent() override = default;

  void setName(const juce::String& newName);
  void setValue(float newValue,
                juce::NotificationType notification = juce::sendNotification);

  const juce::String& getName() const noexcept { return name; }
  float getValue() const noexcept { return value; }

  void paint(juce::Graphics& g) override;
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;

private:
  juce::String name;
  float value{0.0f};

  float dragStartValue{0.0f};
  int startY{0};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PodComponent)
};

}  // namespace audio_plugin
