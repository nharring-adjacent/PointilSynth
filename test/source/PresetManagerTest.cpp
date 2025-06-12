#include "gtest/gtest.h"
#include "Data/PresetManager.h" // Adjusted path
#include "/PointilismInterfaces.h" // For StochasticModel
#include <juce_core/juce_core.h>
#include "nlohmann/json.hpp"

// Basic test suite structure
namespace Pointilism_Test
{
// Test fixture for PresetManager tests.
// Manages a common StochasticModel instance and a temporary file for preset operations.
class PresetManagerTest : public ::testing::Test {
protected:
    Pointilism::StochasticModel testModel; // Model used for testing save/load operations.
    juce::File tempPresetFile; // Temporary file used for saving/loading presets during tests.

    // Constructor: Initializes tempPresetFile path with a random component.
    PresetManagerTest() : tempPresetFile(juce::File::getSpecialLocation(juce::File::tempDirectory)
                                            .getChildFile("testPreset_" + juce::String::toHexString(juce::Random().nextInt64()) + ".json"))
    {
        // Using a randomized file name part to minimize chance of collision if tests run in parallel or if cleanup fails.
    }

    // Destructor: Cleans up the temporary preset file after each test.
    ~PresetManagerTest() override
    {
        if (tempPresetFile.existsAsFile())
        {
            tempPresetFile.deleteFile();
        }
    }

    // Helper function to populate a StochasticModel instance with predefined global parameter values for consistent testing.
    void populateTestModel(Pointilism::StochasticModel& model)
    {
        model.setGlobalDensity(0.75f);
        model.setGlobalMinDistance(0.1f);
        model.setGlobalPitchOffset(-2);
        model.setGlobalPanOffset(0.25f);
        model.setGlobalVelocityOffset(0.9f);
        model.setGlobalDurationOffset(1.1f);
        model.setGlobalTemporalDistribution(Pointilism::StochasticModel::TemporalDistribution::Poisson);
        model.setGlobalTempoSyncEnabled(true);
        model.setGlobalNumVoices(3);
        model.setGlobalNumGrains(150);
    }
};

// Tests the basic construction of the PresetManager.
// Ensures that the PresetManager can be instantiated without issues.
TEST_F(PresetManagerTest, ConstructorTest)
{
    // Test if PresetManager can be constructed
    // More detailed testing of its effect will be in save/load tests
    Pointilism::PresetManager presetManager(testModel);
    SUCCEED(); // If construction doesn't crash, this is a basic success
}

// Tests if savePreset correctly serializes all global parameters from StochasticModel into a JSON file.
// It populates a model, saves it, then checks if the file is created, non-empty, and contains the correct values.
TEST_F(PresetManagerTest, SavePresetSavesCorrectGlobalParameters)
{
    populateTestModel(testModel);
    Pointilism::PresetManager presetManager(testModel);

    ASSERT_TRUE(tempPresetFile.deleteFile()); // Ensure no old file from a previous failed run.
    ASSERT_TRUE(presetManager.savePreset(tempPresetFile)) << "savePreset failed to execute.";
    ASSERT_TRUE(tempPresetFile.existsAsFile()) << "Preset file was not created.";

    juce::String fileContent = tempPresetFile.loadFileAsString();
    ASSERT_FALSE(fileContent.isEmpty()) << "Preset file is empty.";

    nlohmann::json loadedJson;
    try
    {
        loadedJson = nlohmann::json::parse(fileContent.toStdString());
    }
    catch (const nlohmann::json::parse_error& e)
    {
        FAIL() << "JSON parsing error: " << e.what();
    }

    ASSERT_TRUE(loadedJson.contains("global")) << "JSON does not contain 'global' section.";
    const auto& globalParams = loadedJson["global"];

    EXPECT_FLOAT_EQ(globalParams["density"].get<float>(), testModel.getGlobalDensity());
    EXPECT_FLOAT_EQ(globalParams["minDistance"].get<float>(), testModel.getGlobalMinDistance());
    EXPECT_EQ(globalParams["pitchOffset"].get<int>(), testModel.getGlobalPitchOffset());
    EXPECT_FLOAT_EQ(globalParams["panOffset"].get<float>(), testModel.getGlobalPanOffset());
    EXPECT_FLOAT_EQ(globalParams["velocityOffset"].get<float>(), testModel.getGlobalVelocityOffset());
    EXPECT_FLOAT_EQ(globalParams["durationOffset"].get<float>(), testModel.getGlobalDurationOffset());
    EXPECT_EQ(static_cast<Pointilism::StochasticModel::TemporalDistribution>(globalParams["temporalDistribution"].get<int>()), testModel.getGlobalTemporalDistribution());
    EXPECT_EQ(globalParams["tempoSync"].get<bool>(), testModel.isGlobalTempoSyncEnabled());
    EXPECT_EQ(globalParams["numVoices"].get<int>(), testModel.getGlobalNumVoices());
    EXPECT_EQ(globalParams["numGrains"].get<int>(), testModel.getGlobalNumGrains());
}

// Tests if savePreset returns false and does not create a file when given an invalid file path.
// An invalid path here refers to a location where file creation is likely to fail (e.g., a non-existent or protected directory).
TEST_F(PresetManagerTest, SavePresetFailsForInvalidPath)
{
    Pointilism::PresetManager presetManager(testModel);
    // Using a path that is highly unlikely to be writable.
    juce::File invalidFile("/this/path/should/definitely/not/exist/or/be/writable/preset.json");

    // We expect savePreset to return false if it cannot write the file.
    EXPECT_FALSE(presetManager.savePreset(invalidFile)) << "savePreset should fail for an invalid path.";
    ASSERT_FALSE(invalidFile.existsAsFile()) << "File should not have been created at invalid path.";
}

// Tests if loadPreset correctly deserializes global parameters from a JSON file and applies them to StochasticModel.
// It prepares a JSON string, writes it to a temporary file, loads it, and verifies the model's parameters.
TEST_F(PresetManagerTest, LoadPresetLoadsCorrectGlobalParameters)
{
    Pointilism::StochasticModel modelToLoadInto; // Fresh model for loading into.
    Pointilism::PresetManager presetManager(modelToLoadInto);

    // Prepare a JSON object with known values.
    nlohmann::json presetJson;
    presetJson["global"]["density"] = 0.6f;
    presetJson["global"]["minDistance"] = 0.05f;
    presetJson["global"]["pitchOffset"] = 3;
    presetJson["global"]["panOffset"] = -0.15f;
    presetJson["global"]["velocityOffset"] = 0.7f;
    presetJson["global"]["durationOffset"] = 0.8f;
    presetJson["global"]["temporalDistribution"] = static_cast<int>(Pointilism::StochasticModel::TemporalDistribution::Uniform);
    presetJson["global"]["tempoSync"] = false;
    presetJson["global"]["numVoices"] = 2;
    presetJson["global"]["numGrains"] = 75;

    std::string jsonString = presetJson.dump(4); // Serialize JSON to string

    ASSERT_TRUE(tempPresetFile.deleteFile()); // Ensure no old file.
    ASSERT_TRUE(tempPresetFile.replaceWithText(jsonString)) << "Failed to write mock JSON to temp file.";

    ASSERT_TRUE(presetManager.loadPreset(tempPresetFile)) << "loadPreset failed to execute.";

    // Verify that the model parameters match the loaded JSON values.
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalDensity(), 0.6f);
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalMinDistance(), 0.05f);
    EXPECT_EQ(modelToLoadInto.getGlobalPitchOffset(), 3);
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalPanOffset(), -0.15f);
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalVelocityOffset(), 0.7f);
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalDurationOffset(), 0.8f);
    EXPECT_EQ(modelToLoadInto.getGlobalTemporalDistribution(), Pointilism::StochasticModel::TemporalDistribution::Uniform);
    EXPECT_FALSE(modelToLoadInto.isGlobalTempoSyncEnabled());
    EXPECT_EQ(modelToLoadInto.getGlobalNumVoices(), 2);
    EXPECT_EQ(modelToLoadInto.getGlobalNumGrains(), 75);
}

// Tests if loadPreset gracefully handles JSON files where some expected global parameters are missing.
// It should update parameters found in the JSON and leave others in the model untouched (i.e., preserve their initial values).
TEST_F(PresetManagerTest, LoadPresetHandlesMissingParametersGracefully)
{
    Pointilism::StochasticModel modelToLoadInto;
    // Set some initial non-default values to ensure they are not changed if missing in JSON,
    // or to verify they are correctly overwritten if present.
    modelToLoadInto.setGlobalDensity(1.0f); // This value should remain 1.0f as it's missing in the JSON below.
    modelToLoadInto.setGlobalNumVoices(4);  // This value should be changed to 5 by the JSON.

    Pointilism::PresetManager presetManager(modelToLoadInto);

    nlohmann::json presetJson;
    // Deliberately missing "density" to test graceful handling.
    presetJson["global"]["minDistance"] = 0.05f;
    presetJson["global"]["pitchOffset"] = 3;
    // Only including a subset of parameters.
    presetJson["global"]["numVoices"] = 5;


    std::string jsonString = presetJson.dump(4);
    ASSERT_TRUE(tempPresetFile.deleteFile());
    ASSERT_TRUE(tempPresetFile.replaceWithText(jsonString));

    ASSERT_TRUE(presetManager.loadPreset(tempPresetFile));

    // Verify that parameters NOT in the JSON remain unchanged.
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalDensity(), 1.0f); // Should not have changed from its initial value.
    // Verify that parameters PRESENT in the JSON are updated.
    EXPECT_FLOAT_EQ(modelToLoadInto.getGlobalMinDistance(), 0.05f);
    EXPECT_EQ(modelToLoadInto.getGlobalPitchOffset(), 3);
    EXPECT_EQ(modelToLoadInto.getGlobalNumVoices(), 5); // Should have changed from 4 to 5.
}

// Tests if loadPreset returns false when attempting to load a file with malformed JSON content.
// Malformed JSON (e.g., syntax errors) should be detected and handled by returning false.
TEST_F(PresetManagerTest, LoadPresetFailsForMalformedJson)
{
    Pointilism::StochasticModel modelToLoadInto;
    Pointilism::PresetManager presetManager(modelToLoadInto);

    ASSERT_TRUE(tempPresetFile.deleteFile());
    // Writing content that is intentionally not valid JSON.
    ASSERT_TRUE(tempPresetFile.replaceWithText("This is not JSON { an_object: but_not_quite_right }"));

    EXPECT_FALSE(presetManager.loadPreset(tempPresetFile)) << "loadPreset should fail for malformed JSON.";
}

// Tests if loadPreset returns false when attempting to load from a file path that does not exist.
TEST_F(PresetManagerTest, LoadPresetFailsForNonExistentFile)
{
    Pointilism::StochasticModel modelToLoadInto;
    Pointilism::PresetManager presetManager(modelToLoadInto);

    // Using a unique name for a file that we ensure does not exist.
    juce::File nonExistentFile(juce::File::getSpecialLocation(juce::File::tempDirectory)
                                .getChildFile("nonExistentPreset_" + juce::String::toHexString(juce::Random().nextInt64()) + ".json"));
    ASSERT_TRUE(nonExistentFile.deleteFile()); // Ensure it really doesn't exist before the test.

    EXPECT_FALSE(presetManager.loadPreset(nonExistentFile)) << "loadPreset should fail for non-existent file.";
}

// Tests if loadPreset returns false when attempting to load an empty file.
// An empty file is not a valid JSON structure and should be handled by returning false.
TEST_F(PresetManagerTest, LoadPresetFailsForEmptyFile)
{
    Pointilism::StochasticModel modelToLoadInto;
    Pointilism::PresetManager presetManager(modelToLoadInto);

    ASSERT_TRUE(tempPresetFile.deleteFile());
    ASSERT_TRUE(tempPresetFile.create()); // Creates an empty file.

    EXPECT_FALSE(presetManager.loadPreset(tempPresetFile)) << "loadPreset should fail for an empty file.";
}

} // namespace Pointilism_Test
