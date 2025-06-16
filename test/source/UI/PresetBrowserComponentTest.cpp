#include "UI/PresetBrowserComponent.h"
#include "Pointilsynth/PresetManager.h"
#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/PluginProcessor.h"
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

struct PresetBrowserTestFixture : public ::testing::Test {
  PresetBrowserTestFixture() = default;
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_F(PresetBrowserTestFixture, CanConstruct) {
  AudioPluginAudioProcessor processor;
  auto cfg = ConfigManager::getInstance(&processor);
  StochasticModel model(cfg);
  Pointilism::PresetManager manager(model);
  EXPECT_NO_THROW(PresetBrowserComponent browser(manager));
}

}  // namespace audio_plugin
