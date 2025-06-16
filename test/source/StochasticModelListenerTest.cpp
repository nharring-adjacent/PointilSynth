#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/PluginProcessor.h"
#include "Pointilsynth/ConfigManager.h"
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

TEST(StochasticModelListenerTest, UpdatesOnParameterChange) {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  audio_plugin::AudioPluginAudioProcessor processor;
  auto cfg = ConfigManager::getInstance(&processor);
  StochasticModel model(cfg);

  auto& apvts = cfg->getAPVTS();
  if (auto* param = apvts.getParameter(ConfigManager::ParamID::pitch)) {
    param->setValueNotifyingHost(param->convertTo0to1(80.0f));
  }

  EXPECT_FLOAT_EQ(model.getPitch(), 80.0f);
}
