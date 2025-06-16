#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include "Pointilsynth/PointilismInterfaces.h"

namespace audio_plugin {

class VisualizationComponent : public juce::Component, public juce::Timer {
public:
  VisualizationComponent(juce::AbstractFifo& fifo, GrainInfoForVis* buffer);
  ~VisualizationComponent() override;

  void paint(juce::Graphics& g) override;
  void timerCallback() override;

private:
  struct VisualGrain {
    float pan{};    // -1.0 to 1.0
    float pitch{};  // 21.0 to 108.0
    float size{};   // pixel radius
    juce::Colour colour;
    double startTime{};  // seconds
    double maxAge{};     // seconds
  };

  juce::OpenGLContext openGLContext;
  std::vector<VisualGrain> grains;
  juce::AbstractFifo& fifo_;
  GrainInfoForVis* buffer_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizationComponent)
};

}  // namespace audio_plugin
