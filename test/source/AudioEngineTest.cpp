#include "Pointilsynth/PluginProcessor.h"
#include "melatonin_test_helpers/melatonin_test_helpers.h"
#include <catch2/catch_test_macros.hpp>

using namespace audio_plugin;
using namespace melatonin;

TEST_CASE("AudioEngine: Silence") {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  AudioPluginAudioProcessor processor;
  auto cfg = processor.getConfigManager();
  auto& apvts = cfg->getAPVTS();
  if (auto* param = apvts.getParameter(ConfigManager::ParamID::density)) {
    param->setValueNotifyingHost(param->convertTo0to1(0.0f));
  }
  AudioEngine engine(cfg);
  engine.prepareToPlay(44100.0, 512);
  juce::AudioBuffer<float> buffer(2, 512);
  juce::MidiBuffer midi;
  juce::AudioPlayHead::PositionInfo pos;
  engine.processBlock(buffer, midi, pos);
  REQUIRE_THAT(buffer, isEmpty());
}
