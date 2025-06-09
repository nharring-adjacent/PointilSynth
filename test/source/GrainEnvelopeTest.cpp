#include "Pointilsynth/GrainEnvelope.h" // Adjust path if necessary based on include paths
#include "gtest/gtest.h"
#include <cmath> // For M_PI and potentially other math functions

// Helper for floating point comparisons
const float FLOAT_TOLERANCE = 1e-6f;

TEST(GrainEnvelopeTest, InvalidInputs) {
    GrainEnvelope envelope;
    EXPECT_NEAR(envelope.getAmplitude(0, 0), 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(0, -10), 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(-1, 100), 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(100, 100), 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(101, 100), 0.0f, FLOAT_TOLERANCE);
}

TEST(GrainEnvelopeTest, HannShape) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Hann);

    // Test with totalDuration = 1
    // Formula: 0.5f * (1.0f - cosf(2.0f * M_PI * 0 / 1)) = 0.5 * (1 - 1) = 0
    EXPECT_NEAR(envelope.getAmplitude(0, 1), 0.0f, FLOAT_TOLERANCE);

    // Test with totalDuration = 100
    int duration = 100;
    // Start: 0.5 * (1 - cos(0)) = 0
    EXPECT_NEAR(envelope.getAmplitude(0, duration), 0.0f, FLOAT_TOLERANCE);
    // Midpoint: currentSample = 50. 0.5 * (1 - cos(2*PI*50/100)) = 0.5 * (1 - cos(PI)) = 0.5 * (1 - (-1)) = 1.0
    EXPECT_NEAR(envelope.getAmplitude(duration / 2, duration), 1.0f, FLOAT_TOLERANCE);
    // Near end (sample 99 for duration 100): 0.5 * (1 - cos(2*PI*99/100))
    // This should be close to 0 but not exactly 0 due to discrete sampling.
    // The formula is designed to be 0 at currentSample = duration, which is out of bounds.
    // The value at N-1 for an N-point Hann window: 0.5 * (1 - cos(2*PI*(N-1)/N))
    float val_at_N_minus_1 = 0.5f * (1.0f - cosf(2.0f * static_cast<float>(M_PI) * static_cast<float>(duration - 1) / static_cast<float>(duration)));
    EXPECT_NEAR(envelope.getAmplitude(duration - 1, duration), val_at_N_minus_1, FLOAT_TOLERANCE);


    // Test with totalDuration = 2 (minimum to see some shape other than single point)
    duration = 2;
    // Start (sample 0): 0.5 * (1 - cos(0)) = 0
    EXPECT_NEAR(envelope.getAmplitude(0, duration), 0.0f, FLOAT_TOLERANCE);
    // End (sample 1 for duration 2): 0.5 * (1 - cos(2*PI*1/2)) = 0.5 * (1 - cos(PI)) = 1.0
    EXPECT_NEAR(envelope.getAmplitude(1, duration), 1.0f, FLOAT_TOLERANCE);
}

TEST(GrainEnvelopeTest, TrapezoidShapeDuration1) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);
    EXPECT_NEAR(envelope.getAmplitude(0, 1), 1.0f, FLOAT_TOLERANCE);
}

TEST(GrainEnvelopeTest, TrapezoidShapeNormal) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);

    int duration = 100; // e.g., 100 samples
    int rampSamples = static_cast<int>(roundf(0.1f * static_cast<float>(duration))); // 10 samples

    // Attack phase
    // Sample 0: should be 0.0f because rampSamples-1 is 9. 0 / 9 = 0
    EXPECT_NEAR(envelope.getAmplitude(0, duration), 0.0f, FLOAT_TOLERANCE);
    // Sample rampSamples / 2 (e.g. 5): 5 / 9
    EXPECT_NEAR(envelope.getAmplitude(rampSamples / 2, duration), static_cast<float>(rampSamples/2) / static_cast<float>(rampSamples -1), FLOAT_TOLERANCE);
    // Sample rampSamples - 1 (e.g. 9): should be 1.0. (9 / 9)
    EXPECT_NEAR(envelope.getAmplitude(rampSamples - 1, duration), 1.0f, FLOAT_TOLERANCE);

    // Sustain phase
    // Sample rampSamples (e.g. 10)
    EXPECT_NEAR(envelope.getAmplitude(rampSamples, duration), 1.0f, FLOAT_TOLERANCE);
    // Sample duration / 2 (e.g. 50)
    EXPECT_NEAR(envelope.getAmplitude(duration / 2, duration), 1.0f, FLOAT_TOLERANCE);
    // Sample just before release: duration - rampSamples - 1 (e.g. 100 - 10 - 1 = 89)
    EXPECT_NEAR(envelope.getAmplitude(duration - rampSamples - 1, duration), 1.0f, FLOAT_TOLERANCE);

    // Release phase
    // Sample duration - rampSamples (e.g. 90)
    // relativeSampleInRelease = 0. 1.0 - (0 / 9) = 1.0
    EXPECT_NEAR(envelope.getAmplitude(duration - rampSamples, duration), 1.0f, FLOAT_TOLERANCE);
     // Sample duration - rampSamples / 2 (e.g. 95)
    // relativeSampleInRelease = 5. 1.0 - (5 / 9)
    float relativeSampleInRelease = static_cast<float>(rampSamples/2);
    EXPECT_NEAR(envelope.getAmplitude(duration - rampSamples + (rampSamples/2) , duration), 1.0f - (relativeSampleInRelease / static_cast<float>(rampSamples - 1)), FLOAT_TOLERANCE);
    // Sample duration - 1 (e.g. 99)
    // relativeSampleInRelease = 9. 1.0 - (9 / 9) = 0.0
    EXPECT_NEAR(envelope.getAmplitude(duration - 1, duration), 0.0f, FLOAT_TOLERANCE);
}

TEST(GrainEnvelopeTest, TrapezoidShapeShortDurationTriangular) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);

    // Duration where 2 * rampSamples > totalDuration, making it triangular
    // e.g., duration = 10. Default ramp would be 10% -> 1. 2*1 <= 10. This is not triangular by default.
    // Let's pick duration = 5. Default ramp is round(0.5) = 1. 2*1 <= 5.
    // The condition is `if (2 * rampSamples > totalDuration)`, then `rampSamples = totalDuration / 2;`
    // If duration = 5, initial rampSamples = 1. 2*1 <= 5. No change.
    // Attack: 0 to 0. getAmplitude(0,5) -> 0 / (1-1) -> div by zero. This needs care.
    // If rampSamples is 1: attack is currentSample / (1-1) is bad.
    // Code: `if (rampSamples == 1) return 1.0f;` for attack.
    // Code: `if (rampSamples == 1) return 0.0f;` for release.

    // Test with duration = 5. Initial rampSamples = round(0.5) = 1.
    // Attack phase (currentSample < 1):
    // sample 0: rampSamples is 1, so returns 1.0f
    EXPECT_NEAR(envelope.getAmplitude(0, 5), 1.0f, FLOAT_TOLERANCE); // Attack, rampSamples=1
    // Sustain phase (currentSample < 5 - 1 = 4):
    // sample 1:
    EXPECT_NEAR(envelope.getAmplitude(1, 5), 1.0f, FLOAT_TOLERANCE);
    // sample 2:
    EXPECT_NEAR(envelope.getAmplitude(2, 5), 1.0f, FLOAT_TOLERANCE);
    // sample 3:
    EXPECT_NEAR(envelope.getAmplitude(3, 5), 1.0f, FLOAT_TOLERANCE);
    // Release phase (currentSample >= 4):
    // sample 4: rampSamples is 1, so returns 0.0f
    EXPECT_NEAR(envelope.getAmplitude(4, 5), 0.0f, FLOAT_TOLERANCE); // Release, rampSamples=1

    // Test with duration = 3. Initial rampSamples = round(0.3) = 0. Code: `if (rampSamples == 0 && totalDuration > 1) rampSamples = 1;`
    // So rampSamples becomes 1.
    // Attack (currentSample < 1): sample 0 -> returns 1.0f
    EXPECT_NEAR(envelope.getAmplitude(0, 3), 1.0f, FLOAT_TOLERANCE);
    // Sustain (currentSample < 3 - 1 = 2): sample 1 -> returns 1.0f
    EXPECT_NEAR(envelope.getAmplitude(1, 3), 1.0f, FLOAT_TOLERANCE);
    // Release (currentSample >= 2): sample 2 -> returns 0.0f
    EXPECT_NEAR(envelope.getAmplitude(2, 3), 0.0f, FLOAT_TOLERANCE);


    // Test case where 2 * rampSamples > totalDuration forces triangular.
    // e.g. duration = 15. Default ramp = round(1.5) = 2.
    // 2 * rampSamples = 4. This is not > 15.
    // Let's use the example from code: `totalDuration=3 -> rampSamples=1` (after adjustment from 0).
    // This was tested above.

    // Consider totalDuration = 20. rampSamples = round(2) = 2.
    // Attack: 0 to 1.
    //  curr=0: 0/(2-1) = 0.
    //  curr=1: 1/(2-1) = 1.
    // Sustain: 2 to 20-2-1 = 17.
    // Release: 18 to 19.
    //  curr=18: relative=0. 1 - (0/1) = 1.
    //  curr=19: relative=1. 1 - (1/1) = 0.
    EXPECT_NEAR(envelope.getAmplitude(0, 20), 0.0f, FLOAT_TOLERANCE); // Attack sample 0
    EXPECT_NEAR(envelope.getAmplitude(1, 20), 1.0f, FLOAT_TOLERANCE); // Attack sample 1 (end of attack)
    EXPECT_NEAR(envelope.getAmplitude(10, 20), 1.0f, FLOAT_TOLERANCE); // Sustain
    EXPECT_NEAR(envelope.getAmplitude(18, 20), 1.0f, FLOAT_TOLERANCE); // Release sample 0 (start of release)
    EXPECT_NEAR(envelope.getAmplitude(19, 20), 0.0f, FLOAT_TOLERANCE); // Release sample 1 (end of release)


    // Test a case that forces rampSamples = totalDuration / 2.
    // e.g. totalDuration = 4. Initial rampSamples = round(0.4) = 0. Corrected to 1.
    // 2 * 1 <= 4. Not triangular by that rule.
    // Attack (curr < 1): amp(0,4) -> 1.0 (rampSamples=1 logic)
    // Sustain (curr < 4-1=3): amp(1,4)->1.0, amp(2,4)->1.0
    // Release (curr >=3): amp(3,4) -> 0.0 (rampSamples=1 logic)
    EXPECT_NEAR(envelope.getAmplitude(0, 4), 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(1, 4), 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(2, 4), 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(3, 4), 0.0f, FLOAT_TOLERANCE);

    // What if totalDuration = 10, and ramp percentage makes rampSamples = 3.
    // (This would require changing the 0.1f factor, but let's assume rampSamples = 3 for duration = 10)
    // Code: `if (2 * rampSamples > totalDuration)` -> `2 * 3 = 6 <= 10`. Not triangular.
    // What if rampSamples was 6? (e.g. ramp factor 0.6)
    // `2 * 6 = 12 > 10`. Then `rampSamples = 10 / 2 = 5`.
    // This means the envelope becomes purely attack and release, meeting in the middle.
    // Attack: 0 to 4. (rampSamples = 5)
    //  amp(0,10) -> 0 / (5-1) = 0
    //  amp(4,10) -> 4 / (5-1) = 1
    // Release: 5 to 9. (totalDuration - rampSamples = 10 - 5 = 5)
    //  amp(5,10) -> relative = 0. 1 - (0 / 4) = 1
    //  amp(9,10) -> relative = 4. 1 - (4 / 4) = 0
    // This is a true triangular shape if rampSamples is forced by 2*ramp > total.
    // To test this, we'd need to be able to set ramp percentage or have a duration that forces it.
    // The current GrainEnvelope doesn't allow setting ramp percentage.
    // The condition `if (rampSamples == 0 && totalDuration > 1) { rampSamples = 1; }`
    // The condition `if (2 * rampSamples > totalDuration) { rampSamples = totalDuration / 2; }`
    // Consider duration = 2. Initial ramp = round(0.2) = 0. Corrected to 1.
    // 2*1 <= 2. Not triangular by 2*ramp > total.
    // Attack (curr < 1): amp(0,2) -> 1.0
    // Sustain (curr < 2-1=1): none.
    // Release (curr >=1): amp(1,2) -> 0.0
    EXPECT_NEAR(envelope.getAmplitude(0, 2), 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(envelope.getAmplitude(1, 2), 0.0f, FLOAT_TOLERANCE);
}


TEST(GrainEnvelopeTest, TrapezoidShapeRampIsZeroAndAdjusted) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);

    // Test case: totalDuration is small, initial rampSamples is 0, then adjusted to 1.
    // e.g., totalDuration = 3. ramp = round(0.3) = 0. Adjusted to 1.
    // Attack: currentSample < 1. For sample 0, (rampSamples == 1) -> 1.0f
    EXPECT_NEAR(envelope.getAmplitude(0, 3), 1.0f, FLOAT_TOLERANCE);
    // Sustain: currentSample < (3 - 1) = 2. For sample 1.
    EXPECT_NEAR(envelope.getAmplitude(1, 3), 1.0f, FLOAT_TOLERANCE);
    // Release: currentSample >= (3-1) = 2. For sample 2, (rampSamples == 1) -> 0.0f
    EXPECT_NEAR(envelope.getAmplitude(2, 3), 0.0f, FLOAT_TOLERANCE);

    // Test case: totalDuration is very small, e.g. 2. ramp = round(0.2) = 0. Adjusted to 1.
    // Attack: currentSample < 1. For sample 0, (rampSamples == 1) -> 1.0f
    EXPECT_NEAR(envelope.getAmplitude(0, 2), 1.0f, FLOAT_TOLERANCE);
    // Sustain: currentSample < (2 - 1) = 1. No samples in sustain.
    // Release: currentSample >= (2-1) = 1. For sample 1, (rampSamples == 1) -> 0.0f
    EXPECT_NEAR(envelope.getAmplitude(1, 2), 0.0f, FLOAT_TOLERANCE);
}

TEST(GrainEnvelopeTest, TrapezoidRampSamplesZeroSustainAll) {
    GrainEnvelope envelope;
    envelope.setShape(GrainEnvelope::Shape::Trapezoid);
    // This tests the condition: `if (rampSamples == 0 && totalDuration > 0) { return 1.0f; }`
    // This condition is tricky because `rampSamples` is usually adjusted upwards if it's 0 and totalDuration > 1.
    // `int rampSamples = static_cast<int>(roundf(0.1f * totalDuration));`
    // `if (rampSamples == 0 && totalDuration > 1) { rampSamples = 1; }`
    // So, for rampSamples to remain 0, totalDuration must be such that roundf(0.1f * totalDuration) is 0,
    // AND totalDuration <= 1.
    // If totalDuration = 1, getAmplitude(0,1) returns 1.0f (handled at the start of Trapezoid).
    // If totalDuration = 0, getAmplitude returns 0.0f (handled by initial check for all shapes).
    // The only way `rampSamples == 0 && totalDuration > 0` could be hit *after* the adjustments
    // is if `totalDuration / 2` (in the triangular adjustment) becomes 0.
    // E.g., totalDuration = 1, initial ramp = 0. Then `if (rampSamples == 0 && totalDuration > 1)` is false.
    // Then `if (2 * rampSamples > totalDuration)` (2*0 > 1) is false.
    // Then the final `if (rampSamples == 0 && totalDuration > 0)` is true.
    // So, for totalDuration = 1, this path `rampSamples == 0 && totalDuration > 0` should be hit.
    // However, `if (totalDuration == 1) return 1.0f;` is the very first line in Trapezoid.
    // So this specific internal path `if (rampSamples == 0 && totalDuration > 0) { return 1.0f; }` might be hard to hit directly
    // without refactoring or specific setup. The current logic seems to cover totalDuration=1 correctly already.

    // Let's re-verify the logic for totalDuration = 1:
    // 1. `if (totalDuration == 1) return 1.0f;` -> This is hit. Test is `TrapezoidShapeDuration1`.

    // The case `rampSamples == 0 && totalDuration > 0` leading to `return 1.0f` seems to be primarily for
    // durations like totalDuration = 1 where `totalDuration / 2` (if it were made triangular) could be 0.
    // Given the current structure, it seems the `totalDuration == 1` check handles this scenario.
    // No specific separate test seems needed if `TrapezoidShapeDuration1` passes.
}
