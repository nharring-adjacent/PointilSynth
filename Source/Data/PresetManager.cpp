#include "PresetManager.h"
#include "juce_core/juce_core.h"    // For juce::File, juce::FileInputStream, juce::FileOutputStream
#include "nlohmann/json.hpp"        // For nlohmann::json
#include <fstream>                  // For std::ofstream, std::ifstream

namespace Pointilism
{

PresetManager::PresetManager(StochasticModel& model)
    : model_(model)
{
}

bool PresetManager::savePreset(const juce::File& fileToSave)
{
    nlohmann::json presetJson;

    // --- Global Parameters ---
    presetJson["global"]["density"] = model_.getGlobalDensity();
    presetJson["global"]["minDistance"] = model_.getGlobalMinDistance();
    presetJson["global"]["pitchOffset"] = model_.getGlobalPitchOffset();
    presetJson["global"]["panOffset"] = model_.getGlobalPanOffset();
    presetJson["global"]["velocityOffset"] = model_.getGlobalVelocityOffset();
    presetJson["global"]["durationOffset"] = model_.getGlobalDurationOffset();
    presetJson["global"]["temporalDistribution"] = static_cast<int>(model_.getGlobalTemporalDistribution());
    presetJson["global"]["tempoSync"] = model_.isGlobalTempoSyncEnabled();
    presetJson["global"]["numVoices"] = model_.getGlobalNumVoices();
    presetJson["global"]["numGrains"] = model_.getGlobalNumGrains();

    // --- Stochastic Models (Per Voice) ---
    // Assuming StochasticModel has methods to get these parameters for each voice.
    // This part needs to be adapted based on how StochasticModel stores per-voice data.
    // For simplicity, let's assume there's a fixed number of voices or a way to iterate them.
    // This is a placeholder and needs to match the actual StochasticModel interface.
    // For now, we'll save only the global parameters as a starting point.

    // Example for saving per-voice data if model_ had such a structure:
    /*
    for (int i = 0; i < model_.getNumVoices(); ++i) // Assuming model_.getNumVoices() exists
    {
        presetJson["voices"][i]["density"] = model_.getDensity(i); // Assuming getDensity(voiceIndex)
        presetJson["voices"][i]["minDistance"] = model_.getMinDistance(i);
        // ... and so on for other per-voice parameters
    }
    */

    // Use juce::FileOutputStream to write the JSON data
    juce::FileOutputStream outputStream(fileToSave);

    if (!outputStream.openedOk())
    {
        //DBG("Failed to open file for writing: " + fileToSave.getFullPathName());
        return false;
    }

    // Get the JSON data as a string
    std::string jsonString = presetJson.dump(4); // 4 spaces for indentation

    // Write the string to the file
    outputStream.write(jsonString.c_str(), jsonString.length());

    return true;
}

bool PresetManager::loadPreset(const juce::File& fileToLoad)
{
    juce::FileInputStream inputStream(fileToLoad);

    if (!inputStream.openedOk())
    {
        //DBG("Failed to open file for reading: " + fileToLoad.getFullPathName());
        return false;
    }

    try
    {
        // Read the entire file into a string
        juce::String fileText = inputStream.readEntireStreamAsString();

        // Parse the JSON string
        nlohmann::json presetJson = nlohmann::json::parse(fileText.toStdString());

        // --- Global Parameters ---
        if (presetJson.contains("global"))
        {
            auto& globalParams = presetJson["global"];
            if (globalParams.contains("density")) model_.setGlobalDensity(globalParams["density"].get<float>());
            if (globalParams.contains("minDistance")) model_.setGlobalMinDistance(globalParams["minDistance"].get<float>());
            if (globalParams.contains("pitchOffset")) model_.setGlobalPitchOffset(globalParams["pitchOffset"].get<int>());
            if (globalParams.contains("panOffset")) model_.setGlobalPanOffset(globalParams["panOffset"].get<float>());
            if (globalParams.contains("velocityOffset")) model_.setGlobalVelocityOffset(globalParams["velocityOffset"].get<float>());
            if (globalParams.contains("durationOffset")) model_.setGlobalDurationOffset(globalParams["durationOffset"].get<float>());
            if (globalParams.contains("temporalDistribution")) model_.setGlobalTemporalDistribution(static_cast<StochasticModel::TemporalDistribution>(globalParams["temporalDistribution"].get<int>()));
            if (globalParams.contains("tempoSync")) model_.setGlobalTempoSyncEnabled(globalParams["tempoSync"].get<bool>());
            if (globalParams.contains("numVoices")) model_.setGlobalNumVoices(globalParams["numVoices"].get<int>());
            if (globalParams.contains("numGrains")) model_.setGlobalNumGrains(globalParams["numGrains"].get<int>());
        }

        // --- Stochastic Models (Per Voice) ---
        // Similar to saving, this needs to be adapted based on StochasticModel's structure.
        // This is a placeholder.
        /*
        if (presetJson.contains("voices"))
        {
            const auto& voicesJson = presetJson["voices"];
            for (unsigned i = 0; i < voicesJson.size(); ++i)
            {
                if (i >= static_cast<unsigned>(model_.getNumVoices())) break; // Bounds check

                const auto& voiceParams = voicesJson[i];
                if (voiceParams.contains("density")) model_.setDensity(i, voiceParams["density"].get<float>());
                // ... and so on
            }
        }
        */
    }
    catch (const nlohmann::json::parse_error& e)
    {
        //DBG("JSON parsing error: " + juce::String(e.what()));
        return false;
    }
    catch (const nlohmann::json::type_error& e)
    {
        //DBG("JSON type error: " + juce::String(e.what()));
        return false;
    }
    catch (...)
    {
        //DBG("Unknown error loading preset");
        return false;
    }

    return true;
}

} // namespace Pointilism
