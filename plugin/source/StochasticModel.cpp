#include "PointilismInterfaces.h" // Required for StochasticModel definition
#include <random> // Will be needed for other methods
#include <algorithm> // Will be needed for std::clamp in other methods

void StochasticModel::setPitchAndDispersion(float centralPitch, float dispersionAmount)
{
    pitch.store(centralPitch);
    dispersion.store(dispersionAmount);
}

void StochasticModel::setPanAndSpread(float newCentralPan, float newSpreadAmount)
{
    centralPan.store(newCentralPan);
    panSpread.store(newSpreadAmount);
}

void StochasticModel::generateNewGrain(Grain& newGrain)
{
    // Pitch
    using PitchDistributionParams = std::normal_distribution<float>::param_type;
    pitchDistribution.param(PitchDistributionParams(pitch.load(), dispersion.load()));
    newGrain.pitch = pitchDistribution(randomEngine);

    // Pan
    using PanDistributionParams = std::normal_distribution<float>::param_type;
    panDistribution.param(PanDistributionParams(centralPan.load(), panSpread.load()));
    float generatedPan = panDistribution(randomEngine);
    newGrain.pan = std::clamp(generatedPan, -1.0f, 1.0f);

    // Other grain properties will be set elsewhere or in future tasks.
    // For now, ensure isAlive is true and assign a temporary ID or leave as default.
    newGrain.isAlive = true;
    // newGrain.id = ...; // Not part of this task's core requirements for pitch/pan
    // newGrain.amplitude = ...;
    // newGrain.durationInSamples = ...;
    // newGrain.ageInSamples = 0;
    // newGrain.sourceSamplePosition = ...;
}
