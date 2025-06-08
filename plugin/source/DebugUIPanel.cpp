#include "DebugUIPanel.h"
#include "PointilismInterfaces.h" // For StochasticModel::TemporalDistribution

// Constructor
DebugUIPanel::DebugUIPanel(StochasticModel* model) : stochasticModel(model)
{
    // Initialize and configure sliders and labels
    // Pitch
    addAndMakeVisible(pitchSlider);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    pitchSlider.setRange(20.0, 100.0, 0.1); // MIDI note numbers
    if (stochasticModel) pitchSlider.setValue(stochasticModel->getPitch(), juce::dontSendNotification);
    pitchSlider.onValueChange = [this] { pitchSliderChanged(); };
    addAndMakeVisible(pitchLabel);
    pitchLabel.setText("Pitch", juce::dontSendNotification);
    pitchLabel.attachToComponent(&pitchSlider, true); // true for onLeft

    // Dispersion
    addAndMakeVisible(dispersionSlider);
    dispersionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dispersionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    dispersionSlider.setRange(0.0, 24.0, 0.1); // Semitones
    if (stochasticModel) dispersionSlider.setValue(stochasticModel->getDispersion(), juce::dontSendNotification);
    dispersionSlider.onValueChange = [this] { dispersionSliderChanged(); };
    addAndMakeVisible(dispersionLabel);
    dispersionLabel.setText("Dispersion", juce::dontSendNotification);
    dispersionLabel.attachToComponent(&dispersionSlider, true);

    // Duration
    addAndMakeVisible(durationSlider);
    durationSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    durationSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    durationSlider.setRange(10.0, 1000.0, 1.0); // Milliseconds
    if (stochasticModel) durationSlider.setValue(stochasticModel->getAverageDurationMs(), juce::dontSendNotification);
    durationSlider.onValueChange = [this] { durationSliderChanged(); };
    addAndMakeVisible(durationLabel);
    durationLabel.setText("Duration (ms)", juce::dontSendNotification);
    durationLabel.attachToComponent(&durationSlider, true);

    // Duration Variation
    addAndMakeVisible(durationVariationSlider);
    durationVariationSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    durationVariationSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    durationVariationSlider.setRange(0.0, 1.0, 0.01); // Percentage
    if (stochasticModel) durationVariationSlider.setValue(stochasticModel->getDurationVariation(), juce::dontSendNotification);
    durationVariationSlider.onValueChange = [this] { durationVariationSliderChanged(); };
    addAndMakeVisible(durationVariationLabel);
    durationVariationLabel.setText("Duration Var.", juce::dontSendNotification);
    durationVariationLabel.attachToComponent(&durationVariationSlider, true);

    // Pan
    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    panSlider.setRange(-1.0, 1.0, 0.01); // -1 (L) to 1 (R)
    if (stochasticModel) panSlider.setValue(stochasticModel->getCentralPan(), juce::dontSendNotification);
    panSlider.onValueChange = [this] { panSliderChanged(); };
    addAndMakeVisible(panLabel);
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, true);

    // Pan Spread
    addAndMakeVisible(panSpreadSlider);
    panSpreadSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSpreadSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    panSpreadSlider.setRange(0.0, 1.0, 0.01); // Spread amount
    if (stochasticModel) panSpreadSlider.setValue(stochasticModel->getPanSpread(), juce::dontSendNotification);
    panSpreadSlider.onValueChange = [this] { panSpreadSliderChanged(); };
    addAndMakeVisible(panSpreadLabel);
    panSpreadLabel.setText("Pan Spread", juce::dontSendNotification);
    panSpreadLabel.attachToComponent(&panSpreadSlider, true);

    // Density
    addAndMakeVisible(densitySlider);
    densitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    densitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    densitySlider.setRange(0.1, 50.0, 0.1); // Grains per second
    if (stochasticModel) densitySlider.setValue(stochasticModel->getGrainsPerSecond(), juce::dontSendNotification);
    densitySlider.onValueChange = [this] { densitySliderChanged(); };
    addAndMakeVisible(densityLabel);
    densityLabel.setText("Density (gr/s)", juce::dontSendNotification);
    densityLabel.attachToComponent(&densitySlider, true);

    // Temporal Distribution
    addAndMakeVisible(temporalDistributionComboBox);
    temporalDistributionComboBox.addItem("Uniform", static_cast<int>(StochasticModel::TemporalDistribution::Uniform) + 1);
    temporalDistributionComboBox.addItem("Poisson", static_cast<int>(StochasticModel::TemporalDistribution::Poisson) + 1);
    if (stochasticModel) temporalDistributionComboBox.setSelectedId(static_cast<int>(stochasticModel->getTemporalDistributionModel()) + 1, juce::dontSendNotification);
    temporalDistributionComboBox.onChange = [this] { temporalDistributionChanged(); };
    addAndMakeVisible(temporalDistributionLabel);
    temporalDistributionLabel.setText("Distribution", juce::dontSendNotification);
    temporalDistributionLabel.attachToComponent(&temporalDistributionComboBox, true);

    // The editor is responsible for setting our size.
    // setSize(600, 400); // This was set by PluginEditor
}

DebugUIPanel::~DebugUIPanel()
{
    // Destructor (JUCE handles cleanup of child components as they are owned)
    // Listeners are lambda based and managed by JUCE Slider/ComboBox, no manual removal needed.
}

void DebugUIPanel::paint(juce::Graphics& g)
{
    // Fill the background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void DebugUIPanel::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.alignItems = juce::FlexBox::AlignItems::stretch; // Components will stretch to width
    fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    // Helper to add item with margin and fixed height
    auto addItemToFlexBox = [&](juce::Component& comp, float height) {
        // Labels are attached to the left, so they don't need separate FlexItems if they size themselves.
        // The component itself (slider/combobox) takes the full width.
        fb.items.add(juce::FlexItem(comp).withHeight(height).withMargin(juce::FlexItem::Margin(2.0f, 0, 2.0f, 0))); // Top/Bottom margin
    };

    float standardItemHeight = 25.0f; // Standard height for sliders and combobox

    // Labels are attached to the left of these components.
    // Their width is set by label.attachToComponent or can be set explicitly if needed.
    // For attached labels, their width is managed by JUCE.
    // We can set a common width for labels if desired:
    // pitchLabel.setMinimumHorizontalScale(0.8f); // Example
    // Or ensure labels have enough text space.
    // For attached labels, their width is part of the component they are attached to.
    // The FlexBox will lay out the main components (sliders, combobox).

    addItemToFlexBox(pitchSlider, standardItemHeight);
    addItemToFlexBox(dispersionSlider, standardItemHeight);
    addItemToFlexBox(durationSlider, standardItemHeight);
    addItemToFlexBox(durationVariationSlider, standardItemHeight);
    addItemToFlexBox(panSlider, standardItemHeight);
    addItemToFlexBox(panSpreadSlider, standardItemHeight);
    addItemToFlexBox(densitySlider, standardItemHeight);
    addItemToFlexBox(temporalDistributionComboBox, standardItemHeight);

    // Perform layout within the panel's bounds, reduced by a margin for aesthetics
    fb.performLayout(getLocalBounds().reduced(10));
}

// Callback methods
void DebugUIPanel::pitchSliderChanged()
{
    if (stochasticModel)
        stochasticModel->setPitchAndDispersion(pitchSlider.getValue(), dispersionSlider.getValue());
}

void DebugUIPanel::dispersionSliderChanged()
{
    if (stochasticModel)
        stochasticModel->setPitchAndDispersion(pitchSlider.getValue(), dispersionSlider.getValue());
}

void DebugUIPanel::durationSliderChanged()
{
    if (stochasticModel)
        stochasticModel->setDurationAndVariation(durationSlider.getValue(), durationVariationSlider.getValue());
}

void DebugUIPanel::durationVariationSliderChanged()
{
    if (stochasticModel)
        stochasticModel->setDurationAndVariation(durationSlider.getValue(), durationVariationSlider.getValue());
}

void DebugUIPanel::panSliderChanged()
{
    if (stochasticModel)
        stochasticModel->setPanAndSpread(panSlider.getValue(), panSpreadSlider.getValue());
}

void DebugUIPanel::panSpreadSliderChanged()
{
    if (stochasticModel)
        stochasticModel->setPanAndSpread(panSlider.getValue(), panSpreadSlider.getValue());
}

void DebugUIPanel::densitySliderChanged()
{
    if (stochasticModel)
        stochasticModel->setDensity(densitySlider.getValue());
}

void DebugUIPanel::temporalDistributionChanged()
{
    if (stochasticModel)
    {
        int selectedId = temporalDistributionComboBox.getSelectedId();
        // Enum values are 0-indexed, ComboBox IDs are 1-indexed
        if (selectedId > 0) {
             stochasticModel->setTemporalDistribution(static_cast<StochasticModel::TemporalDistribution>(selectedId - 1));
        }
    }
}
