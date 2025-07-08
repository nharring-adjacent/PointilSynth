#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Pointilsynth/ConfigManager.h"

namespace audio_plugin {

/**
 * @class ProbabilityWaveComponent
 * @brief A visual control for oscillator type distribution using an interactive wave metaphor.
 * 
 * This component visualizes oscillator probabilities as sections of a wave, where the area under
 * each section corresponds to the probability of selecting that oscillator type. Users can
 * reshape the wave by dragging control points, adjusting the distribution in real-time.
 */
class ProbabilityWaveComponent : public juce::Component,
                                 public juce::Timer,
                                 public juce::AudioProcessorValueTreeState::Listener
{
public:
    /**
     * Constructor
     * @param apvts The AudioProcessorValueTreeState containing oscillator distribution parameters. Required parameter IDs are sourced from ConfigManager::ParamID::{oscDistSine, oscDistSaw, oscDistSquare, oscDistNoise}.
     */
    explicit ProbabilityWaveComponent(juce::AudioProcessorValueTreeState& apvts);

    /**
     * Destructor - removes parameter listeners
     */
    ~ProbabilityWaveComponent() override;

    /**
     * Paint the component - draws the wave, sections, and control points
     */
    void paint(juce::Graphics& g) override;

    /**
     * Handle component resize events
     */
    void resized() override;

    /**
     * Timer callback for animations
     */
    void timerCallback() override;

    /**
     * AudioProcessorValueTreeState::Listener callback for parameter changes
     */
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    /**
     * Handle mouse press events for control point interaction
     */
    void mouseDown(const juce::MouseEvent& e) override;

    /**
     * Handle mouse drag events for control point movement
     */
    void mouseDrag(const juce::MouseEvent& e) override;

    /**
     * Handle mouse release events to finalize control point positions
     */
    void mouseUp(const juce::MouseEvent& e) override;

private:
    /**
     * Control point structure for the wave shape
     */
    struct ControlPoint {
        float x;           // Normalized x position (0-1)
        float y;           // Normalized height (0-1)
        bool isDragging;   // Drag state
    };

    /**
     * Section representing an oscillator type
     */
    struct OscillatorSection {
        juce::String name;             // Oscillator type name
        juce::Colour color;            // Color matching visualization
        float probability;             // Current probability (0-1)
        juce::Rectangle<float> bounds; // Section bounds
    };

    /**
     * Initialize control points with default positions
     */
    void initializeControlPoints();

    /**
     * Initialize oscillator sections with names and colors
     */
    void initializeOscillatorSections();

    /**
     * Update control points based on current probabilities
     */
    void updateControlPointsFromProbabilities();

    /**
     * Update oscillator section probabilities based on the wave shape
     */
    void updateProbabilities();

    /**
     * Apply the calculated probabilities to the APVTS parameters
     */
    void commitProbabilitiesToParameters();

    /**
     * Normalize probabilities to sum to 1.0 and update APVTS parameters
     * @param changedIndex Index of the parameter that was changed (-1 for general normalization)
     */
    void normalizeAndUpdateParameters(int changedIndex = -1);

    /**
     * Get the wave height at a specific x position by interpolating control points
     */
    float getHeightAtPosition(float x);

    /**
     * Update the positions of control points after resizing
     */
    void updateControlPointPositions();

    // Control points and base values for animation
    std::vector<ControlPoint> controlPoints;
    std::vector<ControlPoint> controlPointsBase;

    // Oscillator sections with name, color, and probability
    std::array<OscillatorSection, 4> oscillatorSections;

    // Animation state
    float animationPhase = 0.0f;

    // Reference to APVTS for parameter access
    juce::AudioProcessorValueTreeState& apvts;

    // Prevent unnecessary parameter updates during initialization
    bool suppressParameterUpdates = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbabilityWaveComponent)
};

} // namespace audio_plugin
