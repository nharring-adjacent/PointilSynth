#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/*
    This component implements the real-time visualization of audio grains.
    It uses JUCE's OpenGL rendering capabilities for high performance and
    includes a mock data generator to simulate grain activity.
*/
class VisualizationComponent  : public juce::Component,
                                public juce::Timer
{
public:
    //==============================================================================
    VisualizationComponent();
    ~VisualizationComponent() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    //==============================================================================
    struct MockGrain
    {
        float x, y, size;
        juce::Colour colour;
    };

    juce::OpenGLContext openGLContext;
    std::vector<MockGrain> mockGrains;

    // This object is used to generate random numbers
    juce::Random random;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizationComponent)
};
