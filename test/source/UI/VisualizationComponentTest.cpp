#include "UI/VisualizationComponent.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

struct VisualizationComponentTestFixture {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  juce::AbstractFifo fifo{8};
  std::array<GrainInfoForVis, 8> buffer{};
};

TEST_CASE_METHOD(VisualizationComponentTestFixture,
                 "CanConstruct",
                 "[VisualizationComponentTestFixture]") {
  REQUIRE_NOTHROW([&] { VisualizationComponent comp(fifo, buffer.data()); }());
}

}  // namespace audio_plugin
