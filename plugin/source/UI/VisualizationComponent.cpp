#include "UI/VisualizationComponent.h"

namespace audio_plugin {

namespace {
inline double currentTimeSeconds() {
  return juce::Time::getMillisecondCounterHiRes() / 1000.0;
}
}  // namespace

VisualizationComponent::VisualizationComponent(juce::AbstractFifo& fifo,
                                               GrainInfoForVis* buffer)
    : fifo_(fifo), buffer_(buffer) {
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
                              [now](const VisualGrain& g) {
                                return now - g.startTime > g.maxAge;
                              }),
               grains.end());
  int start1, size1, start2, size2;
  while (true) {
    fifo_.prepareToRead(1, start1, size1, start2, size2);
    if (size1 == 0)
      break;
    GrainInfoForVis info = buffer_[start1];
    fifo_.finishedRead(size1);
    VisualGrain g;
    g.pan = info.pan;
    g.pitch = info.pitch;
    g.size = 4.0f;
    g.colour = juce::Colours::white;
    g.startTime = now;
    g.maxAge = static_cast<double>(info.durationSeconds);
    grains.push_back(g);
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
