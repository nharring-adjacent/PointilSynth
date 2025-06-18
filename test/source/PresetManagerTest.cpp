#include "Pointilsynth/PresetManager.h"  // Defines Pointilism::PresetManager
#include "Pointilsynth/PointilismInterfaces.h"  // For StochasticModel
#include <catch2/catch_test_macros.hpp>

TEST_CASE("CanConstruct", "[PresetManagerTest]") {
  StochasticModel model;
  REQUIRE_NOTHROW(std::make_unique<Pointilism::PresetManager>(model));
}
