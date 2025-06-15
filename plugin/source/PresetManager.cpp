#include "Pointilsynth/PresetManager.h"
#include "juce_core/juce_core.h"  // For juce::File
#include "nlohmann/json.hpp"      // For nlohmann::json

namespace Pointilism {

PresetManager::PresetManager(StochasticModel& model) : model_(model) {}

bool PresetManager::savePreset(const juce::File& fileToSave) {
  nlohmann::json j;
  j["pitch"] = model_.getPitch();
  j["dispersion"] = model_.getDispersion();
  j["averageDurationMs"] = model_.getAverageDurationMs();
  j["durationVariation"] = model_.getDurationVariation();
  j["centralPan"] = model_.getCentralPan();
  j["panSpread"] = model_.getPanSpread();
  j["globalDensity"] = model_.getGlobalDensity();
  j["globalMinDistance"] = model_.getGlobalMinDistance();
  j["globalPitchOffset"] = model_.getGlobalPitchOffset();
  j["globalPanOffset"] = model_.getGlobalPanOffset();
  j["globalVelocityOffset"] = model_.getGlobalVelocityOffset();
  j["globalDurationOffset"] = model_.getGlobalDurationOffset();
  j["globalTempoSyncEnabled"] = model_.isGlobalTempoSyncEnabled();
  j["globalNumVoices"] = model_.getGlobalNumVoices();
  j["globalNumGrains"] = model_.getGlobalNumGrains();
  j["globalTemporalDistribution"] =
      static_cast<int>(model_.getGlobalTemporalDistribution());

  juce::FileOutputStream out(fileToSave);
  if (!out.openedOk()) {
    return false;
  }

  std::string data = j.dump(4);
  out.write(data.c_str(), data.size());
  return true;
}

bool PresetManager::loadPreset(const juce::File& fileToLoad) {
  juce::FileInputStream in(fileToLoad);
  if (!in.openedOk()) {
    return false;
  }

  auto text = in.readEntireStreamAsString();
  nlohmann::json j;
  try {
    j = nlohmann::json::parse(text.toStdString());
  } catch (const nlohmann::json::exception&) {
    return false;
  }

  if (j.contains("pitch") && j.contains("dispersion"))
    model_.setPitchAndDispersion(j["pitch"].get<float>(),
                                 j["dispersion"].get<float>());

  if (j.contains("averageDurationMs") && j.contains("durationVariation"))
    model_.setDurationAndVariation(j["averageDurationMs"].get<float>(),
                                   j["durationVariation"].get<float>());

  if (j.contains("centralPan") && j.contains("panSpread"))
    model_.setPanAndSpread(j["centralPan"].get<float>(),
                           j["panSpread"].get<float>());

  if (j.contains("globalDensity"))
    model_.setGlobalDensity(j["globalDensity"].get<float>());
  if (j.contains("globalMinDistance"))
    model_.setGlobalMinDistance(j["globalMinDistance"].get<float>());
  if (j.contains("globalPitchOffset"))
    model_.setGlobalPitchOffset(j["globalPitchOffset"].get<int>());
  if (j.contains("globalPanOffset"))
    model_.setGlobalPanOffset(j["globalPanOffset"].get<float>());
  if (j.contains("globalVelocityOffset"))
    model_.setGlobalVelocityOffset(j["globalVelocityOffset"].get<float>());
  if (j.contains("globalDurationOffset"))
    model_.setGlobalDurationOffset(j["globalDurationOffset"].get<float>());
  if (j.contains("globalTempoSyncEnabled"))
    model_.setGlobalTempoSyncEnabled(j["globalTempoSyncEnabled"].get<bool>());
  if (j.contains("globalNumVoices"))
    model_.setGlobalNumVoices(j["globalNumVoices"].get<int>());
  if (j.contains("globalNumGrains"))
    model_.setGlobalNumGrains(j["globalNumGrains"].get<int>());
  if (j.contains("globalTemporalDistribution"))
    model_.setGlobalTemporalDistribution(
        static_cast<StochasticModel::TemporalDistribution>(
            j["globalTemporalDistribution"].get<int>()));

  return true;
}

}  // namespace Pointilism
