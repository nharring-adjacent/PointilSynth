#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PointilismInterfaces.h"  // Assuming this is the correct path
#include "ConfigManager.h"

class DebugUIPanel : public juce::Component {
public:
  // Constructor that takes a ConfigManager
  explicit DebugUIPanel(std::shared_ptr<ConfigManager> cfg);
  ~DebugUIPanel() override;

  // juce::Component overrides
  void paint(juce::Graphics& g) override;
  void resized() override;

private:
  // Config manager
  std::shared_ptr<ConfigManager> configManager;

  // UI Components
  std::unique_ptr<juce::Slider> pitchSlider;
  juce::Label pitchLabel;
  std::unique_ptr<juce::Slider> dispersionSlider;
  juce::Label dispersionLabel;
  std::unique_ptr<juce::Slider> durationSlider;
  juce::Label durationLabel;
  std::unique_ptr<juce::Slider> durationVariationSlider;
  juce::Label durationVariationLabel;
  std::unique_ptr<juce::Slider> panSlider;
  juce::Label panLabel;
  std::unique_ptr<juce::Slider> panSpreadSlider;
  juce::Label panSpreadLabel;
  std::unique_ptr<juce::Slider> densitySlider;
  juce::Label densityLabel;

  std::unique_ptr<juce::ComboBox> temporalDistributionComboBox;
  juce::Label temporalDistributionLabel;

  // Parameter attachments (declared after the components they reference so that
  // they are destroyed first).
  using ComboBoxAttachment =
      juce::AudioProcessorValueTreeState::ComboBoxAttachment;

  std::unique_ptr<ComboBoxAttachment> temporalDistributionAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugUIPanel)
};
