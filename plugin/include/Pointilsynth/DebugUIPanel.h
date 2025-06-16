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

  // Parameter attachments (declared after the components they reference so that
  // they are destroyed first).
  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  using ComboBoxAttachment =
      juce::AudioProcessorValueTreeState::ComboBoxAttachment;

  std::unique_ptr<SliderAttachment> pitchAttachment;
  std::unique_ptr<SliderAttachment> dispersionAttachment;
  std::unique_ptr<SliderAttachment> durationAttachment;
  std::unique_ptr<SliderAttachment> durationVariationAttachment;
  std::unique_ptr<SliderAttachment> panAttachment;
  std::unique_ptr<SliderAttachment> panSpreadAttachment;
  std::unique_ptr<SliderAttachment> densityAttachment;
  std::unique_ptr<ComboBoxAttachment> temporalDistributionAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugUIPanel)
};
