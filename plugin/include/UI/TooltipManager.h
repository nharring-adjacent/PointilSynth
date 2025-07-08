#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

class TooltipManager : public juce::TooltipWindow,
                       private juce::Timer {
public:
    static TooltipManager& getInstance();
    
    // Disable copy/move
    TooltipManager(const TooltipManager&) = delete;
    TooltipManager& operator=(const TooltipManager&) = delete;
    
    // Set tooltip for a component
    void setTooltip(juce::Component* component, const juce::String& tooltip);
    
    // Remove tooltip for a component
    void removeTooltip(juce::Component* component);
    
    // Set global tooltip delay in milliseconds
    void setTooltipDelay(int milliseconds);
    
private:
    TooltipManager();
    ~TooltipManager() override;
    
    void timerCallback() override;
    
    struct TooltipInfo {
        juce::String text;
        juce::Point<int> lastMousePosition;
        bool isShowing = false;
    };
    
    juce::HashMap<juce::Component*, TooltipInfo> tooltips_;
    int delayMs_ = 700; // Default delay in milliseconds
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TooltipManager)
};

} // namespace audio_plugin
