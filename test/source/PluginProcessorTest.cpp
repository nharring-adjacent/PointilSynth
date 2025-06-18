#include "Pointilsynth/PluginProcessor.h"  // Defines audio_plugin::AudioPluginAudioProcessor
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>  // For ScopedJuceInitialiser_GUI, as PluginProcessor creates an Editor

namespace audio_plugin {  // Match namespace of class under test for consistency

// Test fixture for PluginProcessor tests if GUI resources are involved
// (e.g. if the processor's constructor or tested methods use MessageManager or
// create UI elements) PluginProcessor::createEditor() implies GUI context might
// be needed.
struct ProcessorTestFixture {
  ProcessorTestFixture() = default;
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_CASE_METHOD(ProcessorTestFixture,
                 "CanConstructProcessor",
                 "[ProcessorTestFixture]") {
  REQUIRE_NOTHROW(std::make_unique<AudioPluginAudioProcessor>());
}

}  // namespace audio_plugin
