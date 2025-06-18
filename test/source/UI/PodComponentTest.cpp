#include "PodComponent.h"  // Defines audio_plugin::PodComponent
#include "Pointilsynth/ConfigManager.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>  // For ScopedJuceInitialiser_GUI

namespace audio_plugin {
// Minimal ScopedJuceInitialiser_GUI for tests needing it.
struct JuceGuiTestFixture {
  JuceGuiTestFixture() = default;
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_CASE_METHOD(JuceGuiTestFixture,
                 "PodComponentCanConstruct",
                 "[PodComponent]") {
  REQUIRE_NOTHROW(
      std::make_unique<PodComponent>(ConfigManager::ParamID::pitch, "Pitch"));
}
}  // namespace audio_plugin
