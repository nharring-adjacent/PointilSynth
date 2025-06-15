#include "Pointilsynth/PluginEditor.h"  // Defines audio_plugin::PointillisticSynthAudioProcessorEditor
#include "Pointilsynth/PluginProcessor.h"  // For audio_plugin::AudioPluginAudioProcessor
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {
// Minimal ScopedJuceInitialiser_GUI for tests needing it.
struct JuceGuiTestFixture : public ::testing::Test {
  JuceGuiTestFixture() = default;
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

// Test fixture for PluginEditor that needs a PluginProcessor
class PluginEditorTest : public JuceGuiTestFixture {
protected:
  AudioPluginAudioProcessor processor;  // Create a processor instance
public:
  PluginEditorTest() = default;  // processor is default constructed
};

TEST_F(PluginEditorTest, CanConstruct) {
  EXPECT_NO_THROW(
      std::make_unique<PointillisticSynthAudioProcessorEditor>(processor));
}
}  // namespace audio_plugin
