#include "Pointilsynth/GrainEnvelope.h"
#include <gtest/gtest.h>
#include <cmath> // For M_PI, cosf

// Add M_PI definition if not present (though cmath should provide it)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float FLOAT_PRECISION = 1e-6f;

TEST(GrainEnvelopeTest, CanConstruct) {
    EXPECT_NO_THROW(std::make_unique<GrainEnvelope>());
}

TEST(GrainEnvelopeTest, ShapeSettingAndBasicOutput) {
    GrainEnvelope envelope;

    // Default is Trapezoid
    // For totalDuration = 1, rampSamples becomes 0 after triangular adjustment (1/2=0).
    // Then the 'if (rampSamples == 0 && totalDuration > 0)' check makes it return 1.0f.
    // However, the very first check 'if (totalDuration == 1) return 1.0f;' handles this.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 1), 1.0f);

    envelope.setShape(GrainEnvelope::Shape::Hann);
    // Hann: 0.5 * (1 - cos(2*PI*0/1)) = 0.5 * (1-1) = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 1), 0.0f);

    // Check a slightly longer duration for basic shape check
    // Hann: currentSample=0 should always be 0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 100), 0.0f);

    envelope.setShape(GrainEnvelope::Shape::Trapezoid);
    // Trapezoid: totalDuration=100, rampSamples = 10. currentSample=0 is in attack.
    // Attack: currentSample / (rampSamples - 1) = 0 / 9 = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 100), 0.0f);
}

TEST(GrainEnvelopeTest, InvalidInputParameters) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Hann); // Shape shouldn't matter for these checks

    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 0), 0.0f);    // totalDuration = 0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, -1), 0.0f);   // totalDuration = -1
    EXPECT_FLOAT_EQ(envelope.getAmplitude(-1, 100), 0.0f); // currentSample = -1
    EXPECT_FLOAT_EQ(envelope.getAmplitude(100, 100), 0.0f); // currentSample = totalDuration
    EXPECT_FLOAT_EQ(envelope.getAmplitude(101, 100), 0.0f); // currentSample > totalDuration

    envelope.setShape(GrainEnvelope::Shape::Trapezoid);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, -1), 0.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(-1, 100), 0.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(100, 100), 0.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(101, 100), 0.0f);
}

TEST(GrainEnvelopeTest, HannEnvelopeSpecifics) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Hann);
    int totalDuration = 1000; // Reasonably large duration

    // Value at currentSample = 0
    // Formula: 0.5 * (1 - cos(2*PI*0/N)) = 0.5 * (1-1) = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, totalDuration), 0.0f);

    // Value at currentSample = totalDuration / 4
    // Formula: 0.5 * (1 - cos(2*PI*(N/4)/N)) = 0.5 * (1 - cos(PI/2)) = 0.5 * (1 - 0) = 0.5
    EXPECT_NEAR(envelope.getAmplitude(totalDuration / 4, totalDuration), 0.5f, FLOAT_PRECISION);

    // Value at currentSample = totalDuration / 2
    // Formula: 0.5 * (1 - cos(2*PI*(N/2)/N)) = 0.5 * (1 - cos(PI)) = 0.5 * (1 - (-1)) = 1.0
    EXPECT_NEAR(envelope.getAmplitude(totalDuration / 2, totalDuration), 1.0f, FLOAT_PRECISION);

    // Value at currentSample = 3 * totalDuration / 4
    // Formula: 0.5 * (1 - cos(2*PI*(3N/4)/N)) = 0.5 * (1 - cos(3*PI/2)) = 0.5 * (1 - 0) = 0.5
    EXPECT_NEAR(envelope.getAmplitude(3 * totalDuration / 4, totalDuration), 0.5f, FLOAT_PRECISION);

    // Value at currentSample = totalDuration - 1
    // Formula: 0.5 * (1 - cos(2*PI*(N-1)/N)) approx 0.5 * (1 - cos(2*PI)) = 0.0
    // More precisely: 2*PI*(N-1)/N = 2*PI - 2*PI/N. cos(2*PI - x) = cos(x). So cos(2*PI/N).
    // 0.5 * (1 - cos(2*PI/N)). For large N, 2*PI/N is small, cos is near 1. So result near 0.
    EXPECT_NEAR(envelope.getAmplitude(totalDuration - 1, totalDuration),
                0.5f * (1.0f - cosf(2.0f * static_cast<float>(M_PI) * static_cast<float>(totalDuration - 1) / static_cast<float>(totalDuration))),
                FLOAT_PRECISION);
    // A simpler check for near zero for the last sample if duration is large enough:
    EXPECT_NEAR(envelope.getAmplitude(totalDuration - 1, totalDuration), 0.0f, 0.01f);


    // Test with totalDuration = 1
    // Formula: 0.5 * (1 - cos(2*PI*0/1)) = 0.5 * (1-1) = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 1), 0.0f);
}

TEST(GrainEnvelopeTest, TrapezoidEnvelopeSpecifics) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);

    // Test totalDuration = 1
    // Code: if (totalDuration == 1) return 1.0f;
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 1), 1.0f);

    // Test totalDuration = 2 (rampSamples becomes 1)
    // Attack: currentSample = 0. rampSamples = 1. Returns 1.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 2), 1.0f);
    // Release: currentSample = 1. totalDuration - rampSamples = 2-1 = 1. This is release.
    // rampSamples = 1. Returns 0.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(1, 2), 0.0f);

    // Test totalDuration = 3 (rampSamples becomes 1)
    // Attack: currentSample = 0. rampSamples = 1. Returns 1.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 3), 1.0f);
    // Sustain: currentSample = 1. 1 < (3-1)=2. Returns 1.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(1, 3), 1.0f);
    // Release: currentSample = 2. totalDuration - rampSamples = 3-1 = 2. This is release.
    // rampSamples = 1. Returns 0.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(2, 3), 0.0f);

    // Test "standard" duration: totalDuration = 100
    // rampSamples = round(0.1 * 100) = 10. Not triangular.
    int totalDuration = 100;
    int rampSamples = 10; // Expected ramp samples

    // Attack phase
    // currentSample = 0. Expected: 0.0 / (10-1) = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, totalDuration), 0.0f);
    // currentSample = rampSamples / 2 = 5. Expected: 5.0 / 9.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(rampSamples / 2, totalDuration), static_cast<float>(rampSamples / 2) / static_cast<float>(rampSamples - 1));
    // currentSample = rampSamples - 1 = 9. Expected: (10-1)/(10-1) = 1.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(rampSamples - 1, totalDuration), 1.0f);

    // Sustain phase
    // currentSample = rampSamples = 10. Expected: 1.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(rampSamples, totalDuration), 1.0f);
    // currentSample = totalDuration / 2 = 50. Expected: 1.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(totalDuration / 2, totalDuration), 1.0f);
    // currentSample = totalDuration - rampSamples - 1 = 100 - 10 - 1 = 89. Expected: 1.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(totalDuration - rampSamples - 1, totalDuration), 1.0f);

    // Release phase
    // currentSample = totalDuration - rampSamples = 100 - 10 = 90. Expected: 1.0 (start of release ramp)
    // relativeSampleInRelease = 90 - (100 - 10) = 0. 1.0 - (0 / 9) = 1.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(totalDuration - rampSamples, totalDuration), 1.0f);
    // currentSample = totalDuration - rampSamples / 2 = 100 - 5 = 95.
    // relativeSampleInRelease = 95 - (100 - 10) = 5. Expected: 1.0 - (5.0 / 9.0)
    EXPECT_FLOAT_EQ(envelope.getAmplitude(totalDuration - rampSamples / 2, totalDuration), 1.0f - (static_cast<float>(rampSamples / 2) / static_cast<float>(rampSamples - 1)));
    // currentSample = totalDuration - 1 = 99. Expected: 1.0 - ((99 - 90)/9.0) = 1.0 - (9/9) = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(totalDuration - 1, totalDuration), 0.0f);

    // Test triangular case: totalDuration = 5
    // rampSamples = round(0.1 * 5) = 1.
    // 2 * rampSamples (2) <= totalDuration (5). Not triangular by the `2 * rampSamples > totalDuration` rule.
    // It remains rampSamples = 1.
    // Attack: currentSample = 0. rampSamples = 1. Returns 1.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 5), 1.0f);
    // Sustain: currentSample = 1. 1 < (5-1)=4. Returns 1.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(1, 5), 1.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(2, 5), 1.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(3, 5), 1.0f);
    // Release: currentSample = 4. totalDuration - rampSamples = 5-1 = 4. This is release.
    // rampSamples = 1. Returns 0.0f.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(4, 5), 0.0f);

    // Test triangular case: totalDuration = 15
    // rampSamples = round(0.1 * 15) = round(1.5) = 2.
    // 2 * rampSamples (4) <= totalDuration (15). Not made triangular by the specific rule.
    // Attack (rampSamples = 2):
    // s=0: 0/(2-1) = 0
    // s=1: 1/(2-1) = 1
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, 15), 0.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(1, 15), 1.0f);
    // Sustain (from s=2 up to 15-2-1 = 12)
    EXPECT_FLOAT_EQ(envelope.getAmplitude(2, 15), 1.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(12, 15), 1.0f);
    // Release (rampSamples = 2, starts at s = 15-2 = 13)
    // s=13: relative = 0. 1.0 - (0/1) = 1.0
    // s=14: relative = 1. 1.0 - (1/1) = 0.0
    EXPECT_FLOAT_EQ(envelope.getAmplitude(13, 15), 1.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(14, 15), 0.0f);


    // Test case that *should* trigger triangular: totalDuration = 10, initialRampProportion = 0.6f (hypothetically)
    // This would make rampSamples = 6. Then 2*6 (12) > 10. So rampSamples becomes 10/2 = 5.
    // This test requires changing initialRampProportion, which is not possible with current class.
    // The current class always uses 0.1f.
    // Let's test a case where 0.1f * totalDuration leads to `2 * rampSamples > totalDuration`.
    // e.g. totalDuration = 10. rampSamples = round(1) = 1. 2*1 (2) <= 10. No.
    // e.g. totalDuration = 19. rampSamples = round(1.9) = 2. 2*2 (4) <= 19. No.
    // The condition `2 * rampSamples > totalDuration` implies ramp must be > 25% of total duration.
    // Since ramp is 10%, this condition `2 * (0.1 * N) > N` -> `0.2N > N` is false for N > 0.
    // The only way it's hit is if `totalDuration / 2` (from integer division) is what `rampSamples` becomes.
    // E.g., totalDuration = 3. rampSamples = round(0.3) = 0. Then rampSamples = 1.
    // Then: 2 * 1 (2) <= 3. Not triangular.
    // The rule `if (2 * rampSamples > totalDuration) { rampSamples = totalDuration / 2; }`
    // seems to be intended for when initial ramp proportion is larger. With fixed 10%, it's
    // unlikely to make a typical trapezoid triangular unless totalDuration is very small.
    // For totalDuration = 1, rampSamples becomes 0 (1/2).
    // For totalDuration = 2, rampSamples becomes 1 (2/2).
    // For totalDuration = 3, rampSamples becomes 1 (3/2).
    // Let's re-test totalDuration = 2 (rampSamples becomes 1 due to triangular adjustment of 2/2 = 1)
    // Initial rampSamples = round(0.1*2) = 0. Then adjusted to 1.
    // Then `2 * rampSamples > totalDuration` => `2*1 > 2` is FALSE. So `rampSamples` remains 1.
    // This means the triangular adjustment `rampSamples = totalDuration / 2` is NOT hit for small N with 10% ramp.
    // The logic for rampSamples is:
    // 1. rampSamples = round(0.1f * totalDuration)
    // 2. if (rampSamples == 0 && totalDuration > 1) rampSamples = 1
    // 3. if (2 * rampSamples > totalDuration) rampSamples = totalDuration / 2
    //
    // Consider totalDuration = 5.
    // 1. rampSamples = round(0.5) = 1.
    // 2. skipped.
    // 3. 2*1 > 5 is false. rampSamples remains 1.
    // (This matches my test above for totalDuration=5 which behaves as ramp=1).
    //
    // The triangular adjustment `rampSamples = totalDuration / 2` is thus only hit if the *initial* ramp calculation
    // (from a potentially variable proportion, not fixed 10%) results in a large ramp.
    // With fixed 10%, this rule seems dormant unless totalDuration is small enough that step 2 (set to 1)
    // then makes 2*1 > totalDuration. This means totalDuration would have to be < 2.
    // If totalDuration = 1, step 1: ramp = 0. Step 2: skipped. Step 3: 2*0 > 1 false. rampSamples=0.
    //   Then `if (rampSamples == 0 && totalDuration > 0)` -> returns 1.0f. (Handled by first `if (totalDuration == 1)`).
    // So, the specific triangular adjustment `rampSamples = totalDuration / 2` might not be easily testable
    // without a way to set a larger initial ramp proportion.
    // The current tests for small durations (1, 2, 3, 5) effectively cover how it behaves at extremes.
}

// Optional property-based style tests (simplified)
TEST(GrainEnvelopeTest, HannProperties) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Hann);
    int duration1 = 10;
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, duration1), 0.0f);
    EXPECT_NEAR(envelope.getAmplitude(duration1 / 2, duration1), 1.0f, FLOAT_PRECISION);

    int duration2 = 1000;
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, duration2), 0.0f);
    EXPECT_NEAR(envelope.getAmplitude(duration2 / 2, duration2), 1.0f, FLOAT_PRECISION);
}

TEST(GrainEnvelopeTest, TrapezoidProperties) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);

    // For reasonably long durations, middle should be 1.0
    int duration1 = 30; // ramp=3. sustain from 3 to 26. Middle is 15.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(duration1 / 2, duration1), 1.0f);

    int duration2 = 1000; // ramp=100. sustain from 100 to 899. Middle is 500.
    EXPECT_FLOAT_EQ(envelope.getAmplitude(duration2 / 2, duration2), 1.0f);

    // Test attack start for longer durations
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, duration1), 0.0f);
    EXPECT_FLOAT_EQ(envelope.getAmplitude(0, duration2), 0.0f);
}

[end of test/source/GrainEnvelopeTest.cpp]
