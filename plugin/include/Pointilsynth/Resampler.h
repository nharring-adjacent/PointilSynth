#ifndef RESAMPLER_H
#define RESAMPLER_H

// JUCE Header: Provides juce::AudioBuffer
// This line assumes that JUCE headers are available in the include paths of the consuming project.
#include <juce_audio_basics/juce_audio_basics.h>

#include <cmath> // For std::sin, std::cos, std::abs, std::round

// Define M_PI if not already defined (e.g., on Windows with MSVC, or if <cmath> doesn't provide it by default)
// C++17 and later prefer std::numbers::pi from <numbers>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Resampler {

    const int WINDOW_SIDE_POINTS = 16;

    inline double sinc(double x) {
        if (juce::approximatelyEqual(x, 0.0)) { // Or use std::abs(x) < epsilon for floating point comparison
            return 1.0;
        }
        double piX = M_PI * x;
        return std::sin(piX) / piX;
    }

    inline double blackmanWindow(double x_dist_from_center) {
        // x_dist_from_center is the distance from the window center.
        // It ranges from -WINDOW_SIDE_POINTS to +WINDOW_SIDE_POINTS for samples affecting the calculation.
        // Values outside this effectively have a window coefficient of 0.
        if (std::abs(x_dist_from_center) > static_cast<double>(WINDOW_SIDE_POINTS)) {
            return 0.0; // Outside the defined window range
        }

        const double alpha = 0.16; // Standard Blackman alpha
        const double a0 = (1.0 - alpha) / 2.0; // approx 0.42
        const double a1 = 0.5;
        const double a2 = alpha / 2.0;       // approx 0.08

        // Normalized position for the Blackman formula:
        // Our x_dist_from_center is already relative to the center.
        // We normalize it by WINDOW_SIDE_POINTS to range from -1 to 1 at the window edges.
        // If WINDOW_SIDE_POINTS is 0, this would divide by zero. Add a check or ensure WINDOW_SIDE_POINTS > 0.
        // Given it's a const int = 16, it's fine here.
        double normalized_pos = x_dist_from_center / static_cast<double>(WINDOW_SIDE_POINTS);

        // Blackman window formula: w(t) = a0 + a1*cos(pi*t) + a2*cos(2*pi*t) for t in [-1, 1]
        // This is a common form for a window symmetric around t=0.
        return a0 + a1 * std::cos(M_PI * normalized_pos) + a2 * std::cos(2 * M_PI * normalized_pos);
    }

    static float getSample(const juce::AudioBuffer<float>& sourceBuffer, int channel, double readPosition) {
        const int numSamples = sourceBuffer.getNumSamples();
        // If the source buffer is empty, no resampling can be done.
        if (numSamples == 0) {
            return 0.0f;
        }

        // Ensure the requested channel is valid.
        if (channel < 0 || channel >= sourceBuffer.getNumChannels()) {
            // Consider logging an error or returning a specific error code/exception.
            // For now, returning 0.0f for safety in audio context.
            // In a JUCE plugin, you might use JUCE_ASSERT_MESSAGE or DBG for debug builds.
            return 0.0f;
        }

        const float* sourceChannelData = sourceBuffer.getReadPointer(channel);
        // For a valid channel in a juce::AudioBuffer with numSamples > 0, getReadPointer should return a valid pointer.
        // However, a defensive check is not harmful.
        if (!sourceChannelData) {
            // This case should ideally not be reached if getNumChannels() is reliable and channel is valid.
            // Could indicate an issue with buffer state or an unexpected JUCE behavior.
            return 0.0f;
        }

        double outputSample = 0.0;
        // k_center is the nearest integer sample index in the source buffer to the fractional readPosition.
        int k_center = static_cast<int>(std::round(readPosition));

        // Iterate over source sample indices 'k' that are within the span of the window.
        for (int k_offset = -WINDOW_SIDE_POINTS; k_offset <= WINDOW_SIDE_POINTS; ++k_offset) {
            int k = k_center + k_offset; // Actual source sample index to attempt to read

            // Handle edge cases: Zero-padding behavior.
            if (k < 0 || k >= numSamples) {
                continue; // Effectively uses a zero sample for this 'k'.
            }

            // kernelArg is the distance from the desired fractional readPosition
            // to the current integer source sample index 'k'.
            double kernelArg = readPosition - k;

            double sincValue = sinc(kernelArg);
            double windowValue = blackmanWindow(kernelArg);
            double weightedSinc = sincValue * windowValue;

            outputSample += static_cast<double>(sourceChannelData[k]) * weightedSinc;
        }

        return static_cast<float>(outputSample);
    }

} // namespace Resampler

#endif // RESAMPLER_H
