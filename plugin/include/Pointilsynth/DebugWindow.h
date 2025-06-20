#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DebugUIPanel.h"
#include "ConfigManager.h"

class DebugWindow : public juce::DocumentWindow {
public:
  explicit DebugWindow(std::shared_ptr<ConfigManager> cfg);
  ~DebugWindow() override = default;

  void closeButtonPressed() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugWindow)
};
