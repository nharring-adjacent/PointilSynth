#include "PodComponent.h"  // Defines audio_plugin::PodComponent
#include "Pointilsynth/ConfigManager.h"
#include "Pointilsynth/PluginProcessor.h"
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>  // For ScopedJuceInitialiser_GUI

namespace audio_plugin {
// Minimal ScopedJuceInitialiser_GUI for tests needing it.
struct PodComponentTestFixture : public ::testing::Test {
  PodComponentTestFixture() = default;
  ~PodComponentTestFixture() override { ConfigManager::resetInstance(); }

  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  AudioPluginAudioProcessor processor;
};

TEST_F(PodComponentTestFixture, PodComponentCanConstruct) {
  auto& apvts = ConfigManager::getInstance(&processor)->getAPVTS();
  EXPECT_NO_THROW(std::make_unique<PodComponent>(
      apvts, ConfigManager::ParamID::pitch, "Pitch"));
}
}  // namespace audio_plugin
