#include "VisualizationComponent.h"

//==============================================================================
VisualizationComponent::VisualizationComponent()
{
    // Initialize OpenGL context and attach it to this component
    openGLContext.attachTo (*this);

    // Start the timer to update the visualization at approximately 60 FPS
    startTimerHz (60);
}

VisualizationComponent::~VisualizationComponent()
{
    // Detach the OpenGL context when the component is destroyed
    openGLContext.detach();
}

//==============================================================================
void VisualizationComponent::paint (juce::Graphics& g)
{
    // Clear the background
    g.fillAll (juce::Colours::black);

    // Iterate through the mock grains and draw them
    for (const auto& grain : mockGrains)
    {
        g.setColour (grain.colour);
        // The grain's x and y are normalized (0-1), so we scale them to the component's width and height
        float screenX = grain.x * getWidth();
        float screenY = (1.0f - grain.y) * getHeight(); // Y is inverted for screen coordinates
        float diameter = grain.size * 20.0f; // Scale size for visibility

        // Ensure diameter is at least 1.0f to be visible
        if (diameter < 1.0f) diameter = 1.0f;

        g.fillEllipse (screenX - diameter / 2, screenY - diameter / 2, diameter, diameter);
    }
}

void VisualizationComponent::resized()
{
    // This method is called when the component's size is changed.
    // You might want to update your OpenGL viewport or other size-related properties here.
    // For now, we'll leave it empty as the paint method handles scaling.
}

//==============================================================================
void VisualizationComponent::timerCallback()
{
    // Maximum number of grains to display
    const int maxGrains = 200;
    // Probability of adding a new grain (e.g., 0.5 means 50% chance per callback)
    const float addGrainProbability = 0.7f;
    // Probability of removing an existing grain
    const float removeGrainProbability = 0.1f;
    // Probability of a grain's properties changing
    const float changeGrainProbability = 0.3f;

    // Add new grains
    if (mockGrains.size() < maxGrains && random.nextFloat() < addGrainProbability)
    {
        MockGrain newGrain;
        newGrain.x = random.nextFloat(); // Position X (0.0 to 1.0, representing pan)
        newGrain.y = random.nextFloat(); // Position Y (0.0 to 1.0, representing pitch)
        newGrain.size = random.nextFloat() * 0.05f + 0.005f; // Size (0.005 to 0.055)
        newGrain.colour = juce::Colour (random.nextFloat(), random.nextFloat(), random.nextFloat(), 0.8f); // Random RGB, 80% alpha
        mockGrains.push_back (newGrain);
    }

    // Remove grains
    if (!mockGrains.empty() && random.nextFloat() < removeGrainProbability)
    {
        mockGrains.erase (mockGrains.begin() + random.nextInt (mockGrains.size()));
    }

    // Modify existing grains
    for (auto& grain : mockGrains)
    {
        if (random.nextFloat() < changeGrainProbability)
        {
            // Slightly change position
            grain.x += (random.nextFloat() - 0.5f) * 0.05f; // Small change in X
            grain.y += (random.nextFloat() - 0.5f) * 0.05f; // Small change in Y

            // Clamp positions to be within 0.0 - 1.0
            grain.x = juce::jlimit (0.0f, 1.0f, grain.x);
            grain.y = juce::jlimit (0.0f, 1.0f, grain.y);

            // Change size slightly
            grain.size += (random.nextFloat() - 0.5f) * 0.005f;
            grain.size = juce::jlimit (0.005f, 0.055f, grain.size);

            // Change color slightly
            juce::uint8 r = grain.colour.getRed();
            juce::uint8 g = grain.colour.getGreen();
            juce::uint8 b = grain.colour.getBlue();
            r += (random.nextInt(51) - 25); // change by -25 to +25
            g += (random.nextInt(51) - 25);
            b += (random.nextInt(51) - 25);
            grain.colour = juce::Colour(r,g,b).withAlpha(0.8f);
        }
    }

    // Trigger a repaint to show the changes
    repaint();
}
