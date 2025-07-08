#include "UI/ProbabilityWaveComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <numeric>
#include <cmath> // for std::abs

namespace audio_plugin {

ProbabilityWaveComponent::ProbabilityWaveComponent(juce::AudioProcessorValueTreeState& apvtsRef)
    : apvts(apvtsRef)
{
    // Set up parameter listeners
    apvts.addParameterListener(ConfigManager::ParamID::oscDistSine, this);
    apvts.addParameterListener(ConfigManager::ParamID::oscDistSaw, this);
    apvts.addParameterListener(ConfigManager::ParamID::oscDistSquare, this);
    apvts.addParameterListener(ConfigManager::ParamID::oscDistNoise, this);
    
    // Initialize oscillator sections with names and colors
    initializeOscillatorSections();
    
    // Initialize control points
    initializeControlPoints();
    
    // Start animation timer (30 FPS)
    startTimerHz(30);
    
    // Initial parameter sync
    suppressParameterUpdates = true;
    const char* oscParamIds[] = {
        ConfigManager::ParamID::oscDistSine,
        ConfigManager::ParamID::oscDistSaw,
        ConfigManager::ParamID::oscDistSquare,
        ConfigManager::ParamID::oscDistNoise
    };
    for (size_t i = 0; i < oscillatorSections.size(); ++i) {
        parameterChanged(oscParamIds[i], *apvts.getRawParameterValue(oscParamIds[i]));
    }
    suppressParameterUpdates = false;
    
    // Initial update of control points from parameters
    updateControlPointsFromProbabilities();
}

ProbabilityWaveComponent::~ProbabilityWaveComponent()
{
    // Remove parameter listeners
    apvts.removeParameterListener(ConfigManager::ParamID::oscDistSine, this);
    apvts.removeParameterListener(ConfigManager::ParamID::oscDistSaw, this);
    apvts.removeParameterListener(ConfigManager::ParamID::oscDistSquare, this);
    apvts.removeParameterListener(ConfigManager::ParamID::oscDistNoise, this);
}

void ProbabilityWaveComponent::paint(juce::Graphics& g)
{
    // Draw background
    g.fillAll(juce::Colour(0xff2d2d30));
    
    const auto localBounds = getLocalBounds().toFloat();
    const auto height = localBounds.getHeight();
    // Width is not currently used, so we'll comment it out to avoid unused variable warning
    // const auto width = localBounds.getWidth();
    
    // Create a path for the wave
    juce::Path wavePath;
    
    // Start at bottom-left
    wavePath.startNewSubPath(0, getHeight());
    
    // Add control points to the path
    for (const auto& point : controlPoints) {
        float x = point.x * getWidth();
        float y = getHeight() - (point.y * getHeight());
        
        // For the first point, just move to it
        if (&point == &controlPoints.front()) {
            wavePath.startNewSubPath(x, y);
        } else {
            // For subsequent points, add a line
            wavePath.lineTo(x, y);
        }
    }
    
    // Close the path at bottom-right
    wavePath.lineTo(getWidth(), getHeight());
    wavePath.closeSubPath();
    
    // Draw each oscillator section with its own color
    for (size_t i = 0; i < oscillatorSections.size(); ++i) {
        const auto& section = oscillatorSections[i];
        
        // Create a copy of the path for this section
        juce::Path sectionPath(wavePath);
        
        // Clip to section bounds
        sectionPath.setUsingNonZeroWinding(false);
        
        // Create a gradient for the fill
        juce::ColourGradient gradient(
            section.color.withAlpha(0.8f), 0.0f, 0.0f,
            section.color.darker(0.7f).withAlpha(0.6f), 0.0f, height,
            false
        );
        
        // Draw the filled section
        g.setGradientFill(gradient);
        g.fillPath(sectionPath);
        
        // Draw the section border
        g.setColour(section.color.brighter(0.3f).withAlpha(0.8f));
        g.strokePath(sectionPath, juce::PathStrokeType(1.5f));
        
        // Draw section label
        auto labelFont = juce::FontOptions(14.0f, juce::Font::bold);
        g.setFont(labelFont);
        
        juce::Rectangle<int> labelArea(
            static_cast<int>(std::round(section.bounds.getX())),
            static_cast<int>(std::round(section.bounds.getY())),
            static_cast<int>(std::round(section.bounds.getWidth())),
            static_cast<int>(std::round(section.bounds.getHeight()))
        );
        
        g.setColour(juce::Colours::white);
        g.drawText(section.name, labelArea, juce::Justification::centredTop, false);
        
        // Draw probability percentage
        juce::String percentText = juce::String(static_cast<int>(section.probability * 100.0f)) + "%";
        g.drawText(percentText, labelArea.translated(0, 15), juce::Justification::centredTop, false);
    }
    
    // Draw control points
    g.setColour(juce::Colours::white);
    for (const auto& point : controlPoints) {
        float x = point.x * getWidth();
        float y = getHeight() - (point.y * getHeight());
        
        // Only draw control points that aren't at the edges
        if (x > 0 && x < getWidth()) {
            // Draw a subtle glow
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillEllipse(x - 8, y - 8, 16, 16);
            
            // Draw the control point
            g.setColour(juce::Colours::white);
            g.fillEllipse(x - 5, y - 5, 10, 10);
            
            // Highlight the point being dragged
            if (point.isDragging) {
                g.setColour(juce::Colours::white.withAlpha(0.5f));
                g.drawEllipse(x - 7, y - 7, 14, 14, 2.0f);
            }
        }
    }
    
    // Draw a subtle grid
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    for (int i = 1; i <= 3; ++i) {
        float x = i * (getWidth() / 4.0f);
        g.drawLine(x, 0, x, getHeight());
    }
    for (int i = 1; i <= 4; ++i) {
        float y = i * (getHeight() / 5.0f);
        g.drawLine(0, y, getWidth(), y);
    }
}

void ProbabilityWaveComponent::resized()
{
    // Update section bounds based on new component size
    float sectionWidth = getWidth() / 3.0f;
    
    for (size_t i = 0; i < oscillatorSections.size(); ++i) {
        oscillatorSections[i].bounds = juce::Rectangle<float>(
            static_cast<float>(i) * sectionWidth,
            0.0f,
            sectionWidth,
            static_cast<float>(getHeight())
        );
    }
    
    // Update control point positions
    updateControlPointPositions();
}

void ProbabilityWaveComponent::timerCallback()
{
    // Update animation phase
    animationPhase += 0.05f;
    if (animationPhase > juce::MathConstants<float>::twoPi) {
        animationPhase -= juce::MathConstants<float>::twoPi;
    }
    
    // Apply subtle breathing animation to control points
    for (size_t i = 0; i < controlPoints.size(); ++i) {
        // Skip fixed edge points
        if (i == 0 || i == controlPoints.size() - 1) {
            continue;
        }
        
        // Calculate breathing amount based on position and phase
        float breathAmount = 0.01f * std::sin(animationPhase + (i * 0.5f));
        
        // Apply to y position, but keep within bounds
        controlPoints[i].y = juce::jlimit(0.1f, 0.9f, controlPointsBase[i].y + breathAmount);
    }
    
    // Trigger repaint for animation
    repaint();
}

void ProbabilityWaveComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (suppressParameterUpdates) {
        return;
    }
    
    // Store which parameter was changed for intelligent normalization
    int changedIndex = -1;
    if (parameterID == ConfigManager::ParamID::oscDistSine) {
        oscillatorSections[0].probability = newValue;
        changedIndex = 0;
    } else if (parameterID == ConfigManager::ParamID::oscDistSaw) {
        oscillatorSections[1].probability = newValue;
        changedIndex = 1;
    } else if (parameterID == ConfigManager::ParamID::oscDistSquare) {
        oscillatorSections[2].probability = newValue;
        changedIndex = 2;
    } else if (parameterID == ConfigManager::ParamID::oscDistNoise) {
        oscillatorSections[3].probability = newValue;
        changedIndex = 3;
    }
    
    // Apply normalization to ensure parameters always sum to 1.0
    normalizeAndUpdateParameters(changedIndex);
    
    // Update control points to reflect the new probabilities
    updateControlPointsFromProbabilities();
    
    // Trigger repaint
    repaint();
}

void ProbabilityWaveComponent::mouseDown(const juce::MouseEvent& e)
{
    // Check if the user clicked on a control point
    for (auto& point : controlPoints) {
        float x = point.x * getWidth();
        float y = getHeight() - (point.y * getHeight());
        
        if (e.getPosition().getDistanceFrom(juce::Point<int>(
                static_cast<int>(std::round(x)), 
                static_cast<int>(std::round(y)))) < 10.0f) {
            point.isDragging = true;
            break;
        }
    }
}

void ProbabilityWaveComponent::mouseDrag(const juce::MouseEvent& e)
{
    // Update the position of any dragging control points
    for (auto& point : controlPoints) {
        if (point.isDragging) {
            // Update y position (constrained to 0.1-0.9 to keep some wave shape)
            const float componentHeight = static_cast<float>(getHeight());
            float newY = 1.0f - (static_cast<float>(e.getPosition().getY()) / componentHeight);
            point.y = juce::jlimit(0.1f, 0.9f, newY);
            
            // Update base position for animation
            for (auto& basePoint : controlPointsBase) {
                if (std::abs(basePoint.x - point.x) < 0.0001f) {
                    basePoint.y = point.y;
                    break;
                }
            }
            
            // Recalculate the probability distribution
            updateProbabilities();
            
            // Update parameters
            commitProbabilitiesToParameters();
            
            // Trigger repaint
            repaint();
            break;
        }
    }
}

void ProbabilityWaveComponent::mouseUp(const juce::MouseEvent&)
{
    // Reset dragging flags
    bool wasDragging = false;
    for (auto& point : controlPoints) {
        if (point.isDragging) {
            point.isDragging = false;
            wasDragging = true;
        }
    }
    
    // If we were dragging, update the parameters one last time
    if (wasDragging) {
        updateProbabilities();
        commitProbabilitiesToParameters();
    }
}

void ProbabilityWaveComponent::initializeControlPoints()
{
    // Clear any existing points
    controlPoints.clear();
    controlPointsBase.clear();
    
    // Create control points with default positions
    // We'll use 5 points per section (2 at edges, 3 in the middle) for smooth curves
    for (int i = 0; i <= 4; ++i) {
        for (int j = 0; j < 5; ++j) {
            float x = (i + (j / 4.0f)) / 4.0f;
            
            // Skip duplicate points at section boundaries
            if (j > 0 && i < 4) {
                continue;
            }
            
            // Add control point
            ControlPoint point;
            point.x = x;
            point.y = 0.5f;  // Start with a flat wave at 50%
            point.isDragging = false;
            
            controlPoints.push_back(point);
            controlPointsBase.push_back(point);
        }
    }
    
    // Ensure we have at least the edge points
    if (controlPoints.empty()) {
        controlPoints.push_back({0.0f, 0.5f, false});
        controlPoints.push_back({1.0f, 0.5f, false});
        controlPointsBase = controlPoints;
    }
}

void ProbabilityWaveComponent::initializeOscillatorSections()
{
    // Initialize oscillator sections with names and colors
    oscillatorSections = {{
        {"Sine",    juce::Colour(0xffe74c3c), 0.25f, {}},  // Red
        {"Saw",     juce::Colour(0xff3498db), 0.25f, {}},  // Blue
        {"Square",  juce::Colour(0xff2ecc71), 0.25f, {}},  // Green
        {"Noise",   juce::Colour(0xff9b59b6), 0.25f, {}}   // Purple
    }};
}

void ProbabilityWaveComponent::updateControlPointsFromProbabilities()
{
    // Calculate total probability for normalization
    float total = 0.0f;
    for (const auto& section : oscillatorSections) {
        total += section.probability;
    }
    
    // If total is zero (shouldn't happen), use equal distribution
    if (total <= 0.0f) {
        for (auto& section : oscillatorSections) {
            section.probability = 0.25f;
        }
        total = 1.0f;
    }
    
    // Normalize probabilities
    for (auto& section : oscillatorSections) {
        section.probability /= total;
    }
    
    // Update control point heights based on probabilities
    // This is a simplified approach - in a real implementation, you might want
    // to calculate a smooth curve that matches the desired areas
    
    // For now, we'll just set the middle control points of each section
    // based on the probability (height = sqrt(probability) for visual area)
    for (size_t i = 0; i < oscillatorSections.size(); ++i) {
        float sectionStart = i * 0.25f;
        float sectionEnd = (i + 1) * 0.25f;
        float sectionMid = (sectionStart + sectionEnd) * 0.5f;
        
        // Find the control point closest to the middle of this section
        for (auto& point : controlPoints) {
            if (std::abs(point.x - sectionMid) < 0.05f) {
                // Set height based on probability (sqrt for visual area)
                point.y = 0.1f + 0.8f * std::sqrt(oscillatorSections[i].probability * 4.0f);
                point.y = juce::jlimit(0.1f, 0.9f, point.y);
                
                // Update base position for animation
                for (auto& basePoint : controlPointsBase) {
                    if (std::abs(basePoint.x - point.x) < 0.0001f) {
                        basePoint.y = point.y;
                        break;
                    }
                }
                
                break;
            }
        }
    }
}

void ProbabilityWaveComponent::updateProbabilities()
{
    // Calculate the area under the curve for each section
    std::array<float, 4> areas = {0.0f, 0.0f, 0.0f, 0.0f};
    float totalArea = 0.0f;
    
    // Use a fixed number of steps for numerical integration
    const int numSteps = 100;
    const float step = 1.0f / numSteps;
    
    // Calculate area under each section
    for (float x1 = 0.0f; x1 < 1.0f; x1 += step) {
        float x2 = x1 + step;
        
        // Calculate which section this x range belongs to
        // Since we have 4 sections, each section is 0.25 wide (0-0.25, 0.25-0.5, 0.5-0.75, 0.75-1.0)
        for (size_t section = 0; section < areas.size(); ++section) {
            float sectionStart = static_cast<float>(section) * 0.25f;
            float sectionEnd = sectionStart + 0.25f;
            
            // Calculate the overlap between [x1,x2] and [sectionStart,sectionEnd]
            float overlapStart = std::max(x1, sectionStart);
            float overlapEnd = std::min(x2, sectionEnd);
            
            if (overlapStart < overlapEnd) {
                // Calculate y values at the overlap boundaries
                float overlapY1 = getHeightAtPosition(overlapStart);
                float overlapY2 = getHeightAtPosition(overlapEnd);
                
                // Add the area of this segment to the corresponding section
                areas[section] += 0.5f * (overlapY1 + overlapY2) * (overlapEnd - overlapStart);
            }
        }
    }
    
    // Calculate total area
    for (const auto& area : areas) {
        totalArea += area;
    }
    
    // Normalize and update probabilities
    if (totalArea > 0.0f) {
        for (size_t i = 0; i < oscillatorSections.size() && i < areas.size(); ++i) {
            oscillatorSections[i].probability = areas[i] / totalArea;
        }
    }
}

void ProbabilityWaveComponent::commitProbabilitiesToParameters()
{
    if (suppressParameterUpdates) {
        return;
    }

    // Update the parameters with the calculated probabilities, with null checks
    auto* sineParam = apvts.getParameter(ConfigManager::ParamID::oscDistSine);
    auto* sawParam = apvts.getParameter(ConfigManager::ParamID::oscDistSaw);
    auto* squareParam = apvts.getParameter(ConfigManager::ParamID::oscDistSquare);
    auto* noiseParam = apvts.getParameter(ConfigManager::ParamID::oscDistNoise);

    if (!sineParam || !sawParam || !squareParam || !noiseParam) {
        DBG("[ProbabilityWaveComponent] One or more oscillator distribution parameters are missing in APVTS!");
        jassertfalse; // Fail in debug builds for visibility
        return;
    }

    sineParam->setValueNotifyingHost(sineParam->convertTo0to1(oscillatorSections[0].probability));
    sawParam->setValueNotifyingHost(sawParam->convertTo0to1(oscillatorSections[1].probability));
    squareParam->setValueNotifyingHost(squareParam->convertTo0to1(oscillatorSections[2].probability));
    noiseParam->setValueNotifyingHost(noiseParam->convertTo0to1(oscillatorSections[3].probability));
}

void ProbabilityWaveComponent::normalizeAndUpdateParameters(int changedIndex)
{
    // Calculate current total
    float total = 0.0f;
    for (const auto& section : oscillatorSections) {
        total += section.probability;
    }
    
    // Handle edge cases
    if (total <= 0.0f) {
        // All parameters are zero - set equal distribution
        for (auto& section : oscillatorSections) {
            section.probability = 0.25f;
        }
    } else if (changedIndex >= 0 && changedIndex < static_cast<int>(oscillatorSections.size())) {
        // A specific parameter was changed - use intelligent normalization
        size_t idx = static_cast<size_t>(changedIndex);
        float changedValue = oscillatorSections[idx].probability;
        
        if (changedValue >= 0.99f) {
            // User set a parameter to maximum - make it dominant
            for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                if (i == idx) {
                    oscillatorSections[i].probability = 1.0f;
                } else {
                    oscillatorSections[i].probability = 0.0f;
                }
            }
        } else if (changedValue <= 0.01f) {
            // User set a parameter to minimum - redistribute among others
            oscillatorSections[idx].probability = 0.0f;
            int activeCount = 0;
            
            // Count how many other parameters are non-zero
            for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                if (i != idx && oscillatorSections[i].probability > 0.01f) {
                    activeCount++;
                }
            }
            
            if (activeCount == 0) {
                // No other active parameters, distribute equally among others
                float equalShare = 1.0f / (oscillatorSections.size() - 1);
                for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                    if (i != idx) {
                        oscillatorSections[i].probability = equalShare;
                    }
                }
            } else {
                // Normalize the remaining parameters to sum to 1.0
                float otherTotal = 0.0f;
                for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                    if (i != idx) {
                        otherTotal += oscillatorSections[i].probability;
                    }
                }
                if (otherTotal > 0.0f) {
                    for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                        if (i != idx) {
                            oscillatorSections[i].probability = (oscillatorSections[i].probability / otherTotal);
                        }
                    }
                }
            }
        } else {
            // User set a parameter to an intermediate value - adjust others proportionally
            float desiredValue = changedValue;
            float remaining = 1.0f - desiredValue;
            float otherTotal = 0.0f;
            
            // Calculate total of other parameters
            for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                if (i != idx) {
                    otherTotal += oscillatorSections[i].probability;
                }
            }
            
            // Set the changed parameter to the desired value
            oscillatorSections[idx].probability = desiredValue;
            
            // Distribute the remaining probability proportionally among others
            if (otherTotal > 0.0f) {
                for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                    if (i != idx) {
                        oscillatorSections[i].probability = (oscillatorSections[i].probability / otherTotal) * remaining;
                    }
                }
            } else {
                // Other parameters were zero, distribute equally
                float equalShare = remaining / (oscillatorSections.size() - 1);
                for (size_t i = 0; i < oscillatorSections.size(); ++i) {
                    if (i != idx) {
                        oscillatorSections[i].probability = equalShare;
                    }
                }
            }
        }
    } else {
        // General normalization (no specific parameter changed)
        if (std::abs(total - 1.0f) > 0.0001f) {
            // Normalize to sum to 1.0
            for (auto& section : oscillatorSections) {
                section.probability /= total;
            }
        }
    }
    
    // Update the APVTS parameters with normalized values
    // Temporarily suppress parameter change notifications to avoid recursion
    suppressParameterUpdates = true;
    
    auto* sineParam = apvts.getParameter(ConfigManager::ParamID::oscDistSine);
    auto* sawParam = apvts.getParameter(ConfigManager::ParamID::oscDistSaw);
    auto* squareParam = apvts.getParameter(ConfigManager::ParamID::oscDistSquare);
    auto* noiseParam = apvts.getParameter(ConfigManager::ParamID::oscDistNoise);

    if (sineParam && sawParam && squareParam && noiseParam) {
        sineParam->setValueNotifyingHost(sineParam->convertTo0to1(oscillatorSections[0].probability));
        sawParam->setValueNotifyingHost(sawParam->convertTo0to1(oscillatorSections[1].probability));
        squareParam->setValueNotifyingHost(squareParam->convertTo0to1(oscillatorSections[2].probability));
        noiseParam->setValueNotifyingHost(noiseParam->convertTo0to1(oscillatorSections[3].probability));
    }
    
    suppressParameterUpdates = false;
}

float ProbabilityWaveComponent::getHeightAtPosition(float x)
{
    if (controlPoints.size() < 2) {
        return 0.5f;
    }
    
    // Find the two control points that surround x
    for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
        if (x >= controlPoints[i].x && x <= controlPoints[i + 1].x) {
            // Linear interpolation between the two points
            float t = (x - controlPoints[i].x) / (controlPoints[i + 1].x - controlPoints[i].x);
            return controlPoints[i].y * (1.0f - t) + controlPoints[i + 1].y * t;
        }
    }
    
    // If we get here, x is outside the control points range
    return 0.5f;
}

void ProbabilityWaveComponent::updateControlPointPositions()
{
    // This would be called after resizing to update any absolute positions
    // For now, we're using normalized coordinates, so nothing to do here
}

} // namespace audio_plugin
