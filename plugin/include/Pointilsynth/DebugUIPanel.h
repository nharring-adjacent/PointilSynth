#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PointilismInterfaces.h" // Assuming this is the correct path

class DebugUIPanel : public juce::Component
{
public:
    // Constructor that takes a pointer to the StochasticModel
    DebugUIPanel(StochasticModel* model);
    ~DebugUIPanel() override;

    // juce::Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Pointer to the audio engine's stochastic model
    StochasticModel* stochasticModel;

    // UI Components
    juce::Slider pitchSlider;
    juce::Label pitchLabel;
    juce::Slider dispersionSlider;
    juce::Label dispersionLabel;

    juce::Slider durationSlider;
    juce::Label durationLabel;
    juce::Slider durationVariationSlider;
    juce::Label durationVariationLabel;

    juce::Slider panSlider;
    juce::Label panLabel;
    juce::Slider panSpreadSlider;
    juce::Label panSpreadLabel;

    juce::Slider densitySlider;
    juce::Label densityLabel;

    juce::ComboBox temporalDistributionComboBox;
    juce::Label temporalDistributionLabel;

    // Callback methods for UI components
    void pitchSliderChanged();
    void dispersionSliderChanged();
    void durationSliderChanged();
    void durationVariationSliderChanged();
    void panSliderChanged();
    void panSpreadSliderChanged();
    void densitySliderChanged();
    void temporalDistributionChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugUIPanel)

    // Friend declaration for test fixture
    friend class DebugUIPanelTestFixture;
};
