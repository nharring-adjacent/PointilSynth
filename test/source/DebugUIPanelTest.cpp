#include "Pointilsynth/DebugUIPanel.h"
#include "Pointilsynth/PluginProcessor.h"
#include "Pointilsynth/ConfigManager.h"
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

TEST(DebugUIPanelTest, CanConstruct) {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  audio_plugin::AudioPluginAudioProcessor processor;
  auto cfg = ConfigManager::getInstance(&processor);
  EXPECT_NO_THROW(std::make_unique<DebugUIPanel>(cfg));
}
