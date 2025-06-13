#include "Pointilsynth/PresetManager.h" // Defines Pointilism::PresetManager
#include "Pointilsynth/PointilismInterfaces.h" // For StochasticModel
#include <gtest/gtest.h>

TEST(PresetManagerTest, CanConstruct) {
    StochasticModel model;
    EXPECT_NO_THROW(std::make_unique<Pointilism::PresetManager>(model));
}
