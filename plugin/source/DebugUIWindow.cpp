#include "Pointilsynth/DebugUIWindow.h"

DebugUIWindow::DebugUIWindow(std::shared_ptr<ConfigManager> cfg,
                             std::function<void()> onClose)
    : DocumentWindow("Debug UI",
                     juce::Colours::darkgrey,
                     juce::DocumentWindow::allButtons),
      debugUIPanel_(std::move(cfg)),
      onClose_(std::move(onClose)) {
  setUsingNativeTitleBar(true);
  setResizable(true, true);
  setContentOwned(&debugUIPanel_, false);
  centreWithSize(600, 400);
}

void DebugUIWindow::closeButtonPressed() {
  if (onClose_)
    onClose_();
  setVisible(false);
}
