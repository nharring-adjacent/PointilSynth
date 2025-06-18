#include "Pointilsynth/PointilismInterfaces.h"  // Defines Grain, StochasticModel, AudioEngine
#include <catch2/catch_test_macros.hpp>

TEST_CASE("CanConstructGrain", "[PointilismInterfacesTest]") {
  REQUIRE_NOTHROW(Grain{});
  Grain g{};                   // Aggregate initialization
  REQUIRE(g.isAlive == true);  // Check default value
}

TEST_CASE("CanConstructStochasticModel", "[PointilismInterfacesTest]") {
  REQUIRE_NOTHROW(std::make_unique<StochasticModel>());
}

TEST_CASE("CanConstructAudioEngine", "[PointilismInterfacesTest]") {
  REQUIRE_NOTHROW(std::make_unique<AudioEngine>());
}
