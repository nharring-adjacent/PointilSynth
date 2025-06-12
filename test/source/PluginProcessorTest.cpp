#include "Pointilsynth/PluginProcessor.h" // Defines audio_plugin::AudioPluginAudioProcessor
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h> // For ScopedJuceInitialiser_GUI, as PluginProcessor creates an Editor

namespace audio_plugin { // Match namespace of class under test for consistency

// Test fixture for PluginProcessor tests if GUI resources are involved
// (e.g. if the processor's constructor or tested methods use MessageManager or create UI elements)
// PluginProcessor::createEditor() implies GUI context might be needed.
struct ProcessorTestFixture : public ::testing::Test {
    ProcessorTestFixture() = default;
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_F(ProcessorTestFixture, CanConstructProcessor) { // Renamed test, using fixture
    EXPECT_NO_THROW(std::make_unique<AudioPluginAudioProcessor>());
}

} // namespace audio_plugin
