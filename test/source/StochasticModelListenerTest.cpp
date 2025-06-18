#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/PluginProcessor.h"
#include "Pointilsynth/ConfigManager.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

TEST_CASE("UpdatesOnParameterChange", "[StochasticModelListenerTest]") {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  audio_plugin::AudioPluginAudioProcessor processor;
  auto cfg = ConfigManager::getInstance(&processor);
  StochasticModel model(cfg);

  auto& apvts = cfg->getAPVTS();
  if (auto* param = apvts.getParameter(ConfigManager::ParamID::pitch)) {
    param->setValueNotifyingHost(param->convertTo0to1(80.0f));
  }

  REQUIRE(juce::approximatelyEqual(model.getPitch(), 80.0f));
}
