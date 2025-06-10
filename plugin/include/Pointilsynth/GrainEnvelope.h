#ifndef GRAIN_ENVELOPE_H_
#define GRAIN_ENVELOPE_H_

#include <cmath> // For M_PI, later for cosf

// Add M_PI definition if not present (though cmath should provide it)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GrainEnvelope {
public:
    enum class Shape {
        Trapezoid,
        Hann
    };

    GrainEnvelope() : currentShape_(Shape::Trapezoid) {}

    void setShape(Shape newShape) {
        currentShape_ = newShape;
    }

    float getAmplitude(int currentSample, int totalDuration) {
        if (totalDuration <= 0 || currentSample < 0 || currentSample >= totalDuration) {
            return 0.0f; // Invalid parameters or outside the duration
        }

        switch (currentShape_) {
            case Shape::Hann: {
                // Requirement: 0.5 * (1 - cos(2 * PI * currentSample / totalDuration))
                // Gain should be 0.0 at the start and end.
                // For currentSample = 0, cos(0) = 1, so 0.5 * (1-1) = 0.
                // For currentSample = totalDuration, effectively, this means the phase is 2*PI.
                // If currentSample == totalDuration, it's out of bounds by the check above.
                // The formula naturally handles start = 0.
                // To ensure end is also 0, the last valid sample is totalDuration - 1.
                // If we use currentSample / (totalDuration -1) for N-1 points, the last point is 0.
                // If we use currentSample / totalDuration for N points, the Nth point (totalDuration) makes phase 2PI.
                // The prompt is specific: currentSample / totalDuration.
                // The formula 0.5 * (1 - cos(2*PI*n/N)) is used, where n is currentSample and N is totalDuration.
                // This formula correctly evaluates to 0 for n=0.
                // For totalDuration = 1, currentSample must be 0. The formula yields:
                // 0.5 * (1 - cos(0)) = 0.5 * (1 - 1) = 0.0.
                // Thus, no special handling for totalDuration = 1 is needed for Hann.

                // The formula is for a continuous function. For discrete samples 0 to N-1 over a period N:
                // x_n = 0.5 * (1 - cos(2*pi*n/N)) for n = 0, ..., N-1
                // The problem phrasing "currentSample / totalDuration" implies N in the denominator.
                return 0.5f * (1.0f - cosf(2.0f * static_cast<float>(M_PI) * static_cast<float>(currentSample) / static_cast<float>(totalDuration)));
            }
            case Shape::Trapezoid: {
                if (totalDuration == 1) return 1.0f; // Single sample is full amplitude

                // Calculate ramp duration based on 10% of total duration, rounded.
                // This 'rampSamples' is used for both attack and release.
                int rampSamples = static_cast<int>(roundf(0.1f * totalDuration));

                // If rounding resulted in 0 ramp samples for a duration > 1, set to at least 1 sample.
                if (rampSamples == 0 && totalDuration > 1) {
                    rampSamples = 1;
                }

                // If the sum of attack and release ramps (2*rampSamples) would exceed totalDuration,
                // it means there's no space for sustain, or phases overlap. Make it triangular.
                if (2 * rampSamples > totalDuration) {
                    rampSamples = totalDuration / 2; // Integer division handles this.
                                                     // e.g., totalDuration=1 -> rampSamples=0.
                                                     // e.g., totalDuration=2 -> rampSamples=1.
                                                     // e.g., totalDuration=3 -> rampSamples=1.
                }

                // If rampSamples ended up as 0 (e.g. totalDuration was 1, then 1/2 = 0),
                // it implies a very short sound that is effectively all sustain, or a single point.
                if (rampSamples == 0 && totalDuration > 0) { // totalDuration=1 is already handled. This covers if totalDuration > 0 leads to rampSamples=0.
                     return 1.0f; // Treat as sustain.
                }
                // If totalDuration is 0, initial check returns 0.0f. If rampSamples is 0 because totalDuration is 0, this path isn't hit.

                // Attack phase: currentSample from 0 to rampSamples - 1
                if (currentSample < rampSamples) {
                    if (rampSamples == 1) return 1.0f; // Single sample attack: instant 1.0
                    // Linear ramp from 0.0 at sample 0 to 1.0 at sample rampSamples - 1
                    return static_cast<float>(currentSample) / static_cast<float>(rampSamples - 1);
                }
                // Sustain phase: currentSample from rampSamples to totalDuration - rampSamples - 1
                else if (currentSample < totalDuration - rampSamples) {
                    return 1.0f;
                }
                // Release phase: currentSample from totalDuration - rampSamples to totalDuration - 1
                else {
                    if (rampSamples == 1) return 0.0f; // Single sample release: instant 0.0
                    // Linear ramp from 1.0 down to 0.0
                    float relativeSampleInRelease = static_cast<float>(currentSample - (totalDuration - rampSamples));
                    return 1.0f - (relativeSampleInRelease / static_cast<float>(rampSamples - 1));
                }
            }
            default:
                return 0.0f;
        }
    }

private:
    Shape currentShape_;
};

#endif // GRAIN_ENVELOPE_H_
