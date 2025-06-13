#include "Pointilsynth/DebugUIPanel.h"
#include "Pointilsynth/PointilismInterfaces.h" // For StochasticModel
#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>

TEST(DebugUIPanelTest, CanConstruct) {
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;
    StochasticModel model;
    EXPECT_NO_THROW(std::make_unique<DebugUIPanel>(&model));
}
