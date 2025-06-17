#include "UI/VisualizationComponent.h"
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

struct VisualizationComponentTestFixture : public ::testing::Test {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  juce::AbstractFifo fifo{8};
  std::array<GrainInfoForVis, 8> buffer{};
};

TEST_F(VisualizationComponentTestFixture, CanConstruct) {
  EXPECT_NO_THROW(VisualizationComponent comp(fifo, buffer.data()));
}

}  // namespace audio_plugin
