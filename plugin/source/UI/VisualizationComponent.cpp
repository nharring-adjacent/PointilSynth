#include "UI/VisualizationComponent.h"

#include <juce_graphics/juce_graphics.h>
#include <random>

namespace audio_plugin {

namespace {
inline double getCurrentTimeInSeconds() {
  return juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

float randomRange(float min, float max) {
  return juce::Random::getSystemRandom().nextFloat() * (max - min) + min;
}
}  // namespace

VisualizationComponent::VisualizationComponent() {
  openGLContext_.attachTo(*this);
  startTimerHz(60);
}

VisualizationComponent::~VisualizationComponent() {
  openGLContext_.detach();
}

void VisualizationComponent::timerCallback() {
  const double now = getCurrentTimeInSeconds();

  grains_.erase(std::remove_if(grains_.begin(), grains_.end(),
                               [now](const MockGrain& g) {
                                 return now - g.startTime > g.maxAge;
                               }),
                grains_.end());

  if (grains_.size() < 256) {
    int numNew = juce::Random::getSystemRandom().nextInt(5);  // 0-4
    for (int i = 0; i < numNew && grains_.size() < 256; ++i) {
      MockGrain g;
      g.pan = randomRange(-1.0f, 1.0f);
      g.pitch = randomRange(21.0f, 108.0f);
      g.size = randomRange(2.0f, 6.0f);
      g.colour =
          juce::Colour::fromHSV(randomRange(0.0f, 1.0f), 0.8f, 1.0f, 1.0f);
      g.startTime = now;
      g.maxAge = randomRange(1.0f, 4.0f);
      grains_.push_back(g);
    }
  }

  repaint();
}

void VisualizationComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  const auto width = static_cast<float>(getWidth());
  const auto height = static_cast<float>(getHeight());

  for (const auto& grain : grains_) {
    const float x = juce::jmap(grain.pan, -1.0f, 1.0f, 0.0f, width);
    const float y = juce::jmap(grain.pitch, 108.0f, 21.0f, 0.0f, height);

    g.setColour(grain.colour);
    g.fillEllipse(x - grain.size * 0.5f, y - grain.size * 0.5f, grain.size,
                  grain.size);
  }
}

}  // namespace audio_plugin
