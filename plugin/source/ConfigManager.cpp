#include "Pointilsynth/ConfigManager.h"
#include <algorithm>

namespace {
static std::shared_ptr<ConfigManager> instance;
}

std::shared_ptr<ConfigManager> ConfigManager::getInstance(
    juce::AudioProcessor* processor) {
  if (!instance && processor != nullptr)
    instance = std::shared_ptr<ConfigManager>(new ConfigManager(processor));
  else if (instance && processor != nullptr && instance->owner_ != processor)
    instance = std::shared_ptr<ConfigManager>(new ConfigManager(processor));
  return instance;
}

void ConfigManager::resetInstance() {
  instance.reset();
}

juce::AudioProcessorValueTreeState& ConfigManager::getAPVTS() {
  return apvts_;
}

std::unique_ptr<juce::Slider> ConfigManager::createAttachedSlider(
    const juce::String& paramID) {
  auto slider = std::make_unique<juce::Slider>();
  slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
  slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  auto attachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          apvts_, paramID, *slider);
  sliderAttachments_.emplace_back(slider.get(), std::move(attachment));
  return slider;
}

std::unique_ptr<juce::ComboBox> ConfigManager::createAttachedComboBox(
    const juce::String& paramID) {
  auto box = std::make_unique<juce::ComboBox>();
  auto attachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          apvts_, paramID, *box);
  comboBoxAttachments_.emplace_back(box.get(), std::move(attachment));
  return box;
}

void ConfigManager::releaseAttachment(juce::Slider* slider) {
  sliderAttachments_.erase(
      std::remove_if(sliderAttachments_.begin(), sliderAttachments_.end(),
                     [slider](const auto& att) { return att.first == slider; }),
      sliderAttachments_.end());
}

void ConfigManager::releaseAttachment(juce::ComboBox* box) {
  comboBoxAttachments_.erase(
      std::remove_if(comboBoxAttachments_.begin(), comboBoxAttachments_.end(),
                     [box](const auto& att) { return att.first == box; }),
      comboBoxAttachments_.end());
}

void ConfigManager::addListener(const juce::String& paramID, Callback cb) {
  auto listener = std::make_unique<FunctionListener>(std::move(cb));
  apvts_.addParameterListener(paramID, listener.get());
  listeners_[paramID].push_back(std::move(listener));
}

ConfigManager::ConfigManager(juce::AudioProcessor* processor)
    : apvts_(*processor, nullptr, "PARAMETERS", createLayout()),
      owner_(processor) {}

juce::AudioProcessorValueTreeState::ParameterLayout
ConfigManager::createLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::pitch, "Pitch", 20.0f, 100.0f, 60.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::dispersion, "Dispersion", 0.0f, 24.0f, 12.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::avgDuration, "AvgDuration", 10.0f, 1000.0f, 200.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::durationVariation, "DurationVariation", 0.0f, 1.0f, 0.25f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::pan, "Pan", -1.0f, 1.0f, 0.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::panSpread, "PanSpread", 0.0f, 1.0f, 0.5f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParamID::density, "Density", 0.1f, 50.0f, 10.0f));
  params.push_back(std::make_unique<juce::AudioParameterChoice>(
      ParamID::temporalDistribution, "TemporalDistribution",
      juce::StringArray{"Uniform", "Poisson"}, 0));
  return {params.begin(), params.end()};
}
