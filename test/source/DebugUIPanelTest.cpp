#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h> // For ValueTree
#include "DebugUIPanel.h"
#include "PointilismInterfaces.h"
#include "StochasticModel.h" // Required for StochasticModel implementation

// Helper struct to manage the model and panel for tests
// Assumes StochasticModel default constructor provides consistent initial values for all its parameters.
struct PanelTestFixture
{
    std::unique_ptr<StochasticModel> stochasticModel;
    std::unique_ptr<DebugUIPanel> debugUIPanel;
    juce::AudioProcessorValueTreeState apvts; // Mock APVTS if needed, or use a real one

    // Constructor for the fixture
    PanelTestFixture()
        : stochasticModel(std::make_unique<StochasticModel>()),
          // This is a simplified APVTS for testing.
          // In a real scenario, you'd likely need parameters similar to the plugin's APVTS.
          apvts(*static_cast<juce::AudioProcessor*>(nullptr), nullptr, "TestParams", createTestParameterLayout())
    {
        // Initialize the model with APVTS (if it expects one)
        // stochasticModel->initialize(apvts); // Assuming an initialize method or similar
                                            // Or if the model takes APVTS in constructor, adjust accordingly.
                                            // For now, assuming StochasticModel default constructor is sufficient
                                            // and it can operate without full APVTS initialization for these tests,
                                            // or that DebugUIPanel interaction doesn't strictly require fully init'd APVTS params.

        // Pass the model to the DebugUIPanel
        // Assumes DebugUIPanel constructor correctly initializes its components based on the model's state.
        debugUIPanel = std::make_unique<DebugUIPanel>(stochasticModel.get());
    }

    // Helper to create a basic parameter layout for the mock APVTS
    // This might need to be more sophisticated depending on StochasticModel and DebugUIPanel needs
    static juce::AudioProcessorValueTreeState::ParameterLayout createTestParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        // Add parameters that StochasticModel and DebugUIPanel might interact with via APVTS
        layout.add(std::make_unique<juce::AudioParameterFloat>("pitch", "Pitch", juce::NormalisableRange<float>(0.0f, 127.0f), 60.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("dispersion", "Dispersion", juce::NormalisableRange<float>(0.0f, 100.0f), 10.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("duration", "Duration", juce::NormalisableRange<float>(0.01f, 10.0f), 1.0f)); // Example range, adjust if known
        layout.add(std::make_unique<juce::AudioParameterFloat>("durationVariation", "DurationVariation", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("pan", "Pan", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("panSpread", "PanSpread", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("density", "Density", juce::NormalisableRange<float>(0.1f, 100.0f), 50.0f)); // Example range, adjust if known
        // layout.add(std::make_unique<juce::AudioParameterChoice>("temporalDistribution", "TemporalDistribution", juce::StringArray{"Uniform", "Poisson"}, 0));
        return layout;
    }
};


class DebugUIPanelTests : public juce::UnitTest
{
public:
    DebugUIPanelTests() : juce::UnitTest("DebugUIPanel Tests") {}

    void runTest() override
    {
        // --- Test Pitch Slider ---
        beginTest("Pitch Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newPitch = 60.0f;
            // This test assumes that pitchSliderChanged in DebugUIPanel calls
            // stochasticModel->setPitchAndDispersion(newPitch, currentDispersion),
            // where currentDispersion is taken from the model's state at that moment.
            fixture.debugUIPanel->pitchSlider.setValue(newPitch, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getPitch(), newPitch, "Model pitch should update after UI slider change.");
        }

        beginTest("Pitch Slider: Model to UI (Post-Construction Update)");
        {
            PanelTestFixture fixture;
            const float modelPitch = 72.0f;
            // This test assumes StochasticModel's setPitchAndDispersion also updates DebugUIPanel
            // (e.g., via a listener pattern or APVTS update that the panel observes).
            // The dispersion value used is the model's current (default) dispersion.
            fixture.stochasticModel->setPitchAndDispersion(modelPitch, fixture.stochasticModel->getDispersion());
            expectEquals(static_cast<float>(fixture.debugUIPanel->pitchSlider.getValue()), modelPitch, "UI pitch slider should reflect model change after model is updated.");
        }

        // --- Test Dispersion Slider ---
        beginTest("Dispersion Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newDispersion = 12.0f;
            // Assumes dispersionSliderChanged calls stochasticModel->setPitchAndDispersion(currentPitch, newDispersion).
            fixture.debugUIPanel->dispersionSlider.setValue(newDispersion, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getDispersion(), newDispersion, "Model dispersion should update after UI slider change.");
        }

        beginTest("Dispersion Slider: Model to UI (Post-Construction Update)");
        {
            PanelTestFixture fixture;
            const float modelDispersion = 6.0f;
            // Assumes model update notifies panel. Pitch value used is model's current (default) pitch.
            fixture.stochasticModel->setPitchAndDispersion(fixture.stochasticModel->getPitch(), modelDispersion);
            expectEquals(static_cast<float>(fixture.debugUIPanel->dispersionSlider.getValue()), modelDispersion, "UI dispersion slider should reflect model change after model is updated.");
        }

        // --- Test Duration Slider ---
        beginTest("Duration Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newDuration = 500.0f; // ms
            // Assumes durationSliderChanged calls stochasticModel->setDurationAndVariation(newDuration, currentVariation).
            fixture.debugUIPanel->durationSlider.setValue(newDuration, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getAverageDurationMs(), newDuration, "Model average duration (ms) should update after UI slider change.");
        }

        beginTest("Duration Slider: Model to UI (Initialization)");
        {
            StochasticModel tempModel; // Uses default-constructed model state.
            const float testDuration = 350.0f; // ms
            float currentVariation = tempModel.getDurationVariation(); // Preserve model's default variation.
            tempModel.setDurationAndVariation(testDuration, currentVariation);

            DebugUIPanel tempPanel(&tempModel); // Panel should initialize to this model state.
            expectEquals(static_cast<float>(tempPanel.durationSlider.getValue()), testDuration, "UI duration slider should be initialized to model's duration value.");
        }

        // --- Test Duration Variation Slider ---
        beginTest("Duration Variation Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newVariation = 0.5f;
            // Assumes durationVariationSliderChanged calls stochasticModel->setDurationAndVariation(currentDuration, newVariation).
            fixture.debugUIPanel->durationVariationSlider.setValue(newVariation, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getDurationVariation(), newVariation, "Model duration variation should update after UI slider change.");
        }

        beginTest("Duration Variation Slider: Model to UI (Initialization)");
        {
            StochasticModel tempModel2;
            const float testVariation = 0.75f;
            float currentDuration = tempModel2.getAverageDurationMs(); // Preserve model's default duration.
            tempModel2.setDurationAndVariation(currentDuration, testVariation);

            DebugUIPanel tempPanel2(&tempModel2);
            expectEquals(static_cast<float>(tempPanel2.durationVariationSlider.getValue()), testVariation, "UI duration variation slider should be initialized to model's variation value.");
        }

        // --- Test Pan Slider ---
        beginTest("Pan Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newPan = -0.5f;
            // Assumes panSliderChanged calls stochasticModel->setPanAndSpread(newPan, currentSpread).
            fixture.debugUIPanel->panSlider.setValue(newPan, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getCentralPan(), newPan, "Model central pan should update after UI slider change.");
        }

        beginTest("Pan Slider: Model to UI (Initialization)");
        {
            StochasticModel tempModel;
            const float testPan = 0.25f;
            float currentSpread = tempModel.getPanSpread(); // Preserve model's default spread.
            tempModel.setPanAndSpread(testPan, currentSpread);

            DebugUIPanel tempPanel(&tempModel);
            expectEquals(static_cast<float>(tempPanel.panSlider.getValue()), testPan, "UI pan slider should be initialized to model's pan value.");
        }

        // --- Test Pan Spread Slider ---
        beginTest("Pan Spread Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newSpread = 0.8f;
            // Assumes panSpreadSliderChanged calls stochasticModel->setPanAndSpread(currentPan, newSpread).
            fixture.debugUIPanel->panSpreadSlider.setValue(newSpread, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getPanSpread(), newSpread, "Model pan spread should update after UI slider change.");
        }

        beginTest("Pan Spread Slider: Model to UI (Initialization)");
        {
            StochasticModel tempModel2;
            const float testSpread = 0.3f;
            float currentPan = tempModel2.getCentralPan(); // Preserve model's default pan.
            tempModel2.setPanAndSpread(currentPan, testSpread);

            DebugUIPanel tempPanel2(&tempModel2);
            expectEquals(static_cast<float>(tempPanel2.panSpreadSlider.getValue()), testSpread, "UI pan spread slider should be initialized to model's spread value.");
        }

        // --- Test Density Slider ---
        beginTest("Density Slider: UI to Model");
        {
            PanelTestFixture fixture;
            const float newDensity = 25.0f; // grains per second
            // Assumes densitySliderChanged calls stochasticModel->setDensity(newDensity).
            fixture.debugUIPanel->densitySlider.setValue(newDensity, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getGrainsPerSecond(), newDensity, "Model grains per second should update after UI slider change.");
        }

        beginTest("Density Slider: Model to UI (Initialization)");
        {
            StochasticModel tempModel;
            const float testDensity = 15.0f; // grains per second
            tempModel.setDensity(testDensity);

            DebugUIPanel tempPanel(&tempModel);
            expectEquals(static_cast<float>(tempPanel.densitySlider.getValue()), testDensity, "UI density slider should be initialized to model's density value.");
        }

        // --- Test Temporal Distribution ComboBox ---
        // Note: ComboBox item IDs are 1-based, while enum values are typically 0-based.
        beginTest("Temporal Distribution ComboBox: UI to Model - Uniform");
        {
            PanelTestFixture fixture;
            int uniformItemId = static_cast<int>(StochasticModel::TemporalDistribution::Uniform) + 1;
            fixture.debugUIPanel->temporalDistributionComboBox.setSelectedId(uniformItemId, juce::sendNotificationSync);
            // Using expect() for direct enum comparison for clarity. Could also cast to int and use expectEquals().
            expect(fixture.stochasticModel->getTemporalDistributionModel() == StochasticModel::TemporalDistribution::Uniform, "Model temporal distribution should be Uniform after UI change.");
        }

        beginTest("Temporal Distribution ComboBox: UI to Model - Poisson");
        {
            PanelTestFixture fixture;
            int poissonItemId = static_cast<int>(StochasticModel::TemporalDistribution::Poisson) + 1;
            fixture.debugUIPanel->temporalDistributionComboBox.setSelectedId(poissonItemId, juce::sendNotificationSync);
            expect(fixture.stochasticModel->getTemporalDistributionModel() == StochasticModel::TemporalDistribution::Poisson, "Model temporal distribution should be Poisson after UI change.");
        }

        beginTest("Temporal Distribution ComboBox: Model to UI (Initialization) - Uniform");
        {
            StochasticModel tempModelUniform;
            tempModelUniform.setTemporalDistribution(StochasticModel::TemporalDistribution::Uniform);

            DebugUIPanel tempPanelUniform(&tempModelUniform);
            int expectedUniformId = static_cast<int>(StochasticModel::TemporalDistribution::Uniform) + 1;
            expectEquals(tempPanelUniform.temporalDistributionComboBox.getSelectedId(), expectedUniformId, "UI ComboBox should be initialized to Uniform from model state.");
        }

        beginTest("Temporal Distribution ComboBox: Model to UI (Initialization) - Poisson");
        {
            StochasticModel tempModelPoisson;
            tempModelPoisson.setTemporalDistribution(StochasticModel::TemporalDistribution::Poisson);

            DebugUIPanel tempPanelPoisson(&tempModelPoisson);
            int expectedPoissonId = static_cast<int>(StochasticModel::TemporalDistribution::Poisson) + 1;
            expectEquals(tempPanelPoisson.temporalDistributionComboBox.getSelectedId(), expectedPoissonId, "UI ComboBox should be initialized to Poisson from model state.");
        }

        // --- Edge Case Tests (UI to Model) ---
        // These tests verify that sliders correctly send their min/max values to the model.

        // Pitch Slider Edge Cases
        beginTest("Pitch Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->pitchSlider.getRange();
            double minPitch = range.getStart();
            fixture.debugUIPanel->pitchSlider.setValue(minPitch, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getPitch(), static_cast<float>(minPitch), "Model pitch should be at min edge.");
        }
        beginTest("Pitch Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->pitchSlider.getRange();
            double maxPitch = range.getEnd();
            fixture.debugUIPanel->pitchSlider.setValue(maxPitch, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getPitch(), static_cast<float>(maxPitch), "Model pitch should be at max edge.");
        }

        // Dispersion Slider Edge Cases
        beginTest("Dispersion Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->dispersionSlider.getRange();
            double minDispersion = range.getStart();
            fixture.debugUIPanel->dispersionSlider.setValue(minDispersion, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getDispersion(), static_cast<float>(minDispersion), "Model dispersion should be at min edge.");
        }
        beginTest("Dispersion Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->dispersionSlider.getRange();
            double maxDispersion = range.getEnd();
            fixture.debugUIPanel->dispersionSlider.setValue(maxDispersion, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getDispersion(), static_cast<float>(maxDispersion), "Model dispersion should be at max edge.");
        }

        // Duration Slider Edge Cases
        beginTest("Duration Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->durationSlider.getRange();
            double minDuration = range.getStart();
            fixture.debugUIPanel->durationSlider.setValue(minDuration, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getAverageDurationMs(), static_cast<float>(minDuration), "Model duration should be at min edge.");
        }
        beginTest("Duration Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->durationSlider.getRange();
            double maxDuration = range.getEnd();
            fixture.debugUIPanel->durationSlider.setValue(maxDuration, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getAverageDurationMs(), static_cast<float>(maxDuration), "Model duration should be at max edge.");
        }

        // Duration Variation Slider Edge Cases
        beginTest("Duration Variation Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->durationVariationSlider.getRange();
            double minVariation = range.getStart();
            fixture.debugUIPanel->durationVariationSlider.setValue(minVariation, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getDurationVariation(), static_cast<float>(minVariation), "Model duration variation should be at min edge.");
        }
        beginTest("Duration Variation Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->durationVariationSlider.getRange();
            double maxVariation = range.getEnd();
            fixture.debugUIPanel->durationVariationSlider.setValue(maxVariation, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getDurationVariation(), static_cast<float>(maxVariation), "Model duration variation should be at max edge.");
        }

        // Pan Slider Edge Cases
        beginTest("Pan Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->panSlider.getRange();
            double minPan = range.getStart();
            fixture.debugUIPanel->panSlider.setValue(minPan, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getCentralPan(), static_cast<float>(minPan), "Model pan should be at min edge.");
        }
        beginTest("Pan Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->panSlider.getRange();
            double maxPan = range.getEnd();
            fixture.debugUIPanel->panSlider.setValue(maxPan, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getCentralPan(), static_cast<float>(maxPan), "Model pan should be at max edge.");
        }

        // Pan Spread Slider Edge Cases
        beginTest("Pan Spread Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->panSpreadSlider.getRange();
            double minSpread = range.getStart();
            fixture.debugUIPanel->panSpreadSlider.setValue(minSpread, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getPanSpread(), static_cast<float>(minSpread), "Model pan spread should be at min edge.");
        }
        beginTest("Pan Spread Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->panSpreadSlider.getRange();
            double maxSpread = range.getEnd();
            fixture.debugUIPanel->panSpreadSlider.setValue(maxSpread, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getPanSpread(), static_cast<float>(maxSpread), "Model pan spread should be at max edge.");
        }

        // Density Slider Edge Cases
        beginTest("Density Slider: Edge Case - Min Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->densitySlider.getRange();
            double minDensity = range.getStart();
            fixture.debugUIPanel->densitySlider.setValue(minDensity, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getGrainsPerSecond(), static_cast<float>(minDensity), "Model density should be at min edge.");
        }
        beginTest("Density Slider: Edge Case - Max Value");
        {
            PanelTestFixture fixture;
            auto range = fixture.debugUIPanel->densitySlider.getRange();
            double maxDensity = range.getEnd();
            fixture.debugUIPanel->densitySlider.setValue(maxDensity, juce::sendNotificationSync);
            expectEquals(fixture.stochasticModel->getGrainsPerSecond(), static_cast<float>(maxDensity), "Model density should be at max edge.");
        }
    }
};

// This creates an instance of the test class, making it runnable
static DebugUIPanelTests debugUIPanelTests;
