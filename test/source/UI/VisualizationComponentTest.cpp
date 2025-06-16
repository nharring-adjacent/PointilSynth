#include "UI/VisualizationComponent.h"
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

struct VisualizationComponentTestFixture : public ::testing::Test {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
};

TEST_F(VisualizationComponentTestFixture, CanConstruct) {
  EXPECT_NO_THROW(VisualizationComponent comp);
}

}  // namespace audio_plugin
