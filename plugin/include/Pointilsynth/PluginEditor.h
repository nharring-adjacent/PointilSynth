#pragma once

#include "PluginProcessor.h"  // Adjusted path
#include "DebugUIPanel.h"     // Added for DebugUIPanel
#include "PodComponent.h"     // Placeholder pod controls
#include "UI/VisualizationComponent.h"
#include <juce_core/juce_core.h>
// PointilismInterfaces.h is included by PluginProcessor.h, which is included
// above. If direct use of StochasticModel type was needed here, an include
// would be good: #include "../../source/PointilismInterfaces.h"

namespace audio_plugin {

class PointillisticSynthAudioProcessorEditor
    : public juce::AudioProcessorEditor {
public:
  PointillisticSynthAudioProcessorEditor(
      audio_plugin::AudioPluginAudioProcessor&,
      juce::AbstractFifo& fifo,
      GrainInfoForVis* buffer);
  ~PointillisticSynthAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  audio_plugin::AudioPluginAudioProcessor& processorRef;

  DebugUIPanel debugUIPanel;  // Added DebugUIPanel member

  PodComponent pitchPod;
  PodComponent densityPod;
  PodComponent durationPod;
  PodComponent panPod;
  VisualizationComponent visualizationComponent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      PointillisticSynthAudioProcessorEditor)
};

}  // namespace audio_plugin
