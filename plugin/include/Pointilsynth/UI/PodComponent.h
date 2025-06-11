#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

class PodComponent : public juce::Component {
public:
    PodComponent() {}
    ~PodComponent() override {}
    void setName(const juce::String& newName) override;

    void paint(juce::Graphics& g) override {
        // Placeholder: fill with a color to see it
        g.fillAll(juce::Colours::blue.withAlpha(0.5f));
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds(), static_cast<int>(1.f));
        g.drawText("Pod", getLocalBounds(), juce::Justification::centred, false);
    }

    void resized() override {
        // In a real component, you'd layout child components here
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PodComponent)
};

} // namespace audio_plugin
