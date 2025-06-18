#include "Pointilsynth/DebugUIPanel.h"
#include "Pointilsynth/PluginProcessor.h"
#include "Pointilsynth/ConfigManager.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

TEST_CASE("CanConstruct", "[DebugUIPanelTest]") {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  audio_plugin::AudioPluginAudioProcessor processor;
  auto cfg = ConfigManager::getInstance(&processor);
  REQUIRE_NOTHROW(std::make_unique<DebugUIPanel>(cfg));
}
