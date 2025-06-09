#include "gtest/gtest.h" // Use Google Test
#include "plugin/source/PointilismInterfaces.h"
#include <limits>
#include <cmath> // For std::abs
#include <vector>
#include <numeric> // For std::accumulate
#include <algorithm> // For std::min_element, std::max_element

// Original TEST_CASE: StochasticModel Parameter Setters
// Each SECTION will become a new TEST within the StochasticModelTest suite.

TEST(StochasticModelTest, PitchAndDispersionSetter) {
    StochasticModel model;
    model.seedRandomEngine(12345);

    model.setPitchAndDispersion(60.0f, 5.0f);
    EXPECT_NEAR(model.getPitch(), 60.0f, 1e-5f);
    EXPECT_NEAR(model.getDispersion(), 5.0f, 1e-5f);

    Grain g1;
    model.generateNewGrain(g1);
    // float initialPitch = g1.pitch; // Keep if used for complex logic, otherwise can remove

    model.setPitchAndDispersion(60.0f, 0.0f);
    Grain g2;
    model.generateNewGrain(g2);
    EXPECT_NEAR(g2.pitch, 60.0f, 0.01f);

    model.setPitchAndDispersion(60.0f, 20.0f);
    Grain g3;
    model.generateNewGrain(g3);
    EXPECT_FALSE(std::abs(g3.pitch - 60.0f) < 0.1f);
}

TEST(StochasticModelTest, DurationAndVariationSetter) {
    StochasticModel model;
    model.seedRandomEngine(12345);
    model.setSampleRate(44100.0); // Explicitly set sample rate for consistent duration calcs

    model.setDurationAndVariation(500.0f, 0.1f);
    EXPECT_NEAR(model.getAverageDurationMs(), 500.0f, 1e-5f);
    EXPECT_NEAR(model.getDurationVariation(), 0.1f, 1e-5f);

    // Grain g1; // Initial grain - not used for assertions in this version
    // model.generateNewGrain(g1);
    // int initialDuration = g1.durationInSamples;

    model.setDurationAndVariation(500.0f, 0.0f);
    Grain g2;
    model.generateNewGrain(g2);
    float expectedDurationMs = 500.0f * (1.0f + 0.0f);
    int expectedDurationSamples = static_cast<int>((expectedDurationMs / 1000.0f) * model.getSampleRate());
    EXPECT_EQ(g2.durationInSamples, expectedDurationSamples);

    model.setDurationAndVariation(500.0f, 0.5f);
    Grain g3;
    model.generateNewGrain(g3);
    EXPECT_NE(g3.durationInSamples, expectedDurationSamples);
}

TEST(StochasticModelTest, PanAndSpreadSetter) {
    StochasticModel model;
    model.seedRandomEngine(12345);

    model.setPanAndSpread(0.5f, 0.2f);
    EXPECT_NEAR(model.getCentralPan(), 0.5f, 1e-5f);
    EXPECT_NEAR(model.getPanSpread(), 0.2f, 1e-5f);

    Grain g1;
    model.generateNewGrain(g1);
    EXPECT_GE(g1.pan, -1.0f);
    EXPECT_LE(g1.pan, 1.0f);
    // float initialPan = g1.pan;

    model.setPanAndSpread(0.0f, 0.0f);
    Grain g2;
    model.generateNewGrain(g2);
    EXPECT_NEAR(g2.pan, 0.0f, 0.01f);

    model.setPanAndSpread(0.0f, 0.8f);
    Grain g3;
    model.generateNewGrain(g3);
    EXPECT_FALSE(std::abs(g3.pan - 0.0f) < 0.1f);
}

TEST(StochasticModelTest, GlobalDensitySetter) {
    StochasticModel model;
    model.setGlobalDensity(20.0f);
    EXPECT_NEAR(model.getGlobalDensity(), 20.0f, 1e-5f);
}

TEST(StochasticModelTest, GlobalTemporalDistributionSetter) {
    StochasticModel model;
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    EXPECT_EQ(model.getGlobalTemporalDistribution(), StochasticModel::TemporalDistribution::Poisson);

    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Uniform);
    EXPECT_EQ(model.getGlobalTemporalDistribution(), StochasticModel::TemporalDistribution::Uniform);
}

TEST(StochasticModelTest, SampleRateSetter) {
    StochasticModel model;
    model.setSampleRate(48000.0);
    EXPECT_NEAR(model.getSampleRate(), 48000.0, 1e-5); // sampleRate is double

    model.setDurationAndVariation(1000.0f, 0.0f);
    int durationAt48k = static_cast<int>((1000.0f / 1000.0f) * 48000.0);
    Grain g1;
    model.generateNewGrain(g1);
    EXPECT_EQ(g1.durationInSamples, durationAt48k);

    model.setSampleRate(96000.0);
    EXPECT_NEAR(model.getSampleRate(), 96000.0, 1e-5);
    int durationAt96k = static_cast<int>((1000.0f / 1000.0f) * 96000.0);
    Grain g2;
    model.generateNewGrain(g2);
    EXPECT_EQ(g2.durationInSamples, durationAt96k);
}

// Helper functions for statistical calculations
namespace Stats {
    float calculateMean(const std::vector<float>& data) {
        if (data.empty()) return 0.0f;
        float sum = std::accumulate(data.begin(), data.end(), 0.0f);
        return sum / data.size();
    }

    float calculateStdDev(const std::vector<float>& data, float mean) {
        if (data.size() < 2) return 0.0f; // Stddev is undefined for less than 2 samples
        float sq_sum_diff = 0.0f;
        for (float val : data) {
            sq_sum_diff += (val - mean) * (val - mean);
        }
        return std::sqrt(sq_sum_diff / (data.size() - 1)); // Sample standard deviation
    }

    // Mean for integer data
    static double calculateIntMean(const std::vector<int>& data) {
        if (data.empty()) return 0.0;
        long long sum = std::accumulate(data.begin(), data.end(), 0LL); // Use long long for sum
        return static_cast<double>(sum) / data.size();
    }

    // Variance for integer data (Sample Variance)
    static double calculateIntVar(const std::vector<int>& data, double mean) {
        if (data.size() < 2) return 0.0; // Variance is undefined for less than 2 samples
        double sq_sum_diff = 0.0;
        for (int val : data) {
            sq_sum_diff += (static_cast<double>(val) - mean) * (static_cast<double>(val) - mean);
        }
        return sq_sum_diff / (data.size() - 1);
    }
}


// Test Suite for generateNewGrain
class StochasticModelGenerateGrainTest : public ::testing::Test {
protected:
    StochasticModel model;
    const int NUM_SAMPLES = 10000; // Number of grains to generate for statistical tests
    const float STAT_TOLERANCE_LOOSE = 0.1f; // Tolerance for mean/stddev (e.g., 10%)
    const float STAT_TOLERANCE_TIGHT = 0.05f; // Tighter tolerance

    void SetUp() override {
        model.seedRandomEngine(12345); // Consistent seed for all tests
        model.setSampleRate(44100.0);   // Default sample rate
    }
};

TEST_F(StochasticModelGenerateGrainTest, IsAliveFlag) {
    Grain grain;
    model.generateNewGrain(grain);
    EXPECT_TRUE(grain.isAlive);
}

TEST_F(StochasticModelGenerateGrainTest, PitchGenerationNormalDistribution) {
    float targetMeanPitch = 60.0f;
    float targetStdDevPitch = 5.0f;
    model.setPitchAndDispersion(targetMeanPitch, targetStdDevPitch);

    std::vector<float> pitches;
    pitches.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        pitches.push_back(grain.pitch);
    }

    float observedMean = Stats::calculateMean(pitches);
    float observedStdDev = Stats::calculateStdDev(pitches, observedMean);

    EXPECT_NEAR(observedMean, targetMeanPitch, targetMeanPitch * STAT_TOLERANCE_LOOSE);
    EXPECT_NEAR(observedStdDev, targetStdDevPitch, targetStdDevPitch * STAT_TOLERANCE_LOOSE);
}

TEST_F(StochasticModelGenerateGrainTest, PitchGenerationZeroDispersion) {
    float targetMeanPitch = 72.5f;
    model.setPitchAndDispersion(targetMeanPitch, 0.0f); // Zero dispersion

    std::vector<float> pitches;
    pitches.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        pitches.push_back(grain.pitch);
    }

    float observedMean = Stats::calculateMean(pitches);
    for (float pitch : pitches) {
        EXPECT_NEAR(pitch, targetMeanPitch, 1e-4f); // Should be very close to mean
    }
    EXPECT_NEAR(observedMean, targetMeanPitch, 1e-4f);
}

TEST_F(StochasticModelGenerateGrainTest, PanGenerationNormalDistributionAndClamping) {
    float targetMeanPan = 0.1f;
    float targetStdDevPan = 0.25f; // Small enough spread, most values should be within [-1, 1] before clamping
    model.setPanAndSpread(targetMeanPan, targetStdDevPan);

    std::vector<float> pans;
    pans.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        pans.push_back(grain.pan);
    }

    float observedMean = Stats::calculateMean(pans);
    float observedStdDev = Stats::calculateStdDev(pans, observedMean);

    for (float pan : pans) {
        EXPECT_GE(pan, -1.0f);
        EXPECT_LE(pan, 1.0f);
    }

    EXPECT_NEAR(observedMean, targetMeanPan, targetMeanPan * STAT_TOLERANCE_LOOSE + STAT_TOLERANCE_TIGHT); // Pan can be 0, so add small absolute tolerance
    EXPECT_NEAR(observedStdDev, targetStdDevPan, targetStdDevPan * STAT_TOLERANCE_LOOSE);
}

TEST_F(StochasticModelGenerateGrainTest, PanGenerationZeroSpread) {
    float targetMeanPan = -0.5f;
    model.setPanAndSpread(targetMeanPan, 0.0f); // Zero spread

    std::vector<float> pans;
    pans.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        pans.push_back(grain.pan);
    }

    float observedMean = Stats::calculateMean(pans);
    for (float pan : pans) {
        EXPECT_NEAR(pan, targetMeanPan, 1e-4f);
    }
    EXPECT_NEAR(observedMean, targetMeanPan, 1e-4f);
}

TEST_F(StochasticModelGenerateGrainTest, PanGenerationWideSpreadAndClamping) {
    float targetMeanPan = 0.0f;
    float targetStdDevPan = 1.5f; // Large spread, many values will be clamped
    model.setPanAndSpread(targetMeanPan, targetStdDevPan);

    std::vector<float> pans;
    pans.reserve(NUM_SAMPLES);
    bool foundClampedLow = false;
    bool foundClampedHigh = false;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        pans.push_back(grain.pan);
        if (grain.pan == -1.0f) foundClampedLow = true;
        if (grain.pan == 1.0f) foundClampedHigh = true;
    }

    for (float pan : pans) {
        EXPECT_GE(pan, -1.0f);
        EXPECT_LE(pan, 1.0f);
    }
    // With such wide spread, mean should still be around targetMeanPan due to symmetry
    // but stddev will be smaller than targetStdDevPan due to clamping.
    float observedMean = Stats::calculateMean(pans);
    EXPECT_NEAR(observedMean, targetMeanPan, STAT_TOLERANCE_LOOSE);

    // It's hard to predict exact stddev after clamping without simulation.
    // But we expect clamping to occur.
    EXPECT_TRUE(foundClampedLow);
    EXPECT_TRUE(foundClampedHigh);
}


TEST_F(StochasticModelGenerateGrainTest, DurationGenerationUniformDistribution) {
    double sampleRate = 44100.0;
    model.setSampleRate(sampleRate);
    float avgDurationMs = 200.0f;
    float variation = 0.25f; // Duration will be avgDurationMs * [1-0.25, 1+0.25] = [150ms, 250ms]
    model.setDurationAndVariation(avgDurationMs, variation);

    float expectedMinDurationMs = avgDurationMs * (1.0f - variation);
    float expectedMaxDurationMs = avgDurationMs * (1.0f + variation);
    float expectedMeanDurationMs = avgDurationMs;

    std::vector<float> durationsMs;
    durationsMs.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        // Convert durationInSamples back to ms for comparison
        durationsMs.push_back(static_cast<float>(grain.durationInSamples) / sampleRate * 1000.0f);
    }

    float observedMeanMs = Stats::calculateMean(durationsMs);
    float observedMinMs = *std::min_element(durationsMs.begin(), durationsMs.end());
    float observedMaxMs = *std::max_element(durationsMs.begin(), durationsMs.end());

    // Check mean
    EXPECT_NEAR(observedMeanMs, expectedMeanDurationMs, expectedMeanDurationMs * STAT_TOLERANCE_TIGHT);

    // Check if observed range is close to expected range
    // Allow some tolerance because min/max of N samples might not hit theoretical bounds
    float rangeTolerance = (expectedMaxDurationMs - expectedMinDurationMs) * 0.05; // 5% of range for tolerance
    EXPECT_NEAR(observedMinMs, expectedMinDurationMs, rangeTolerance);
    EXPECT_NEAR(observedMaxMs, expectedMaxDurationMs, rangeTolerance);

    // Check that all values are within the absolute possible range (plus small epsilon for float issues)
    for (float duration : durationsMs) {
        EXPECT_GE(duration, expectedMinDurationMs - 1e-3f);
        EXPECT_LE(duration, expectedMaxDurationMs + 1e-3f);
    }
}

TEST_F(StochasticModelGenerateGrainTest, DurationGenerationZeroVariation) {
    double sampleRate = 44100.0;
    model.setSampleRate(sampleRate);
    float avgDurationMs = 300.0f;
    model.setDurationAndVariation(avgDurationMs, 0.0f); // Zero variation

    float expectedDurationMs = avgDurationMs;
    int expectedDurationSamples = static_cast<int>((expectedDurationMs / 1000.0f) * sampleRate);

    std::vector<int> durationsSamples;
    durationsSamples.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        durationsSamples.push_back(grain.durationInSamples);
    }

    for (int duration : durationsSamples) {
        EXPECT_EQ(duration, expectedDurationSamples);
    }
}

TEST_F(StochasticModelGenerateGrainTest, DurationGenerationNegativeClamping) {
    // Test that duration is clamped at 0 if avgDurationMs * (1 - variation) becomes negative
    double sampleRate = 44100.0;
    model.setSampleRate(sampleRate);
    float avgDurationMs = 50.0f;
    float variation = 1.5f; // avg * (1-1.5) = -25ms. Should be clamped to 0.
    model.setDurationAndVariation(avgDurationMs, variation);

    float expectedMinDurationMsAfterClamp = 0.0f;
    // avg * (1+1.5) = 125ms
    float expectedMaxDurationMs = avgDurationMs * (1.0f + variation);


    std::vector<float> durationsMs;
    durationsMs.reserve(NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        Grain grain;
        model.generateNewGrain(grain);
        durationsMs.push_back(static_cast<float>(grain.durationInSamples) / sampleRate * 1000.0f);
    }

    float observedMinMs = *std::min_element(durationsMs.begin(), durationsMs.end());
    float observedMaxMs = *std::max_element(durationsMs.begin(), durationsMs.end());

    // Check if min is clamped at 0
    EXPECT_NEAR(observedMinMs, expectedMinDurationMsAfterClamp, 1e-3f); // Duration in samples is int, so conversion might not be exactly 0.0 for min

    // Check if max is as expected
    float rangeTolerance = (expectedMaxDurationMs - expectedMinDurationMsAfterClamp) * 0.05;
    EXPECT_NEAR(observedMaxMs, expectedMaxDurationMs, rangeTolerance);

    for (float duration : durationsMs) {
        EXPECT_GE(duration, 0.0f - 1e-4f); // Should not be negative
    }
}


// Test Suite for getSamplesUntilNextEvent
class StochasticModelNextEventTest : public ::testing::Test {
protected:
    StochasticModel model;
    const int NUM_SAMPLES_STATISTICAL = 10000; // Number of samples for statistical tests
    const double STAT_TOLERANCE_EVENT_TIME = 0.15; // Tolerance for mean/variance of event times (Poisson)

    void SetUp() override {
        model.seedRandomEngine(54321); // Consistent seed
        model.setSampleRate(44100.0);   // Default sample rate
    }
};

TEST_F(StochasticModelNextEventTest, UniformDistributionBasic) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Uniform);
    double sampleRate = model.getSampleRate();

    float densityGPS = 10.0f;
    model.setGlobalDensity(densityGPS);
    int expectedSamples = static_cast<int>(sampleRate / densityGPS);

    for (int i = 0; i < 10; ++i) { // Call multiple times, should be consistent
        EXPECT_EQ(model.getSamplesUntilNextEvent(), expectedSamples);
    }

    densityGPS = 1.0f;
    model.setGlobalDensity(densityGPS);
    expectedSamples = static_cast<int>(sampleRate / densityGPS);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(model.getSamplesUntilNextEvent(), expectedSamples);
    }
}

TEST_F(StochasticModelNextEventTest, PoissonDistributionBasic) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    double sampleRate = model.getSampleRate();
    float densityGPS = 10.0f; // Grains per second
    model.setGlobalDensity(densityGPS);

    double expectedLambda = sampleRate / densityGPS; // Mean of Poisson distribution

    std::vector<int> eventIntervals;
    eventIntervals.reserve(NUM_SAMPLES_STATISTICAL);
    for (int i = 0; i < NUM_SAMPLES_STATISTICAL; ++i) {
        eventIntervals.push_back(model.getSamplesUntilNextEvent());
    }

    double observedMean = Stats::calculateIntMean(eventIntervals);
    double observedVariance = Stats::calculateIntVar(eventIntervals, observedMean);

    // For Poisson, mean and variance should be approximately equal to lambda
    EXPECT_NEAR(observedMean, expectedLambda, expectedLambda * STAT_TOLERANCE_EVENT_TIME);
    EXPECT_NEAR(observedVariance, expectedLambda, expectedLambda * STAT_TOLERANCE_EVENT_TIME * 1.5); // Variance can be more noisy
}

TEST_F(StochasticModelNextEventTest, EdgeCaseZeroDensity) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Uniform);
    model.setGlobalDensity(0.0f);
    EXPECT_EQ(model.getSamplesUntilNextEvent(), INT_MAX);

    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    model.setGlobalDensity(0.0f);
    EXPECT_EQ(model.getSamplesUntilNextEvent(), INT_MAX);
}

TEST_F(StochasticModelNextEventTest, EdgeCaseNegativeDensity) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Uniform);
    model.setGlobalDensity(-10.0f);
    EXPECT_EQ(model.getSamplesUntilNextEvent(), INT_MAX);

    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    model.setGlobalDensity(-10.0f);
    EXPECT_EQ(model.getSamplesUntilNextEvent(), INT_MAX);
}

TEST_F(StochasticModelNextEventTest, EdgeCaseZeroSampleRate) {
    // Note: Setting sample rate to 0 is unusual but testing the model's robustness
    model.setSampleRate(0.0);
    model.setGlobalDensity(10.0f); // Positive density

    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Uniform);
    EXPECT_EQ(model.getSamplesUntilNextEvent(), INT_MAX);

    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    EXPECT_EQ(model.getSamplesUntilNextEvent(), INT_MAX);
}

TEST_F(StochasticModelNextEventTest, PoissonDistributionLowDensity) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    double sampleRate = model.getSampleRate();
    // Very low density, e.g., 1 grain every 10 seconds
    float densityGPS = 0.1f;
    model.setGlobalDensity(densityGPS);

    double expectedLambda = sampleRate / densityGPS; // Should be a large number

    std::vector<int> eventIntervals;
    // Reduce sample count for this test as intervals can be very large, speeding up test
    int reducedSamples = NUM_SAMPLES_STATISTICAL / 10;
    eventIntervals.reserve(reducedSamples);
    for (int i = 0; i < reducedSamples; ++i) {
        eventIntervals.push_back(model.getSamplesUntilNextEvent());
    }

    double observedMean = Stats::calculateIntMean(eventIntervals);
    EXPECT_NEAR(observedMean, expectedLambda, expectedLambda * STAT_TOLERANCE_EVENT_TIME);
    // All generated intervals should be non-negative
    for(int interval : eventIntervals) {
        ASSERT_GE(interval, 0);
    }
}

TEST_F(StochasticModelNextEventTest, PoissonDistributionHighDensity) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    double sampleRate = model.getSampleRate();
    // High density, e.g., 1000 grains per second
    // This means lambda = 44100 / 1000 = 44.1 (average samples per grain)
    float densityGPS = 1000.0f;
    model.setGlobalDensity(densityGPS);

    double expectedLambda = sampleRate / densityGPS;
    ASSERT_GT(expectedLambda, 0); // Lambda must be > 0 for poisson

    std::vector<int> eventIntervals;
    eventIntervals.reserve(NUM_SAMPLES_STATISTICAL);
    for (int i = 0; i < NUM_SAMPLES_STATISTICAL; ++i) {
        eventIntervals.push_back(model.getSamplesUntilNextEvent());
    }

    double observedMean = Stats::calculateIntMean(eventIntervals);
    double observedVariance = Stats::calculateIntVar(eventIntervals, observedMean);

    EXPECT_NEAR(observedMean, expectedLambda, expectedLambda * STAT_TOLERANCE_EVENT_TIME);
    EXPECT_NEAR(observedVariance, expectedLambda, expectedLambda * STAT_TOLERANCE_EVENT_TIME * 1.5);
}

TEST_F(StochasticModelNextEventTest, PoissonDistributionVeryHighDensityLambdaLessThanOne) {
    model.setGlobalTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);
    double sampleRate = 100.0; // Use a low sample rate for this test
    model.setSampleRate(sampleRate);

    // Density such that lambda = sampleRate / density is < 1
    // e.g., density = 200 GPS => lambda = 100 / 200 = 0.5
    float densityGPS = 200.0f;
    model.setGlobalDensity(densityGPS);

    double expectedLambda = sampleRate / densityGPS;
    ASSERT_LT(expectedLambda, 1.0);
    ASSERT_GT(expectedLambda, 0.0); // Ensure it's positive

    // The code uses std::max(epsilon, averageSamplesPerGrain) for poisson mean.
    // So, actualLambdaUsed will be std::max(DBL_EPSILON, expectedLambda).
    double actualLambdaUsed = std::max(std::numeric_limits<double>::epsilon(), expectedLambda);

    std::vector<int> eventIntervals;
    eventIntervals.reserve(NUM_SAMPLES_STATISTICAL);
    for (int i = 0; i < NUM_SAMPLES_STATISTICAL; ++i) {
        eventIntervals.push_back(model.getSamplesUntilNextEvent());
    }

    double observedMean = Stats::calculateIntMean(eventIntervals);
    // When lambda is very small, many events will be 0.
    // Mean should still be around actualLambdaUsed.
    EXPECT_NEAR(observedMean, actualLambdaUsed, actualLambdaUsed * STAT_TOLERANCE_EVENT_TIME + 0.1); // Add small abs tolerance for very small lambda

    bool hasZero = false;
    for(int val : eventIntervals) {
        if (val == 0) hasZero = true;
        ASSERT_GE(val, 0); // Poisson should not return negative
    }
    if (actualLambdaUsed < 1.0) { // If lambda is small, 0s are expected.
        EXPECT_TRUE(hasZero);
    }
}
