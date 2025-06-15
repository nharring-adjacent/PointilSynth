#include "Pointilsynth/Resampler.h"  // Defines Resampler namespace
#include <gtest/gtest.h>
#include <juce_audio_basics/juce_audio_basics.h>  // For juce::AudioBuffer

TEST(ResamplerTest, CanIncludeAndCallSinc) {
  // Simple test to ensure compilation and basic call
  double val = 0.5;
  EXPECT_NO_THROW(Resampler::sinc(val));
  // A more robust test might check the value, but for now, no throw is enough.
}

TEST(ResamplerTest, GetSampleRuns) {
  juce::AudioBuffer<float> buffer(1, 100);  // 1 channel, 100 samples
  buffer.clear();                           // Fill with zeros
  // Test with some basic parameters
  EXPECT_NO_THROW(Resampler::getSample(buffer, 0, 10.5));
}
