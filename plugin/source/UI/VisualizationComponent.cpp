#include "UI/VisualizationComponent.h"
#include <cmath>
#include <algorithm>
#include "Pointilsynth/InertialHistoryManager.h"

namespace audio_plugin {

namespace {
inline double currentTimeSeconds() {
    return juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

juce::Colour getColorForPitch(float pitch, float velocity) {
    // Map pitch to hue (0.0-1.0)
    float hue = juce::jmap(pitch, 21.0f, 108.0f, 0.0f, 1.0f);
    // Map velocity to saturation and brightness
    float saturation = juce::jmap(velocity, 0.0f, 1.0f, 0.3f, 1.0f);
    float brightness = juce::jmap(velocity, 0.0f, 1.0f, 0.5f, 1.0f);
    return juce::Colour::fromHSV(hue, saturation, brightness, 0.8f);
}

// Preset configurations
namespace {
    const juce::ColourGradient createDefaultGradient() {
        juce::ColourGradient g;
        g.addColour(static_cast<double>(0.0f), juce::Colours::red);
        g.addColour(static_cast<double>(0.2f), juce::Colours::orange);
        g.addColour(static_cast<double>(0.4f), juce::Colours::yellow);
        g.addColour(static_cast<double>(0.6f), juce::Colours::green);
        g.addColour(static_cast<double>(0.8f), juce::Colours::blue);
        g.addColour(static_cast<double>(1.0f), juce::Colours::violet);
        return g;
    }
    
    const juce::ColourGradient createMinimalistGradient() {
        juce::ColourGradient g;
        g.addColour(static_cast<double>(0.0f), juce::Colour::fromFloatRGBA(0.9f, 0.9f, 0.9f, 0.8f));
        g.addColour(static_cast<double>(1.0f), juce::Colour::fromFloatRGBA(0.7f, 0.7f, 0.7f, 0.6f));
        return g;
    }
    
    const juce::ColourGradient createRetroGradient() {
        juce::ColourGradient g;
        g.addColour(static_cast<double>(0.0f), juce::Colours::red);
        g.addColour(static_cast<double>(0.5f), juce::Colours::yellow);
        g.addColour(static_cast<double>(1.0f), juce::Colours::lime);
        return g;
    }
}

}  // namespace

// Visual preset implementations
void VisualizationComponent::setupDefaultPreset() {
    setParticleSize(3.0f, 15.0f);
    setParticleOpacity(0.8f);
    setGravity(0.05f);
    setTrailLength(20);
    setColorScheme(createDefaultGradient());
}

void VisualizationComponent::setupMinimalistPreset() {
    setParticleSize(2.0f, 8.0f);
    setParticleOpacity(0.6f);
    setGravity(0.01f);
    setTrailLength(10);
    setColorScheme(createMinimalistGradient());
}

void VisualizationComponent::setupRetroPreset() {
    setParticleSize(4.0f, 20.0f);
    setParticleOpacity(0.9f);
    setGravity(0.1f);
    setTrailLength(5);
    setColorScheme(createRetroGradient());
}

void VisualizationComponent::setVisualPreset(VisualPreset preset) {
    currentPreset_ = preset;
    
    switch (preset) {
        case VisualPreset::Minimalist: setupMinimalistPreset(); break;
        case VisualPreset::Retro: setupRetroPreset(); break;
        case VisualPreset::ParticleWave: /* Setup particle wave */ break;
        case VisualPreset::Spectral: /* Setup spectral */ break;
        case VisualPreset::Default:
        default: setupDefaultPreset(); break;
    }
}

// Metering and visual feedback
void VisualizationComponent::setMeterValues(float left, float right) {
    leftMeterLevel_ = juce::jlimit(0.0f, 1.0f, left);
    rightMeterLevel_ = juce::jlimit(0.0f, 1.0f, right);
    
    // Update peaks if this is a new max
    leftMeterPeak_ = std::max(leftMeterPeak_, leftMeterLevel_);
    rightMeterPeak_ = std::max(rightMeterPeak_, rightMeterLevel_);
    lastPeakUpdate_ = juce::Time::currentTimeMillis();
}

void VisualizationComponent::updateMeterPeaks() {
    auto now = juce::Time::currentTimeMillis();
    if (now - lastPeakUpdate_ > 1000) { // 1 second peak hold
        leftMeterPeak_ *= 0.9f;
        rightMeterPeak_ *= 0.9f;
    }
}

void VisualizationComponent::triggerVisualFeedback(int note, float velocity) {
    VisualFeedback feedback;
    feedback.positionX = juce::jmap(float(note), 0.0f, 127.0f, 0.0f, 1.0f);
    feedback.positionY = 0.5f;
    feedback.radius = 0.0f;
    feedback.alpha = 0.8f * velocity;
    feedback.color = getColorForPitch(note, velocity);
    
    const juce::CriticalSection::ScopedLockType lock(particleLock);
    activeFeedbacks_.push_back(feedback);
}

void VisualizationComponent::updateVisualFeedbacks() {
    auto now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    (void)now; // suppress unused variable warning
    
    for (auto it = activeFeedbacks_.begin(); it != activeFeedbacks_.end(); ) {
        it->radius += 0.1f;
        it->alpha -= 0.01f;
        
        if (it->alpha <= 0.0f) {
            it = activeFeedbacks_.erase(it);
        } else {
            ++it;
        }
    }
}

void VisualizationComponent::drawVisualFeedbacks(juce::Graphics& g) {
    for (const auto& feedback : activeFeedbacks_) {
        auto color = feedback.color.withAlpha(feedback.alpha);
        g.setColour(color);
        
        float x = feedback.positionX * getWidth();
        float y = feedback.positionY * getHeight();
        float diameter = feedback.radius * 100.0f;
        
        g.drawEllipse(x - diameter/2, y - diameter/2, diameter, diameter, 2.0f);
    }
}

VisualizationComponent::VisualizationComponent(juce::AudioProcessorValueTreeState& apvts)
    : juce::Component("VisualizationComponent"),
      juce::Timer(),
      animationTime(0.0),
      apvts_(apvts),
      particleLock(),
      grains(),
      lastUpdateTime(juce::Time::getMillisecondCounterHiRes() / 1000.0) {
    setOpaque(true);
    setSize(800, 600);
    
    // Initialize graphics buffer
    particleBuffer = juce::Image(juce::Image::ARGB, 800, 600, true);
    
    // Start timer for smooth 60 FPS animation
    startTimerHz(60);
}

VisualizationComponent::~VisualizationComponent() {
    stopTimer();
}

void VisualizationComponent::updateParticles() {
    const double now = currentTimeSeconds();
    const double deltaTime = now - lastUpdateTime;
    lastUpdateTime = now;
    
    // Update existing particles
    for (auto& grain : grains) {
        // Uniform descent
        grain.y += static_cast<float>(static_cast<double>(grain.velocityY) * deltaTime);
        
        // Apply damping to horizontal velocity
        grain.velocityX *= 0.99f;
        grain.x += static_cast<float>(static_cast<double>(grain.velocityX) * deltaTime);
        
        // Calculate age-based properties for pop animation
        float age = static_cast<float>((now - grain.startTime) / grain.maxAge);
        float popThreshold = 0.8f; // Start popping when 80% of life is gone
        
        if (age < popThreshold) {
            grain.currentSize = grain.size; // No size change until pop
            grain.currentAlpha = 1.0f - juce::jlimit(0.0f, 1.0f, age * 0.5f); // Gradual fade
        } else {
            float popProgress = juce::jmap(age, popThreshold, 1.0f, 0.0f, 1.0f);
            grain.currentSize = grain.size * (1.0f + popProgress * 0.5f); // Increase size by 50%
            grain.currentAlpha = 1.0f - popProgress; // Fade out completely during pop
        }
    }
    
    // Remove dead particles
    grains.erase(std::remove_if(grains.begin(), grains.end(),
                              [now](const VisualGrain& g) {
                                  return (now - g.startTime) > g.maxAge;
                              }),
               grains.end());
    
    bufferNeedsUpdate = true;
}

void VisualizationComponent::setVisualizationFifo(juce::AbstractFifo* fifo, GrainInfoForVis* buffer) {
    visualizationFifo_ = fifo;
    visualizationBuffer_ = buffer;
}

void VisualizationComponent::addGrainInfo(const GrainInfoForVis& info) {
    const double now = currentTimeSeconds();

    VisualGrain grain;
    grain.x = juce::jmap(info.pan, -1.0f, 1.0f, 0.0f, 1.0f); // Map pan to 0.0-1.0 for visualization
    grain.y = 0.0f; // Start at the top
    grain.pitch = info.pitch;
    grain.size = 6.0f + info.velocity * 10.0f; // Size based on velocity
    grain.velocityX = (static_cast<float>(rand() % 100 - 50) * 0.0005f); // Very subtle random horizontal velocity
    grain.velocityY = 0.5f; // Constant downward velocity for proportional travel
    grain.startTime = now;
    grain.maxAge = static_cast<double>(info.durationSeconds) * 2.0; // Visual life proportional to audio duration
    grain.colour = getColorForPitch(info.pitch, info.velocity); // Use existing color function
    grain.currentSize = grain.size;
    grain.currentAlpha = 1.0f;
    
    // Add to particles with thread safety
    const juce::CriticalSection::ScopedLockType lock(particleLock);
    grains.push_back(grain);
}

void VisualizationComponent::drawParticles(juce::Graphics& g) {
    const juce::CriticalSection::ScopedLockType lock(particleLock);
    
    for (const auto& grain : grains) {
        // Convert normalized coordinates to screen coordinates
        float x = juce::jmap(grain.x, 0.0f, 1.0f, 0.0f, static_cast<float>(getWidth()));
        float y = juce::jmap(grain.y, 0.0f, 1.0f, 0.0f, static_cast<float>(getHeight()));
        
        // Set color and alpha
        g.setColour(grain.colour.withAlpha(grain.currentAlpha));
        
        // Draw particle as a circle
        float radius = grain.currentSize * 0.5f;
        g.fillEllipse(x - radius, y - radius, grain.currentSize, grain.currentSize);
        
        // Draw trails if enabled
        if (trailLength_ > 0 && !grain.trailPositions.empty()) {
            juce::Path trailPath;
            bool firstPoint = true;
            
            for (size_t i = 0; i < grain.trailPositions.size(); ++i) {
                float trailX = juce::jmap(grain.trailPositions[i].first, 0.0f, 1.0f, 0.0f, static_cast<float>(getWidth()));
                float trailY = juce::jmap(grain.trailPositions[i].second, 0.0f, 1.0f, 0.0f, static_cast<float>(getHeight()));
                
                if (firstPoint) {
                    trailPath.startNewSubPath(trailX, trailY);
                    firstPoint = false;
                } else {
                    trailPath.lineTo(trailX, trailY);
                }
            }
            
            float trailAlpha = grain.currentAlpha * 0.3f;
            g.setColour(grain.colour.withAlpha(trailAlpha));
            g.strokePath(trailPath, juce::PathStrokeType(1.0f));
        }
    }
}

void VisualizationComponent::resized() {
    // Recreate buffer with new size
    if (getWidth() > 0 && getHeight() > 0) {
        particleBuffer = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
        bufferNeedsUpdate = true;
    }
}

void VisualizationComponent::timerCallback() {
    // Read new grain info from FIFO
    if (visualizationFifo_ && visualizationBuffer_) {
        int start1, size1, start2, size2;
        visualizationFifo_->prepareToRead(visualizationFifo_->getNumReady(), start1, size1, start2, size2);
        
        for (int i = 0; i < size1; ++i) {
            addGrainInfo(visualizationBuffer_[start1 + i]);
        }
        for (int i = 0; i < size2; ++i) {
            addGrainInfo(visualizationBuffer_[start2 + i]);
        }
        
        visualizationFifo_->finishedRead(size1 + size2);
    }
    
    if (isVisible()) {
        updateParticles();
        updateVisualFeedbacks();
        updateMeterPeaks();
        repaint();
    }
}

void VisualizationComponent::paint(juce::Graphics& g) {
    // Fill background
    g.fillAll(juce::Colour(0xff1a1a1a));
    
    // Draw particles
    drawParticles(g);
    
    // Draw overlays (meters, feedback, etc.)
    drawVisualFeedbacks(g);
    drawMeters(g);
}

void VisualizationComponent::updateAnimation() {
    animationTime += 1.0 / 60.0; // Assuming 60 FPS
    repaint();
}

void VisualizationComponent::drawMeters(juce::Graphics& g) {
    if (!showMeters_) return;

    const int meterWidth = 10;
    const int meterHeight = getHeight() - 20;
    const int meterX = getWidth() - meterWidth - 10;
    const int meterY = 10;

    // Draw left meter
    g.setColour(juce::Colours::grey);
    g.drawRect(meterX - meterWidth - 5, meterY, meterWidth, meterHeight);
    g.setColour(juce::Colours::green);
    g.fillRect(meterX - meterWidth - 5, meterY + static_cast<int>(meterHeight * (1.0f - leftMeterLevel_)), meterWidth, static_cast<int>(meterHeight * leftMeterLevel_));

    // Draw right meter
    g.setColour(juce::Colours::grey);
    g.drawRect(meterX, meterY, meterWidth, meterHeight);
    g.setColour(juce::Colours::green);
    g.fillRect(meterX, meterY + static_cast<int>(meterHeight * (1.0f - rightMeterLevel_)), meterWidth, static_cast<int>(meterHeight * rightMeterLevel_));
}

void VisualizationComponent::setGravity(float newGravity) {
    gravity_ = newGravity;
}

void VisualizationComponent::setColorScheme(const juce::ColourGradient& gradient) {
    colorGradient = gradient;
}

void VisualizationComponent::setTrailLength(int numFrames) {
    trailLength_ = numFrames;
}

void VisualizationComponent::setParticleSize(float minSize, float maxSize) {
    minParticleSize_ = minSize;
    maxParticleSize_ = maxSize;
}

void VisualizationComponent::setParticleOpacity(float opacity) {
    particleOpacity_ = opacity;
}

}  // namespace audio_plugin