#include "PodComponent.h"

PodComponent::PodComponent() : value(0.0f), lastMouseY(0.0f)
{
    // Constructor implementation
}

PodComponent::~PodComponent()
{
    // Destructor implementation
}

void PodComponent::setName(const juce::String& newName)
{
    name = newName;
    repaint(); // Repaint to reflect the new name
}

void PodComponent::setValue(float newValue, juce::NotificationType notification)
{
    if (value != newValue)
    {
        value = newValue;
        repaint(); // Repaint to reflect the new value

        if (notification == juce::sendNotificationAsync)
        {
            // Handle asynchronous notification if needed
        }
    }
}

void PodComponent::paint(juce::Graphics& g)
{
    // Set color to a dark grey
    g.fillAll(juce::Colour(0xff303030)); // Dark grey background

    // Set text color to white
    g.setColour(juce::Colours::white);

    // Draw the name of the pod
    g.setFont(juce::Font(16.0f));
    g.drawText(name, getLocalBounds().reduced(10, 0), juce::Justification::centredTop, true);

    // Draw the value of the pod
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    juce::String valueText = juce::String(value, 2); // Format value to 2 decimal places
    g.drawText(valueText, getLocalBounds().reduced(0, 10), juce::Justification::centredBottom, true);

    // Basic knob drawing (a simple circle)
    float knobRadius = juce::jmin(getWidth(), getHeight()) / 4.0f;
    juce::Point<float> centre(getWidth() / 2.0f, getHeight() / 2.0f);

    // Set knob color to a lighter grey
    g.setColour(juce::Colour(0xff505050)); // Lighter grey for the knob
    g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    // Indicator for the knob's value
    float angle = juce::jmap(value, 0.0f, 1.0f, -juce::MathConstants<float>::pi * 0.75f, juce::MathConstants<float>::pi * 0.75f);
    juce::Point<float> indicatorPos(centre.x + knobRadius * std::sin(angle), centre.y - knobRadius * std::cos(angle));

    // Set indicator color to a bright color (e.g., light blue)
    g.setColour(juce::Colour(0xff70c0ff)); // Light blue for the indicator
    g.drawLine(centre.x, centre.y, indicatorPos.x, indicatorPos.y, 2.0f);
}

void PodComponent::mouseDown(const juce::MouseEvent& event)
{
    lastMouseY = event.position.y;
}

void PodComponent::mouseDrag(const juce::MouseEvent& event)
{
    float diffY = event.position.y - lastMouseY;
    float newValue = value - diffY * 0.01f; // Adjust sensitivity as needed
    newValue = juce::jlimit(0.0f, 1.0f, newValue); // Clamp value between 0 and 1

    setValue(newValue, juce::sendNotificationAsync); // Update value and notify
    lastMouseY = event.position.y;
}
