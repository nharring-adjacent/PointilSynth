#include "Pointilsynth/Oscillator.h"  // Defines Pointilsynth::Oscillator
#include <catch2/catch_test_macros.hpp>

TEST_CASE("CanConstruct", "[OscillatorTest]") {
  REQUIRE_NOTHROW(std::make_unique<Pointilsynth::Oscillator>());
}
