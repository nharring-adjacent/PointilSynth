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
  pitchLabel.setText("Pitch", juce::dontSendNotification);
  pitchLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(pitchLabel);

  // Dispersion
  dispersionSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::dispersion);
  dispersionSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  dispersionSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  dispersionSlider->setRange(0.0, 24.0, 0.1);  // Semitones
  addAndMakeVisible(*dispersionSlider);
  dispersionLabel.setText("Dispersion", juce::dontSendNotification);
  dispersionLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(dispersionLabel);

  // Duration
  durationSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::avgDuration);
  durationSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  durationSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  durationSlider->setRange(10.0, 1000.0, 1.0);  // Milliseconds
  addAndMakeVisible(*durationSlider);
  durationLabel.setText("Duration (ms)", juce::dontSendNotification);
  durationLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(durationLabel);

  // Duration Variation
  durationVariationSlider = configManager->createAttachedSlider(
      ConfigManager::ParamID::durationVariation);
  durationVariationSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  durationVariationSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false,
                                           80, 20);
  durationVariationSlider->setRange(0.0, 1.0, 0.01);  // Percentage
  addAndMakeVisible(*durationVariationSlider);
  durationVariationLabel.setText("Duration Var.", juce::dontSendNotification);
  durationVariationLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(durationVariationLabel);

  // Pan
  panSlider = configManager->createAttachedSlider(ConfigManager::ParamID::pan);
  panSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  panSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  panSlider->setRange(-1.0, 1.0, 0.01);  // -1 (L) to 1 (R)
  addAndMakeVisible(*panSlider);
  panLabel.setText("Pan", juce::dontSendNotification);
  panLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(panLabel);

  // Pan Spread
  panSpreadSlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::panSpread);
  panSpreadSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  panSpreadSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  panSpreadSlider->setRange(0.0, 1.0, 0.01);  // Spread amount
  addAndMakeVisible(*panSpreadSlider);
  panSpreadLabel.setText("Pan Spread", juce::dontSendNotification);
  panSpreadLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(panSpreadLabel);

  // Density
  densitySlider =
      configManager->createAttachedSlider(ConfigManager::ParamID::density);
  densitySlider->setSliderStyle(juce::Slider::LinearHorizontal);
  densitySlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
  densitySlider->setRange(0.1, 50.0, 0.1);  // Grains per second
  addAndMakeVisible(*densitySlider);
  densityLabel.setText("Density (gr/s)", juce::dontSendNotification);
  densityLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(densityLabel);

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
  temporalDistributionLabel.setText("Distribution", juce::dontSendNotification);
  temporalDistributionLabel.setJustificationType(
      juce::Justification::centredLeft);
  addAndMakeVisible(temporalDistributionLabel);

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
  auto area = getLocalBounds().reduced(10);
  int labelWidth = 90;
  int rowHeight = 25;

  auto layoutRow = [&](juce::Component& comp, juce::Label& label) {
    auto row = area.removeFromTop(rowHeight);
    label.setBounds(row.removeFromLeft(labelWidth));
    comp.setBounds(row.reduced(2));
  };

  layoutRow(*pitchSlider, pitchLabel);
  layoutRow(*dispersionSlider, dispersionLabel);
  layoutRow(*durationSlider, durationLabel);
  layoutRow(*durationVariationSlider, durationVariationLabel);
  layoutRow(*panSlider, panLabel);
  layoutRow(*panSpreadSlider, panSpreadLabel);
  layoutRow(*densitySlider, densityLabel);
  layoutRow(*temporalDistributionComboBox, temporalDistributionLabel);
}
