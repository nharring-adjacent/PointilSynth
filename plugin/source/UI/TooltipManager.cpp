#include "UI/TooltipManager.h"

namespace audio_plugin {

TooltipManager& TooltipManager::getInstance() {
    static TooltipManager instance;
    return instance;
}

TooltipManager::TooltipManager()
    : juce::TooltipWindow(this, 50) {
    startTimerHz(30); // Check for tooltip updates 30 times per second
}

TooltipManager::~TooltipManager() {
    stopTimer();
}

void TooltipManager::setTooltip(juce::Component* component, const juce::String& tooltip) {
    if (component != nullptr) {
        tooltips_.set(component, {tooltip, {}, false});
    }
}

void TooltipManager::removeTooltip(juce::Component* component) {
    tooltips_.remove(component);
}

void TooltipManager::setTooltipDelay(int milliseconds) {
    delayMs_ = juce::jmax(0, milliseconds);
}

void TooltipManager::timerCallback() {
    auto* desktop = juce::Desktop::getInstance();
    auto* mainComponent = desktop.getMainMouseSource().getComponentUnderMouse();
    
    if (mainComponent == nullptr) {
        hideTip();
        return;
    }
    
    // Find the top-level component that has a tooltip
    juce::Component* componentWithTooltip = nullptr;
    TooltipInfo* tooltipInfo = nullptr;
    
    for (auto* current = mainComponent; current != nullptr; current = current->getParentComponent()) {
        if (tooltips_.contains(current)) {
            componentWithTooltip = current;
            tooltipInfo = &tooltips_.getReference(current);
            break;
        }
    }
    
    if (componentWithTooltip == nullptr) {
        hideTip();
        return;
    }
    
    auto mousePosition = componentWithTooltip->getMouseXYRelative();
    
    // If we have a new component or the mouse has moved significantly
    if (!tooltipInfo->isShowing || std::abs(mousePosition.getX() - tooltipInfo->lastMousePosition.getX()) > 3 ||
                                   std::abs(mousePosition.getY() - tooltipInfo->lastMousePosition.getY()) > 3) {
        
        tooltipInfo->lastMousePosition = mousePosition;
        tooltipInfo->isShowing = true;
        
        // Show the tooltip after the delay
        juce::Timer::callAfterDelay(delayMs_, [this, component = juce::Component::SafePointer(componentWithTooltip)] {
            if (component != nullptr && tooltips_.contains(component.get())) {
                auto& info = tooltips_.getReference(component.get());
                if (info.isShowing) {
                    displayTip(component->getScreenPosition() + info.lastMousePosition + juce::Point<int>(0, 20),
                              info.text);
                }
            }
        });
    }
}

} // namespace audio_plugin
