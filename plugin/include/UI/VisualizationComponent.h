#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <vector>

namespace audio_plugin {

class VisualizationComponent : public juce::Component, public juce::Timer {
public:
  VisualizationComponent();
  ~VisualizationComponent() override;

  void paint(juce::Graphics& g) override;
  void timerCallback() override;

private:
  struct MockGrain {
    float pan = 0.0f;
    float pitch = 60.0f;
    float size = 4.0f;
    juce::Colour colour;
    double startTime = 0.0;
    double maxAge = 1.0;
  };

  juce::OpenGLContext openGLContext_;
  std::vector<MockGrain> grains_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizationComponent)
};

}  // namespace audio_plugin
