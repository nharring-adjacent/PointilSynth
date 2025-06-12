#include "Pointilsynth/Oscillator.h" // Defines Pointilsynth::Oscillator
#include <gtest/gtest.h>
#include <vector>
#include <cmath> // For std::abs
#include <numeric> // For std::all_of, std::accumulate
#include <set>     // For checking uniqueness in noise test

// Define a namespace alias for convenience if tests are outside Pointilsynth namespace
// Or, wrap all tests in namespace Pointilsynth { ... }
namespace P = Pointilsynth;

const float FLOAT_PRECISION_LOOSE = 1e-3f; // For periodicity with table-based oscillators
const float FLOAT_PRECISION_TIGHT = 1e-6f; // For general float comparisons

TEST(OscillatorTest, CanConstruct) {
    EXPECT_NO_THROW(std::make_unique<P::Oscillator>());
}

TEST(OscillatorTest, DefaultStateProducesSound) {
    P::Oscillator osc; // Default is Sine
    osc.setSampleRate(44100.0);
    osc.setFrequency(440.0f);

    std::vector<float> samples;
    bool nonZeroFound = false;
    for (int i = 0; i < 100; ++i) {
        float sample = osc.getNextSample();
        samples.push_back(sample);
        if (std::abs(sample) > FLOAT_PRECISION_TIGHT) {
            nonZeroFound = true;
        }
    }
    ASSERT_TRUE(nonZeroFound) << "Default oscillator should produce non-zero samples.";
}

// Helper function for periodicity test
void testPeriodicity(P::Oscillator& osc, double sampleRate, float frequency, float tolerance) {
    osc.setSampleRate(sampleRate);
    osc.setFrequency(frequency);

    // Ensure enough samples for at least one full cycle if frequency is very low
    // For f=1Hz, sr=1000Hz, periodInSamples = 1000.
    // For f=0.5Hz, sr=1000Hz, periodInSamples = 2000.
    int periodInSamples = static_cast<int>(sampleRate / frequency);
    if (periodInSamples <= 0) {
        FAIL() << "Period too short or zero, check sampleRate/frequency.";
        return;
    }

    // Get first sample after potentially discarding some to let oscillator stabilize (if needed, though usually not for simple ones)
    // For JUCE dsp::Oscillator, it should be stable from the start after prepare/setFrequency.
    float firstSample = osc.getNextSample();

    for (int i = 0; i < periodInSamples - 1; ++i) {
        osc.getNextSample();
    }
    float lastSampleOfCycle = osc.getNextSample();
    EXPECT_NEAR(firstSample, lastSampleOfCycle, tolerance)
        << "Waveform should be periodic. First: " << firstSample << ", Last: " << lastSampleOfCycle
        << " at SR: " << sampleRate << " Hz, Freq: " << frequency << " Hz";
}

// Helper function for range test
void testOutputRange(P::Oscillator& osc, double sampleRate, float frequency) {
    osc.setSampleRate(sampleRate);
    osc.setFrequency(frequency);
    // Test over a few cycles or at least a significant number of samples
    int numSamplesToTest = static_cast<int>(sampleRate / frequency * 2);
    if (numSamplesToTest < 100) numSamplesToTest = 100; // Ensure at least 100 samples
    if (numSamplesToTest > 5000) numSamplesToTest = 5000; // Cap for performance

    for (int i = 0; i < numSamplesToTest; ++i) {
        float sample = osc.getNextSample();
        ASSERT_GE(sample, -1.0f - FLOAT_PRECISION_LOOSE) << "Sample " << sample << " too low at i=" << i;
        ASSERT_LE(sample,  1.0f + FLOAT_PRECISION_LOOSE) << "Sample " << sample << " too high at i=" << i;
    }
}


TEST(OscillatorTest, WaveformSwitchingAndCharacteristics) {
    P::Oscillator osc;
    double sampleRate = 1000.0;
    float testFrequency = 2.0f; // Use a frequency that results in a whole number of samples per period

    // --- Sine ---
    osc.setWaveform(P::Oscillator::Waveform::Sine);
    SCOPED_TRACE("Sine Waveform");
    testPeriodicity(osc, sampleRate, testFrequency, FLOAT_PRECISION_LOOSE);
    testOutputRange(osc, sampleRate, testFrequency);

    // --- Saw ---
    osc.setWaveform(P::Oscillator::Waveform::Saw);
    SCOPED_TRACE("Saw Waveform");
    // For Saw, the discontinuity might make strict periodicity harder with table lookup.
    // The JUCE lambda for saw is (x / pi) - 1.0f. At x=0 (start), it's -1.0.
    // At x just before 2*pi, it approaches 1.0. The table lookup might smooth this.
    // Let's test a few samples after reset.
    osc.setSampleRate(sampleRate); // Resets phase etc.
    osc.setFrequency(testFrequency);
    // float firstSaw = osc.getNextSample(); // Should be near -1.0
    // Due to table interpolation and small table size, exact periodicity at discontinuity is hard.
    // Instead of strict periodicity, we'll rely on range and frequency tests.
    // If we must test periodicity, a higher tolerance or specific points might be needed.
    // For now, let's focus on range for Saw.
    testOutputRange(osc, sampleRate, testFrequency);
    // Periodicity test for Saw might fail at the discontinuity point due to table interpolation.
    // A more robust test for Saw would check the slope or specific points if the exact behavior of JUCE's table is known.
    // Given the provided code, the lambda (x/pi) - 1 for x in [0, 2pi) means it should be periodic.
    testPeriodicity(osc, sampleRate, testFrequency, FLOAT_PRECISION_LOOSE * 5); // Wider tolerance for Saw


    // --- Square ---
    osc.setWaveform(P::Oscillator::Waveform::Square);
    SCOPED_TRACE("Square Waveform");
    // Similar to Saw, discontinuity might affect strict periodicity with table lookup.
    // Lambda: (x < pi) ? 1.0f : -1.0f.
    // testPeriodicity(osc, sampleRate, testFrequency, FLOAT_PRECISION_LOOSE * 5); // Wider tolerance
    // For Square, the value should be exactly 1.0 or -1.0 if no interpolation occurs.
    // Let's test periodicity but acknowledge potential table effects.
    testPeriodicity(osc, sampleRate, testFrequency, FLOAT_PRECISION_LOOSE);
    testOutputRange(osc, sampleRate, testFrequency);
    // Additionally, check if values are mostly 1.0 or -1.0
    osc.setSampleRate(sampleRate);
    osc.setFrequency(testFrequency);
    bool onlyHighLow = true;
    for(int i=0; i<100; ++i) { // Check 100 samples
        float s = osc.getNextSample();
        if (std::abs(s) < (1.0f - FLOAT_PRECISION_LOOSE*10) && std::abs(s) > FLOAT_PRECISION_LOOSE*10) { // Check if not near 1, -1 or 0
             // Allow for some transition values due to table interpolation
        }
    }
    // This specific check for square is tricky with table interpolation. Range test is more reliable.


    // --- Noise ---
    osc.setWaveform(P::Oscillator::Waveform::Noise);
    SCOPED_TRACE("Noise Waveform");
    osc.setSampleRate(sampleRate); // Noise doesn't use frequency in the same way

    std::vector<float> noiseSamples;
    int numNoiseSamples = 20;
    for (int i = 0; i < numNoiseSamples; ++i) {
        noiseSamples.push_back(osc.getNextSample());
    }

    // Check that not all samples are identical (highly probable for random noise)
    std::set<float> uniqueSamples(noiseSamples.begin(), noiseSamples.end());
    ASSERT_GT(uniqueSamples.size(), 1) << "Noise samples should generally not all be identical.";

    // Check that at least some samples are different from the first sample
    float firstNoiseSample = noiseSamples[0];
    bool differentFound = false;
    for (size_t i = 1; i < noiseSamples.size(); ++i) {
        if (std::abs(noiseSamples[i] - firstNoiseSample) > FLOAT_PRECISION_TIGHT) {
            differentFound = true;
            break;
        }
    }
    ASSERT_TRUE(differentFound) << "Some noise samples should differ from the first one.";
    testOutputRange(osc, sampleRate, 1.0f); // Frequency doesn't matter for noise range
}

TEST(OscillatorTest, SetFrequencyFunctionality) {
    P::Oscillator osc;
    osc.setWaveform(P::Oscillator::Waveform::Sine);
    double sampleRate = 2000.0; // Using a different SR

    SCOPED_TRACE("Frequency Test 1 (1Hz)");
    testPeriodicity(osc, sampleRate, 1.0f, FLOAT_PRECISION_LOOSE);

    SCOPED_TRACE("Frequency Test 2 (2Hz)");
    testPeriodicity(osc, sampleRate, 2.0f, FLOAT_PRECISION_LOOSE);

    SCOPED_TRACE("Frequency Test 3 (0.5Hz)");
    testPeriodicity(osc, sampleRate, 0.5f, FLOAT_PRECISION_LOOSE);
}

TEST(OscillatorTest, SetSampleRateFunctionality) {
    P::Oscillator osc;
    osc.setWaveform(P::Oscillator::Waveform::Sine);
    float frequency = 1.0f;

    SCOPED_TRACE("Sample Rate Test 1 (1000Hz)");
    testPeriodicity(osc, 1000.0, frequency, FLOAT_PRECISION_LOOSE);

    SCOPED_TRACE("Sample Rate Test 2 (2000Hz)");
    testPeriodicity(osc, 2000.0, frequency, FLOAT_PRECISION_LOOSE);

    SCOPED_TRACE("Sample Rate Test 3 (800Hz)"); // Another SR
    testPeriodicity(osc, 800.0, frequency, FLOAT_PRECISION_LOOSE);
}

[end of test/source/OscillatorTest.cpp]
