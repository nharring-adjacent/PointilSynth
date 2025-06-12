// IMPORTANT: The include for DebugUIPanel.cpp has been removed as we are using friend class.

#include "Pointilsynth/DebugUIPanel.h" // This should be included AFTER DebugUIPanel.cpp if that strategy was used. Now it's fine.
#include "Pointilsynth/PointilismInterfaces.h" // For StochasticModel
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h> // Should be included by DebugUIPanel.h or .cpp, but good to have

// Test fixture for DebugUIPanel tests
class DebugUIPanelTestFixture : public ::testing::Test {
protected:
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;
    StochasticModel model;
    std::unique_ptr<DebugUIPanel> panel;

    DebugUIPanelTestFixture() {
        // Initialization order matters: model first, then panel.
    }

    void SetUp() override {
        // Default panel creation, tests that need specific model state *before* panel construction
        // should call `createPanelWithModelSetup` or `resetPanel` themselves.
        panel = std::make_unique<DebugUIPanel>(&model);
    }

    void TearDown() override {
        panel.reset();
    }

    // Helper to re-create panel if model is modified for init tests
    void resetPanel() {
        panel.reset(); // Ensure old panel is destructed first
        panel = std::make_unique<DebugUIPanel>(&model);
    }
};

// --- Initialization Tests ---

TEST_F(DebugUIPanelTestFixture, PitchSliderInitializesCorrectly) {
    float testPitch = 0.75f;
    model.setPitchAndDispersion(testPitch, 0.25f);
    resetPanel(); // Recreate panel with new model state
    ASSERT_EQ(panel->pitchSlider.getValue(), testPitch);
}

TEST_F(DebugUIPanelTestFixture, DispersionSliderInitializesCorrectly) {
    float testDispersion = 0.65f;
    model.setPitchAndDispersion(0.5f, testDispersion);
    resetPanel();
    ASSERT_EQ(panel->dispersionSlider.getValue(), testDispersion);
}

TEST_F(DebugUIPanelTestFixture, DurationSliderInitializesCorrectly) {
    float testDuration = 150.0f; // Assuming model uses ms
    model.setDurationAndVariation(testDuration, 0.2f);
    resetPanel();
    ASSERT_EQ(panel->durationSlider.getValue(), testDuration);
}

TEST_F(DebugUIPanelTestFixture, DurationVariationSliderInitializesCorrectly) {
    float testVariation = 0.35f;
    model.setDurationAndVariation(200.0f, testVariation);
    resetPanel();
    ASSERT_EQ(panel->durationVariationSlider.getValue(), testVariation);
}

TEST_F(DebugUIPanelTestFixture, PanSliderInitializesCorrectly) {
    float testPan = -0.45f; // Assuming -1 to 1 range
    model.setPanAndSpread(testPan, 0.5f);
    resetPanel();
    ASSERT_EQ(panel->panSlider.getValue(), testPan);
}

TEST_F(DebugUIPanelTestFixture, PanSpreadSliderInitializesCorrectly) {
    float testSpread = 0.7f;
    model.setPanAndSpread(0.0f, testSpread);
    resetPanel();
    ASSERT_EQ(panel->panSpreadSlider.getValue(), testSpread);
}

TEST_F(DebugUIPanelTestFixture, DensitySliderInitializesCorrectly) {
    float testDensity = 25.0f;
    model.setGlobalDensity(testDensity);
    resetPanel();
    ASSERT_EQ(panel->densitySlider.getValue(), testDensity);
}

TEST_F(DebugUIPanelTestFixture, TemporalDistributionComboBoxInitializesCorrectly) {
    auto testDistribution = StochasticModel::TemporalDistribution::Poisson;
    model.setGlobalTemporalDistribution(testDistribution);
    resetPanel();
    // ComboBox IDs are 1-based
    ASSERT_EQ(panel->temporalDistributionComboBox.getSelectedId(), static_cast<int>(testDistribution) + 1);
}

// --- Callback Tests ---

TEST_F(DebugUIPanelTestFixture, PitchSliderCallbackUpdatesModel) {
    float testPitch = 0.8f;
    // Set initial known value for the other slider involved in the callback
    float initialDispersion = 0.3f;
    panel->dispersionSlider.setValue(initialDispersion);
    // Ensure model matches this initial dispersion before testing pitch change
    model.setPitchAndDispersion(panel->pitchSlider.getValue(), initialDispersion);
    resetPanel(); // reflect initial setup
    panel->dispersionSlider.setValue(initialDispersion); // setValue again as panel was reset

    panel->pitchSlider.setValue(testPitch);
    panel->pitchSliderChanged(); // Call the private callback

    ASSERT_EQ(model.getPitch(), testPitch);
    ASSERT_EQ(model.getDispersion(), initialDispersion); // Should remain unchanged by pitch slider
}

TEST_F(DebugUIPanelTestFixture, DispersionSliderCallbackUpdatesModel) {
    float testDispersion = 0.75f;
    // Set initial known value for the other slider involved in the callback
    float initialPitch = panel->pitchSlider.getValue(); // Use current value from panel
    panel->pitchSlider.setValue(initialPitch); // Ensure it's set if not default
    model.setPitchAndDispersion(initialPitch, panel->dispersionSlider.getValue());
    resetPanel();
    panel->pitchSlider.setValue(initialPitch);


    panel->dispersionSlider.setValue(testDispersion);
    panel->dispersionSliderChanged();

    ASSERT_EQ(model.getDispersion(), testDispersion);
    ASSERT_EQ(model.getPitch(), initialPitch); // Should remain unchanged by dispersion slider
}

TEST_F(DebugUIPanelTestFixture, DurationSliderCallbackUpdatesModel) {
    float testDuration = 250.0f;
    float initialVariation = panel->durationVariationSlider.getValue();
    panel->durationVariationSlider.setValue(initialVariation);
    model.setDurationAndVariation(panel->durationSlider.getValue(), initialVariation);
    resetPanel();
    panel->durationVariationSlider.setValue(initialVariation);


    panel->durationSlider.setValue(testDuration);
    panel->durationSliderChanged();

    ASSERT_EQ(model.getAverageDurationMs(), testDuration);
    ASSERT_EQ(model.getDurationVariation(), initialVariation);
}

TEST_F(DebugUIPanelTestFixture, DurationVariationSliderCallbackUpdatesModel) {
    float testVariation = 0.45f;
    float initialDuration = panel->durationSlider.getValue();
    panel->durationSlider.setValue(initialDuration);
    model.setDurationAndVariation(initialDuration, panel->durationVariationSlider.getValue());
    resetPanel();
    panel->durationSlider.setValue(initialDuration);


    panel->durationVariationSlider.setValue(testVariation);
    panel->durationVariationSliderChanged();

    ASSERT_EQ(model.getDurationVariation(), testVariation);
    ASSERT_EQ(model.getAverageDurationMs(), initialDuration);
}

TEST_F(DebugUIPanelTestFixture, PanSliderCallbackUpdatesModel) {
    float testPan = 0.33f;
    float initialSpread = panel->panSpreadSlider.getValue();
    panel->panSpreadSlider.setValue(initialSpread);
    model.setPanAndSpread(panel->panSlider.getValue(), initialSpread);
    resetPanel();
    panel->panSpreadSlider.setValue(initialSpread);

    panel->panSlider.setValue(testPan);
    panel->panSliderChanged();

    ASSERT_EQ(model.getCentralPan(), testPan);
    ASSERT_EQ(model.getPanSpread(), initialSpread);
}

TEST_F(DebugUIPanelTestFixture, PanSpreadSliderCallbackUpdatesModel) {
    float testSpread = 0.88f;
    float initialPan = panel->panSlider.getValue();
    panel->panSlider.setValue(initialPan);
    model.setPanAndSpread(initialPan, panel->panSpreadSlider.getValue());
    resetPanel();
    panel->panSlider.setValue(initialPan);

    panel->panSpreadSlider.setValue(testSpread);
    panel->panSpreadSliderChanged();

    ASSERT_EQ(model.getPanSpread(), testSpread);
    ASSERT_EQ(model.getCentralPan(), initialPan);
}

TEST_F(DebugUIPanelTestFixture, DensitySliderCallbackUpdatesModel) {
    float testDensity = 77.0f;
    panel->densitySlider.setValue(testDensity);
    panel->densitySliderChanged();
    ASSERT_EQ(model.getGlobalDensity(), testDensity);
}

TEST_F(DebugUIPanelTestFixture, TemporalDistributionComboBoxCallbackUpdatesModel) {
    auto testDistribution = StochasticModel::TemporalDistribution::Poisson;
    // ComboBox IDs are 1-based
    panel->temporalDistributionComboBox.setSelectedId(static_cast<int>(testDistribution) + 1);
    panel->temporalDistributionChanged();
    ASSERT_EQ(model.getGlobalTemporalDistribution(), testDistribution);

    auto testDistribution2 = StochasticModel::TemporalDistribution::Uniform;
    panel->temporalDistributionComboBox.setSelectedId(static_cast<int>(testDistribution2) + 1);
    panel->temporalDistributionChanged();
    ASSERT_EQ(model.getGlobalTemporalDistribution(), testDistribution2);
}

// Original simple construction test (can be removed if fixture covers it sufficiently)
TEST(DebugUIPanelTest, CanConstruct) {
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;
    StochasticModel model;
    EXPECT_NO_THROW(std::make_unique<DebugUIPanel>(&model));
}
