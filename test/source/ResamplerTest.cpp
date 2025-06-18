#include "Pointilsynth/Resampler.h"  // Defines Resampler namespace
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>  // For juce::AudioBuffer

TEST_CASE("CanIncludeAndCallSinc", "[ResamplerTest]") {
  // Simple test to ensure compilation and basic call
  double val = 0.5;
  REQUIRE_NOTHROW(Resampler::sinc(val));
  // A more robust test might check the value, but for now, no throw is enough.
}

TEST_CASE("GetSampleRuns", "[ResamplerTest]") {
  juce::AudioBuffer<float> buffer(1, 100);  // 1 channel, 100 samples
  buffer.clear();                           // Fill with zeros
  // Test with some basic parameters
  REQUIRE_NOTHROW(Resampler::getSample(buffer, 0, 10.5));
}
