#include "UI/VisualizationComponent.h"

namespace audio_plugin {

namespace {
inline double currentTimeSeconds() {
  return juce::Time::getMillisecondCounterHiRes() / 1000.0;
}
}  // namespace

VisualizationComponent::VisualizationComponent() {
  openGLContext.attachTo(*this);
  startTimerHz(60);  // 60 FPS
}

VisualizationComponent::~VisualizationComponent() {
  stopTimer();
  openGLContext.detach();
}

void VisualizationComponent::timerCallback() {
  const double now = currentTimeSeconds();

  grains.erase(std::remove_if(grains.begin(), grains.end(),
                              [now](const MockGrain& g) {
                                return now - g.startTime > g.maxAge;
                              }),
               grains.end());

  juce::Random r;
  if (grains.size() < 256) {
    int toAdd = r.nextInt(5);  // 0-4 grains
    for (int i = 0; i < toAdd && grains.size() < 256; ++i) {
      MockGrain g;
      g.pan = r.nextFloat() * 2.0f - 1.0f;
      g.pitch = 21.0f + r.nextFloat() * (108.0f - 21.0f);
      g.size = 2.0f + r.nextFloat() * 4.0f;
      float hue = r.nextFloat();
      float sat = 0.8f + r.nextFloat() * 0.2f;
      float val = 0.8f + r.nextFloat() * 0.2f;
      g.colour = juce::Colour::fromHSV(hue, sat, val, 1.0f);
      g.startTime = now;
      g.maxAge = 1.0 + static_cast<double>(r.nextFloat()) * 3.0;
      grains.push_back(g);
    }
  }

  repaint();
}

void VisualizationComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  const int width = getWidth();
  const int height = getHeight();

  for (const auto& grain : grains) {
    float x =
        juce::jmap(grain.pan, -1.0f, 1.0f, 0.0f, static_cast<float>(width));
    float y = juce::jmap(grain.pitch, 108.0f, 21.0f, 0.0f,
                         static_cast<float>(height));
    float diameter = grain.size * 2.0f;
    g.setColour(grain.colour);
    g.fillEllipse(x - grain.size, y - grain.size, diameter, diameter);
  }
}

}  // namespace audio_plugin
