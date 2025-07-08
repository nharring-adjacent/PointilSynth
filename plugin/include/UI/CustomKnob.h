#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

#include <functional>

namespace audio_plugin {

class CustomKnob : public juce::Slider {
public:
    CustomKnob(const juce::String& name = {}) {
        setWantsKeyboardFocus(false);
        setMouseClickGrabsKeyboardFocus(false);

        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x20ffffff));
        setName(name);
    }

    std::function<void()> onClick;

    void mouseDown(const juce::MouseEvent& event) override {
        if (onClick) {
            onClick();
        }
        juce::Slider::mouseDown(event);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Draw track
        g.setColour(findColour(juce::Slider::trackColourId));
        auto trackWidth = 4.0f;
        auto trackBounds = bounds.reduced(10.0f);
        auto trackRect = juce::Rectangle<float>(trackWidth, trackWidth)
            .withCentre(trackBounds.getCentre())
            .withY(trackBounds.getY());
        
        // Draw track background
        g.setColour(findColour(juce::Slider::backgroundColourId));
        g.fillEllipse(trackBounds.reduced(2.0f));
        
        // Draw track outline
        g.setColour(findColour(juce::Slider::trackColourId).withAlpha(0.3f));
        g.drawEllipse(trackBounds.reduced(2.0f), 1.0f);
        
        // Draw value arc
        auto angleRange = juce::MathConstants<float>::pi * 1.2f;
        auto startAngle = juce::MathConstants<float>::pi + (juce::MathConstants<float>::pi - angleRange) * 0.5f;
        auto endAngle = startAngle + angleRange * (float)proportionOfWidthToLength(0.0, 1.0);
        
        juce::Path path;
        path.addArc(trackBounds.getX(), trackBounds.getY(),
                   trackBounds.getWidth(), trackBounds.getHeight(),
                   startAngle, endAngle, true);
        
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.strokePath(path, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Draw thumb
        auto thumbBounds = trackBounds.reduced(8.0f);
        auto thumbAngle = startAngle + angleRange * (float)proportionOfWidthToLength(0.0, 1.0);
        auto thumbX = thumbBounds.getCentreX() + thumbBounds.getWidth() * 0.4f * std::cos(thumbAngle - juce::MathConstants<float>::halfPi);
        auto thumbY = thumbBounds.getCentreY() + thumbBounds.getHeight() * 0.4f * std::sin(thumbAngle - juce::MathConstants<float>::halfPi);
        
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbX - 5.0f, thumbY - 5.0f, 10.0f, 10.0f);
        
        // Draw name
        if (getName().isNotEmpty()) {
            g.setColour(findColour(juce::Slider::textBoxTextColourId));
            g.setFont(12.0f);
            g.drawText(getName(), 0, getHeight() - 30, getWidth(), 20, juce::Justification::centred);
        }
    }
};

} // namespace audio_plugin
