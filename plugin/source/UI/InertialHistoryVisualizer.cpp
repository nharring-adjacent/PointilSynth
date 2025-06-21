#include "UI/InertialHistoryVisualizer.h"

#include <cmath>

namespace audio_plugin {

InertialHistoryVisualizer::InertialHistoryVisualizer(
    InertialHistoryManager& manager)
    : manager_(manager) {
  startTimerHz(30);
}

void InertialHistoryVisualizer::timerCallback() {
  const size_t noteCount = manager_.getNumNotes();
  if (histories_.size() < noteCount)
    histories_.resize(noteCount);

  for (size_t i = 0; i < noteCount; ++i) {
    const auto& note = manager_.getNote(i);
    const float value = std::sin(static_cast<float>(note.ageInBars) *
                                 juce::MathConstants<float>::twoPi) *
                        note.currentInfluence;
    auto& hist = histories_[i];
    hist.push_back(value);
    if (static_cast<int>(hist.size()) > maxPoints_)
      hist.pop_front();
  }
  for (size_t i = noteCount; i < histories_.size(); ++i) {
    auto& hist = histories_[i];
    hist.push_back(0.0f);
    if (static_cast<int>(hist.size()) > maxPoints_)
      hist.pop_front();
  }

  repaint();
}

void InertialHistoryVisualizer::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
  const int width = getWidth();
  const int height = getHeight();

  if (histories_.empty())
    return;

  const size_t historyPoints = histories_.front().size();
  if (historyPoints < 2)
    return;

  std::vector<float> baseline(historyPoints, 0.0f);
  const float scale = static_cast<float>(height) / 4.0f;
  const float xStep =
      static_cast<float>(width) / static_cast<float>(maxPoints_ - 1);

  for (size_t i = 0; i < histories_.size(); ++i) {
    const auto& hist = histories_[i];
    if (hist.empty())
      continue;

    juce::Path p;
    for (size_t j = 0; j < historyPoints; ++j) {
      baseline[j] += hist[j];
      const float x = static_cast<float>(j) * xStep;
      const float y = static_cast<float>(height) / 2.0f - baseline[j] * scale;
      if (j == 0)
        p.startNewSubPath(x, y);
      else
        p.lineTo(x, y);
    }
    juce::Colour colour = juce::Colour::fromHSV(
        static_cast<float>(i) / static_cast<float>(histories_.size()), 0.7f,
        0.9f, 1.0f);
    g.setColour(colour);
    g.strokePath(p, juce::PathStrokeType(1.0f));
  }
}

}  // namespace audio_plugin
