#include "Pointilsynth/PointilismInterfaces.h"  // Defines Grain, StochasticModel, AudioEngine
#include <gtest/gtest.h>

TEST(PointilismInterfacesTest, CanConstructGrain) {
  EXPECT_NO_THROW(Grain grain);
  Grain g{};                   // Aggregate initialization
  EXPECT_EQ(g.isAlive, true);  // Check default value
}

TEST(PointilismInterfacesTest, CanConstructStochasticModel) {
  EXPECT_NO_THROW(std::make_unique<StochasticModel>());
}

TEST(PointilismInterfacesTest, CanConstructAudioEngine) {
  EXPECT_NO_THROW(std::make_unique<AudioEngine>());
}
