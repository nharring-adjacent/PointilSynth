#pragma once

#include <deque>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "Pointilsynth/InertialHistoryManager.h"

namespace audio_plugin {

class InertialHistoryVisualizer : public juce::Component, private juce::Timer {
public:
  explicit InertialHistoryVisualizer(InertialHistoryManager& manager);
  ~InertialHistoryVisualizer() override = default;

  void paint(juce::Graphics& g) override;

private:
  void timerCallback() override;

  InertialHistoryManager& manager_;
  std::vector<std::deque<float>> histories_;

  static constexpr int maxPoints_ = 128;
};

}  // namespace audio_plugin
