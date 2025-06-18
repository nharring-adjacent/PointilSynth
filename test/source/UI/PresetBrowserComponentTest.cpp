#include "UI/PresetBrowserComponent.h"
#include "Pointilsynth/PresetManager.h"
#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/PluginProcessor.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

struct PresetBrowserTestFixture {
  PresetBrowserTestFixture() = default;
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_CASE_METHOD(PresetBrowserTestFixture,
                 "CanConstruct",
                 "[PresetBrowserTestFixture]") {
  AudioPluginAudioProcessor processor;
  auto cfg = ConfigManager::getInstance(&processor);
  StochasticModel model(cfg);
  Pointilism::PresetManager manager(model);
  REQUIRE_NOTHROW([&] { PresetBrowserComponent browser(manager); }());
}

}  // namespace audio_plugin
