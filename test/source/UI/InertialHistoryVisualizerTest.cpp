#include "UI/InertialHistoryVisualizer.h"
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

using audio_plugin::InertialHistoryVisualizer;

struct InertialHistoryVisualizerFixture {
  juce::ScopedJuceInitialiser_GUI libraryInitialiser;
  InertialHistoryManager manager;
};

TEST_CASE_METHOD(InertialHistoryVisualizerFixture,
                 "CanConstruct",
                 "[InertialHistoryVisualizer]") {
  REQUIRE_NOTHROW([&] { InertialHistoryVisualizer comp(manager); }());
}

TEST_CASE_METHOD(InertialHistoryVisualizerFixture,
                 "PaintDoesNotCrash",
                 "[InertialHistoryVisualizer]") {
  InertialHistoryVisualizer comp(manager);
  comp.setBounds(0, 0, 100, 50);
  juce::Image img(juce::Image::RGB, 100, 50, true);
  juce::Graphics g(img);
  REQUIRE_NOTHROW([&] { comp.paint(g); }());
}
