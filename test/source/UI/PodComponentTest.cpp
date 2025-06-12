#include "Pointilsynth/UI/PodComponent.h" // Defines audio_plugin::PodComponent
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h> // For ScopedJuceInitialiser_GUI

namespace audio_plugin {
// Minimal ScopedJuceInitialiser_GUI for tests needing it.
struct JuceGuiTestFixture : public ::testing::Test {
    JuceGuiTestFixture() = default;
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_F(JuceGuiTestFixture, PodComponentCanConstruct) {
    EXPECT_NO_THROW(std::make_unique<PodComponent>());
}
} // namespace audio_plugin
