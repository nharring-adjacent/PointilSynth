#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>

namespace audio_plugin {

class VisualizationComponent : public juce::Component, public juce::Timer {
public:
  VisualizationComponent();
  ~VisualizationComponent() override;

  void paint(juce::Graphics& g) override;
  void timerCallback() override;

private:
  struct MockGrain {
    float pan{};    // -1.0 to 1.0
    float pitch{};  // 21.0 to 108.0
    float size{};   // pixel radius
    juce::Colour colour;
    double startTime{};  // seconds
    double maxAge{};     // seconds
  };

  juce::OpenGLContext openGLContext;
  std::vector<MockGrain> grains;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizationComponent)
};

}  // namespace audio_plugin
