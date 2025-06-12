#pragma once

#include <juce_core/juce_core.h> // For juce::File
#include "nlohmann/json.hpp"     // For nlohmann::json
#include "../../source/PointilismInterfaces.h" // For StochasticModel

// Forward declaration in case StochasticModel is in a namespace
// class StochasticModel; // Not strictly necessary if PointilismInterfaces.h is included correctly

namespace Pointilism
{

class PresetManager
{
public:
    /**
     * Constructor.
     * @param model A reference to the StochasticModel to be managed.
     */
    PresetManager(StochasticModel& model);

    /**
     * Saves the current state of the StochasticModel to a JSON file.
     * @param fileToSave The juce::File object representing the file to save to.
     * @return True if saving was successful, false otherwise.
     */
    bool savePreset(const juce::File& fileToSave);

    /**
     * Loads the state of the StochasticModel from a JSON file.
     * @param fileToLoad The juce::File object representing the file to load from.
     * @return True if loading was successful, false otherwise.
     */
    bool loadPreset(const juce::File& fileToLoad);

private:
    StochasticModel& model_;

    // Helper to convert enum to string and vice-versa for serialization
    // This is important because nlohmann::json doesn't automatically handle enums well.
    // We'll store the enum as its underlying integer type or a string.
    // For TemporalDistribution, int is fine.
};

} // namespace Pointilism
