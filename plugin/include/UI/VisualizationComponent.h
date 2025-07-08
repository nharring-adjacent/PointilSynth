#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>
#include <atomic>
#include "Pointilsynth/PointilismInterfaces.h"
#include "Pointilsynth/InertialHistoryManager.h"

// Forward declaration
class InertialHistoryManager;

namespace audio_plugin {

class VisualizationComponent : public juce::Component,
                              public juce::Timer {
public:
    explicit VisualizationComponent(juce::AudioProcessorValueTreeState& apvts);
    ~VisualizationComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Animation and rendering
    void updateAnimation();

    // Visual presets
    enum class VisualPreset {
        Default,
        Minimalist,
        Retro,
        ParticleWave,
        Spectral
    };

    // Visual settings
    void setParticleSize(float minSize, float maxSize);
    void setParticleOpacity(float opacity);
    void setGravity(float gravity);
    void setTrailLength(int numFrames);
    void setColorScheme(const juce::ColourGradient& gradient);
    void setVisualPreset(VisualPreset preset);
    
    // Metering
    void setShowMeters(bool show) { showMeters_ = show; }
    void setMeterValues(float left, float right);
    
    // Set the InertialHistoryManager to visualize
    void setInertialHistoryManager(const InertialHistoryManager* manager) {
        const juce::CriticalSection::ScopedLockType lock(particleLock);
        inertialHistoryManager_ = manager;
    }
    
    // Visual feedback
    void triggerVisualFeedback(int note, float velocity);

    // Add grain info for visualization
    void addGrainInfo(const GrainInfoForVis& info);
    
    // Connect to visualization FIFO
    void setVisualizationFifo(juce::AbstractFifo* fifo, GrainInfoForVis* buffer);

private:
    // Animation state
    double animationTime{0.0};
    
    // APVTS reference
    juce::AudioProcessorValueTreeState& apvts_;

    struct VisualGrain {
        // Position and movement
        float x{};            // -1.0 to 1.0 (pan)
        float y{};            // 0.0 to 1.0 (pitch)
        float velocityX{};     // pixels per second
        float velocityY{};     // pixels per second
        
        // Visual properties
        float size{};          // base size in pixels
        float currentSize{};   // current size (may be affected by age/effects)
        float currentAlpha{};  // current alpha (for fade in/out)
        juce::Colour colour;   // base color
        
        // Lifecycle
        double startTime{};    // seconds
        double maxAge{};       // seconds
        int trailLength{0};    // number of previous positions to track
        std::vector<std::pair<float, float>> trailPositions; // trail history
        
        // Audio properties (for visualization)
        float pitch{60.0f};    // MIDI note number
        float velocity{1.0f};   // 0.0 to 1.0
    };

    // Graphics resources
    void updateParticles();
    void drawParticles(juce::Graphics& g);
    
    
    // Graphics state
    juce::Image particleBuffer;
    bool bufferNeedsUpdate{true};
    
    // Thread safety
    juce::CriticalSection particleLock;
    
    // Particle system state
    std::vector<VisualGrain> grains;
    float gravity{98.1f};  // pixels per second^2
    float minParticleSize{2.0f};
    float maxParticleSize{20.0f};
    float particleOpacity{0.8f};
    int trailLength{0};
    
    // Audio connection
    const InertialHistoryManager* inertialHistoryManager_{nullptr};
    
    // Timing
    double lastUpdateTime{0.0};
    
    
    // Visual state
    VisualPreset currentPreset_{VisualPreset::Default};
    bool showMeters_{true};
    float leftMeterLevel_{0.0f};
    float rightMeterLevel_{0.0f};
    float leftMeterPeak_{0.0f};
    float rightMeterPeak_{0.0f};
    juce::int64 lastPeakUpdate_{0};
    
    // Visual feedback
    struct VisualFeedback {
        float positionX{0.0f};
        float positionY{0.0f};
        float radius{0.0f};
        float alpha{0.0f};
        juce::Colour color{juce::Colours::white};
    };
    std::vector<VisualFeedback> activeFeedbacks_;
    
    // Preset configurations
    void applyPreset(VisualPreset preset);
    void setupDefaultPreset();
    void setupMinimalistPreset();
    void setupRetroPreset();
    void setupParticleWavePreset();
    void setupSpectralPreset();
    
    // Meter drawing
    void drawMeters(juce::Graphics& g);
    void updateMeterPeaks();
    
    // Visual feedback
    void updateVisualFeedbacks();
    void drawVisualFeedbacks(juce::Graphics& g);
    
    // Drawing methods
    void drawNoteHistory();
    void drawNoteConnections(const InertialNote& note);
    void drawParticleTrail(const VisualGrain& grain);
    
    // Performance optimization
    juce::int64 lastBufferUpdate{0};
    
        // Color management
    juce::ColourGradient colorGradient;

    // Particle properties
    float minParticleSize_ = 2.0f;
    float maxParticleSize_ = 20.0f;
    float particleOpacity_ = 0.8f;
    float gravity_ = 98.1f;
    int trailLength_ = 0;
    
    // Visualization FIFO connection
    juce::AbstractFifo* visualizationFifo_ = nullptr;
    GrainInfoForVis* visualizationBuffer_ = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizationComponent)
};

}  // namespace audio_plugin
