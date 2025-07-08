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
  // Minimal stub processor to satisfy APVTS requirements
  struct DummyProcessor : juce::AudioProcessor {
    DummyProcessor() : juce::AudioProcessor(BusesProperties()) {}
    const juce::String getName() const override { return "Dummy"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    void reset() override {}
    juce::AudioProcessor* getWrappedProcessor() { return nullptr; }
  };
  DummyProcessor dummyProcessor;
  juce::AudioProcessorValueTreeState::ParameterLayout layout;
  juce::AudioProcessorValueTreeState apvts(dummyProcessor, nullptr, "params", std::move(layout));
  audio_plugin::PresetManager manager(apvts);
  REQUIRE_NOTHROW([&] { PresetBrowserComponent browser(manager); }());
}

}  // namespace audio_plugin
