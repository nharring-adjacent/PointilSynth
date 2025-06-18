#include "Pointilsynth/PluginEditor.h"  // Defines audio_plugin::PointillisticSynthAudioProcessorEditor
#include "Pointilsynth/PluginProcessor.h"  // For audio_plugin::AudioPluginAudioProcessor
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {
// Minimal ScopedJuceInitialiser_GUI for tests needing it.
struct JuceGuiTestFixture {
  JuceGuiTestFixture() = default;
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

// Test fixture for PluginEditor that needs a PluginProcessor
class PluginEditorTest : public JuceGuiTestFixture {
protected:
  AudioPluginAudioProcessor processor;  // Create a processor instance
  juce::AbstractFifo fifo{8};
  std::array<GrainInfoForVis, 8> buffer{};

public:
  PluginEditorTest() = default;  // processor is default constructed
};

TEST_CASE_METHOD(PluginEditorTest, "CanConstruct", "[PluginEditorTest]") {
  REQUIRE_NOTHROW(std::make_unique<PointillisticSynthAudioProcessorEditor>(
      processor, fifo, buffer.data()));
}
}  // namespace audio_plugin
