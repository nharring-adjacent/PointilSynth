#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DebugUIPanel.h"
#include "ConfigManager.h"
#include <functional>

class DebugUIWindow : public juce::DocumentWindow {
public:
  DebugUIWindow(std::shared_ptr<ConfigManager> cfg,
                std::function<void()> onClose);
  void closeButtonPressed() override;

private:
  DebugUIPanel debugUIPanel_;
  std::function<void()> onClose_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugUIWindow)
};
