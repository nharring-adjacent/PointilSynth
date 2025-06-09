#include "Pointilsynth/Resampler.h" // Adjust if necessary
#include "juce_audio_basics/juce_audio_basics.h"
#include "gtest/gtest.h"
#include <cmath> // For M_PI, sin, cos, etc.
#include <vector>

// Helper for floating point comparisons
const double DOUBLE_TOLERANCE = 1e-9; // Resampler uses doubles internally for some calcs
const float FLOAT_TOLERANCE = 1e-6f;

// Define M_PI if not already defined (e.g. by cmath or Resampler.h)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TEST(ResamplerTest, SincFunction) {
    EXPECT_NEAR(Resampler::sinc(0.0), 1.0, DOUBLE_TOLERANCE);
    EXPECT_NEAR(Resampler::sinc(1.0), std::sin(M_PI) / M_PI, DOUBLE_TOLERANCE); // sin(PI) is 0
    EXPECT_NEAR(Resampler::sinc(-1.0), std::sin(-M_PI) / -M_PI, DOUBLE_TOLERANCE); // sin(-PI) is 0
    EXPECT_NEAR(Resampler::sinc(0.5), std::sin(M_PI * 0.5) / (M_PI * 0.5), DOUBLE_TOLERANCE); // sin(PI/2)/(PI/2) = 1 / (PI/2) = 2/PI
    EXPECT_NEAR(Resampler::sinc(-0.5), std::sin(M_PI * -0.5) / (M_PI * -0.5), DOUBLE_TOLERANCE); // Should be same as sinc(0.5)
    EXPECT_NEAR(Resampler::sinc(1.5), std::sin(M_PI * 1.5) / (M_PI * 1.5), DOUBLE_TOLERANCE); // sin(3PI/2)/(3PI/2) = -1 / (3PI/2) = -2/(3PI)
}

TEST(ResamplerTest, BlackmanWindowFunction) {
    // Test at window center
    // normalized_pos = 0. cos(0)=1. a0 + a1 + a2
    const double alpha = 0.16;
    const double a0 = (1.0 - alpha) / 2.0;
    const double a1 = 0.5;
    const double a2 = alpha / 2.0;
    EXPECT_NEAR(Resampler::blackmanWindow(0.0), a0 + a1 + a2, DOUBLE_TOLERANCE);

    // Test at window edge (WINDOW_SIDE_POINTS)
    // normalized_pos = 1. cos(PI)=-1. cos(2PI)=1. a0 - a1 + a2
    EXPECT_NEAR(Resampler::blackmanWindow(static_cast<double>(Resampler::WINDOW_SIDE_POINTS)), a0 - a1 + a2, DOUBLE_TOLERANCE);
    EXPECT_NEAR(Resampler::blackmanWindow(static_cast<double>(-Resampler::WINDOW_SIDE_POINTS)), a0 - a1 + a2, DOUBLE_TOLERANCE);

    // Test outside window
    EXPECT_NEAR(Resampler::blackmanWindow(static_cast<double>(Resampler::WINDOW_SIDE_POINTS + 1)), 0.0, DOUBLE_TOLERANCE);
    EXPECT_NEAR(Resampler::blackmanWindow(static_cast<double>(-(Resampler::WINDOW_SIDE_POINTS + 1))), 0.0, DOUBLE_TOLERANCE);

    // Test at a fractional point within the window if WINDOW_SIDE_POINTS > 0
    if (Resampler::WINDOW_SIDE_POINTS > 0) {
        double test_dist = static_cast<double>(Resampler::WINDOW_SIDE_POINTS) / 2.0;
        double norm_pos = test_dist / static_cast<double>(Resampler::WINDOW_SIDE_POINTS); // Should be 0.5
        double expected_val = a0 + a1 * std::cos(M_PI * norm_pos) + a2 * std::cos(2 * M_PI * norm_pos);
        EXPECT_NEAR(Resampler::blackmanWindow(test_dist), expected_val, DOUBLE_TOLERANCE);
    }
}

TEST(ResamplerTest, GetSampleEmptyBuffer) {
    juce::AudioBuffer<float> buffer(0, 0); // 0 channels, 0 samples
    EXPECT_NEAR(Resampler::getSample(buffer, 0, 0.0), 0.0f, FLOAT_TOLERANCE);
}

TEST(ResamplerTest, GetSampleInvalidChannel) {
    juce::AudioBuffer<float> buffer(1, 100); // 1 channel, 100 samples
    EXPECT_NEAR(Resampler::getSample(buffer, 1, 0.0), 0.0f, FLOAT_TOLERANCE); // Channel 1 is invalid
    EXPECT_NEAR(Resampler::getSample(buffer, -1, 0.0), 0.0f, FLOAT_TOLERANCE); // Channel -1 is invalid
}

TEST(ResamplerTest, GetSampleAtIntegerPositionsIdentity) {
    const int numChannels = 1;
    const int numSamples = Resampler::WINDOW_SIDE_POINTS * 4; // Ensure enough samples around read pos
    juce::AudioBuffer<float> buffer(numChannels, numSamples);

    // Fill with an impulse in the middle
    int impulsePos = numSamples / 2;
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] = (i == impulsePos) ? 1.0f : 0.0f;
        }
    }

    // Reading at the exact impulse position.
    // The sum of sinc(k_offset) * blackmanWindow(k_offset) for k_offset = 0 should be 1*1=1.
    // For k_offset != 0, sinc(k_offset) is 0.
    // So, the result should be very close to the original sample value.
    float resampledValue = Resampler::getSample(buffer, 0, static_cast<double>(impulsePos));
    EXPECT_NEAR(resampledValue, 1.0f, FLOAT_TOLERANCE);

    // Reading at a zero position near the impulse
    // float resampledZero = Resampler::getSample(buffer, 0, static_cast<double>(impulsePos + 2));
    // This will not be exactly zero due to windowing, but should be small if window is symmetric.
    // The exact value depends on the sum of weighted sinc contributions from neighbors.
    // For this test, let's focus on the impulse itself. More detailed tests for interpolation follow.
}


TEST(ResamplerTest, GetSampleInterpolationStepFunction) {
    const int numChannels = 1;
    // Ensure enough samples for the window to operate without hitting buffer edges too much initially
    const int numSamples = Resampler::WINDOW_SIDE_POINTS * 4;
    juce::AudioBuffer<float> buffer(numChannels, numSamples);

    int stepPos = numSamples / 2;
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] = (i < stepPos) ? 0.0f : 1.0f;
        }
    }

    // Read exactly at stepPos - 0.5 (midpoint between 0 and 1)
    // The ideal sinc interpolation (no window) would give 0.5.
    // With windowing, it will be different. This test is more about behavior.
    double readPos = static_cast<double>(stepPos) - 0.5;
    float val = Resampler::getSample(buffer, 0, readPos);
    // We expect a value between 0 and 1.
    EXPECT_GT(val, -FLOAT_TOLERANCE); // Greater than 0 (allowing for slight undershoot)
    EXPECT_LT(val, 1.0f + FLOAT_TOLERANCE); // Less than 1 (allowing for slight overshoot)

    // A more rigorous test would require pre-calculating the expected windowed-sinc output.
    // For now, we check basic properties.

    // Read at stepPos - 1. Should be 0.0f if window doesn't reach far, or close to it.
    // The nearest sample is 0. Sinc(0)*Win(0) = 1. Sinc(-1)*Win(-1)=0.
    // So it should be close to sourceData[stepPos-1] = 0.0f
    readPos = static_cast<double>(stepPos - 1);
    val = Resampler::getSample(buffer, 0, readPos);
    EXPECT_NEAR(val, 0.0f, 0.1f); // Allow some deviation due to window

    // Read at stepPos. Should be close to sourceData[stepPos] = 1.0f
    readPos = static_cast<double>(stepPos);
    val = Resampler::getSample(buffer, 0, readPos);
    EXPECT_NEAR(val, 1.0f, 0.1f); // Allow some deviation
}


TEST(ResamplerTest, GetSampleBoundaryConditions) {
    const int numChannels = 1;
    const int numSamples = Resampler::WINDOW_SIDE_POINTS * 2; // Smaller buffer
    juce::AudioBuffer<float> buffer(numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] = static_cast<float>(i); // Simple ramp
        }
    }

    // Read near beginning (e.g., readPosition = 0.5)
    // k_center will be 0 or 1. Window will try to read samples < 0, which should be zero-padded.
    double readPosStart = 0.5;
    float valStart = Resampler::getSample(buffer, 0, readPosStart);
    // Exact value is hard to predict without manual calculation of windowed sinc sum.
    // Should be between buffer.getSample(0,0) and buffer.getSample(0,1) roughly,
    // but influenced by zero padding.
    // For this test, just ensure it doesn't crash and returns a number.
    EXPECT_TRUE(std::isfinite(valStart));


    // Read near end (e.g., readPosition = numSamples - 1.5)
    // k_center will be numSamples-2 or numSamples-1. Window will try to read samples >= numSamples.
    double readPosEnd = static_cast<double>(numSamples - 1) - 0.5; // e.g. if numSamples=32, read at 30.5
    float valEnd = Resampler::getSample(buffer, 0, readPosEnd);
    EXPECT_TRUE(std::isfinite(valEnd));

    // Read exactly at the first sample
    float valFirstExact = Resampler::getSample(buffer, 0, 0.0);
    // Will be influenced by zero-padding on the left of the window.
    // Should be closer to buffer.getSample(0,0) than not.
    EXPECT_NEAR(valFirstExact, buffer.getSample(0,0), 0.5f); // Loose tolerance

    // Read exactly at the last sample
    float valLastExact = Resampler::getSample(buffer, 0, static_cast<double>(numSamples - 1));
    EXPECT_NEAR(valLastExact, buffer.getSample(0,numSamples-1), 0.5f); // Loose tolerance
}

// More advanced: Test with a known sine wave and check phase/amplitude.
// This requires generating a reference windowed-sinc resampled signal, which is complex.
// For now, the step function and impulse tests cover basic numerical behavior.
