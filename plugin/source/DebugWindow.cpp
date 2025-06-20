#include "Pointilsynth/DebugWindow.h"

DebugWindow::DebugWindow(std::shared_ptr<ConfigManager> cfg)
    : juce::DocumentWindow("Debug",
                           juce::Colours::lightgrey,
                           juce::DocumentWindow::allButtons) {
  setUsingNativeTitleBar(true);
  setResizable(true, true);
  setAlwaysOnTop(true);
  setContentOwned(new DebugUIPanel(std::move(cfg)), true);
  centreWithSize(400, 300);
}

void DebugWindow::closeButtonPressed() {
  setVisible(false);
}
