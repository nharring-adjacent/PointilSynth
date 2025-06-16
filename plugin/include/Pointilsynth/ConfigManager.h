#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

class ConfigManager {
public:
  struct ParamID {
    static constexpr const char* pitch = "pitch";
    static constexpr const char* dispersion = "dispersion";
    static constexpr const char* avgDuration = "avgDuration";
    static constexpr const char* durationVariation = "durationVariation";
    static constexpr const char* pan = "pan";
    static constexpr const char* panSpread = "panSpread";
    static constexpr const char* density = "density";
    static constexpr const char* temporalDistribution = "temporalDistribution";
  };

  using Callback = std::function<void(float)>;

  static std::shared_ptr<ConfigManager> getInstance(
      juce::AudioProcessor* processor = nullptr);
  static void resetInstance();
  void addListener(const juce::String& paramID, Callback cb);

  std::unique_ptr<juce::Slider> createAttachedSlider(
      const juce::String& paramID);
  std::unique_ptr<juce::ComboBox> createAttachedComboBox(
      const juce::String& paramID);
  void releaseAttachment(juce::Slider* slider);
  void releaseAttachment(juce::ComboBox* box);

private:
  juce::AudioProcessorValueTreeState& getAPVTS();
  class FunctionListener : public juce::AudioProcessorValueTreeState::Listener {
  public:
    explicit FunctionListener(Callback fn) : fn_(std::move(fn)) {}
    void parameterChanged(const juce::String&, float newValue) override {
      fn_(newValue);
    }
    Callback fn_;
  };

  explicit ConfigManager(juce::AudioProcessor* processor);
  static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

  juce::AudioProcessorValueTreeState apvts_;
  juce::AudioProcessor* owner_{nullptr};
  std::unordered_map<juce::String,
                     std::vector<std::unique_ptr<FunctionListener>>>
      listeners_;
  std::vector<std::pair<
      juce::Slider*,
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>>
      sliderAttachments_;
  std::vector<std::pair<
      juce::ComboBox*,
      std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>>
      comboBoxAttachments_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigManager)
};
