#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PodComponent : public juce::Component
{
public:
    PodComponent();
    ~PodComponent() override;

    void setName(const juce::String& newName);
    void setValue(float newValue, juce::NotificationType notification);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

private:
    juce::String name;
    float value;
    float lastMouseY; // For mouse drag calculation
};
