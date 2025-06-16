#include "Pointilsynth/DebugUIPanel.h"
#include "Pointilsynth/PointilismInterfaces.h"  // For StochasticModel::TemporalDistribution
#include "Pointilsynth/ConfigManager.h"

// Constructor
DebugUIPanel::DebugUIPanel(std::shared_ptr<ConfigManager> cfg)
    : configManager(std::move(cfg)) {
  // Initialize and configure sliders and labels
  // Pitch
  pitchSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::pitch);
  pitchSlider->setRange(20.0, 100.0, 0.1);  // MIDI note numbers
  pitchSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  pitchSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  addAndMakeVisible(*pitchSlider);
  addAndMakeVisible(pitchLabel);
  pitchLabel.setText("Pitch", juce::dontSendNotification);
  pitchLabel.attachToComponent(pitchSlider.get(), true);  // true for onLeft

  // Dispersion
  dispersionSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::dispersion);
  dispersionSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  dispersionSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  dispersionSlider->setRange(0.0, 24.0, 0.1);  // Semitones
  addAndMakeVisible(*dispersionSlider);
  addAndMakeVisible(dispersionLabel);
  dispersionLabel.setText("Dispersion", juce::dontSendNotification);
  dispersionLabel.attachToComponent(dispersionSlider.get(), true);

  // Duration
  durationSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::avgDuration);
  durationSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  durationSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  durationSlider->setRange(10.0, 1000.0, 1.0);  // Milliseconds
  addAndMakeVisible(*durationSlider);
  addAndMakeVisible(durationLabel);
  durationLabel.setText("Duration (ms)", juce::dontSendNotification);
  durationLabel.attachToComponent(durationSlider.get(), true);

  // Duration Variation
  durationVariationSlider = configManager->createAttachedSlider(
      ConfigManager::ParamID::durationVariation);
  durationVariationSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  durationVariationSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false,
                                           80, 20);
  durationVariationSlider->setRange(0.0, 1.0, 0.01);  // Percentage
  addAndMakeVisible(*durationVariationSlider);
  addAndMakeVisible(durationVariationLabel);
  durationVariationLabel.setText("Duration Var.", juce::dontSendNotification);
  durationVariationLabel.attachToComponent(durationVariationSlider.get(), true);

  // Pan
  panSlider = configManager->createAttachedSlider(ConfigManager::ParamID::pan);
  panSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  panSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  panSlider->setRange(-1.0, 1.0, 0.01);  // -1 (L) to 1 (R)
  addAndMakeVisible(*panSlider);
  addAndMakeVisible(panLabel);
  panLabel.setText("Pan", juce::dontSendNotification);
  panLabel.attachToComponent(panSlider.get(), true);

  // Pan Spread
  panSpreadSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::panSpread);
  panSpreadSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  panSpreadSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  panSpreadSlider->setRange(0.0, 1.0, 0.01);  // Spread amount
  addAndMakeVisible(*panSpreadSlider);
  addAndMakeVisible(panSpreadLabel);
  panSpreadLabel.setText("Pan Spread", juce::dontSendNotification);
  panSpreadLabel.attachToComponent(panSpreadSlider.get(), true);

  // Density
  densitySlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::density);
  densitySlider->setSliderStyle(juce::Slider::LinearHorizontal);
  densitySlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  densitySlider->setRange(0.1, 50.0, 0.1);  // Grains per second
  addAndMakeVisible(*densitySlider);
  addAndMakeVisible(densityLabel);
  densityLabel.setText("Density (gr/s)", juce::dontSendNotification);
  densityLabel.attachToComponent(densitySlider.get(), true);

  // Temporal Distribution
  temporalDistributionComboBox = configManager->createAttachedComboBox(
      ConfigManager::ParamID::temporalDistribution);
  addAndMakeVisible(*temporalDistributionComboBox);
  temporalDistributionComboBox->addItem(
      "Uniform",
      static_cast<int>(StochasticModel::TemporalDistribution::Uniform) + 1);
  temporalDistributionComboBox->addItem(
      "Poisson",
      static_cast<int>(StochasticModel::TemporalDistribution::Poisson) + 1);
  // Attachment created by ConfigManager
  addAndMakeVisible(temporalDistributionLabel);
  temporalDistributionLabel.setText("Distribution", juce::dontSendNotification);
  temporalDistributionLabel.attachToComponent(
      temporalDistributionComboBox.get(), true);

  // The editor is responsible for setting our size.
  // setSize(600, 400); // This was set by PluginEditor
}

DebugUIPanel::~DebugUIPanel() {
  // Destructor (JUCE handles cleanup of child components as they are owned)
  // Listeners are lambda based and managed by JUCE Slider/ComboBox, no manual
  // removal needed.
  if (configManager) {
    configManager->releaseAttachment(pitchSlider.get());
    configManager->releaseAttachment(dispersionSlider.get());
    configManager->releaseAttachment(durationSlider.get());
    configManager->releaseAttachment(durationVariationSlider.get());
    configManager->releaseAttachment(panSlider.get());
    configManager->releaseAttachment(panSpreadSlider.get());
    configManager->releaseAttachment(densitySlider.get());
    configManager->releaseAttachment(temporalDistributionComboBox.get());
  }
}

void DebugUIPanel::paint(juce::Graphics& g) {
  // Fill the background
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void DebugUIPanel::resized() {
  juce::FlexBox fb;
  fb.flexDirection = juce::FlexBox::Direction::column;
  fb.alignItems =
      juce::FlexBox::AlignItems::stretch;  // Components will stretch to width
  fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;

  // Helper to add item with margin and fixed height
  auto addItemToFlexBox = [&](juce::Component& comp, float height) {
    // Labels are attached to the left, so they don't need separate FlexItems if
    // they size themselves. The component itself (slider/combobox) takes the
    // full width.
    fb.items.add(juce::FlexItem(comp).withHeight(height).withMargin(
        juce::FlexItem::Margin(2.0f, 0, 2.0f, 0)));  // Top/Bottom margin
  };

  float standardItemHeight = 25.0f;  // Standard height for sliders and combobox

  // Labels are attached to the left of these components.
  // Their width is set by label.attachToComponent or can be set explicitly if
  // needed. For attached labels, their width is managed by JUCE. We can set a
  // common width for labels if desired:
  // pitchLabel.setMinimumHorizontalScale(0.8f); // Example
  // Or ensure labels have enough text space.
  // For attached labels, their width is part of the component they are attached
  // to. The FlexBox will lay out the main components (sliders, combobox).

  addItemToFlexBox(*pitchSlider, standardItemHeight);
  addItemToFlexBox(*dispersionSlider, standardItemHeight);
  addItemToFlexBox(*durationSlider, standardItemHeight);
  addItemToFlexBox(*durationVariationSlider, standardItemHeight);
  addItemToFlexBox(*panSlider, standardItemHeight);
  addItemToFlexBox(*panSpreadSlider, standardItemHeight);
  addItemToFlexBox(*densitySlider, standardItemHeight);
  addItemToFlexBox(*temporalDistributionComboBox, standardItemHeight);

  // Perform layout within the panel's bounds, reduced by a margin for
  // aesthetics
  fb.performLayout(getLocalBounds().reduced(10));
}
