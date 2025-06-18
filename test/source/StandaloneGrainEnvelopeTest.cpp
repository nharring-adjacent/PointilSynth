#include "GrainEnvelope.h"
#include <juce_core/juce_core.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE("HannShapeReturnsExpectedValues", "[StandaloneGrainEnvelopeTest]") {
  GrainEnvelope env;
  env.setShape(GrainEnvelope::Shape::Hann);
  const int duration = 100;
  REQUIRE(juce::approximatelyEqual(env.getAmplitude(0, duration), 0.0f));
  REQUIRE(env.getAmplitude(duration / 2, duration) ==
          Catch::Approx(1.0f).margin(1e-6f));
  REQUIRE(env.getAmplitude(duration - 1, duration) ==
          Catch::Approx(0.0f).margin(1e-2f));
}

TEST_CASE("TrapezoidShapeReturnsExpectedValues",
          "[StandaloneGrainEnvelopeTest]") {
  GrainEnvelope env;
  env.setShape(GrainEnvelope::Shape::Trapezoid);
  const int duration = 100;
  REQUIRE(juce::approximatelyEqual(env.getAmplitude(0, duration), 0.0f));
  REQUIRE(juce::approximatelyEqual(env.getAmplitude(10, duration), 1.0f));
  REQUIRE(juce::approximatelyEqual(env.getAmplitude(50, duration), 1.0f));
  REQUIRE(env.getAmplitude(99, duration) == Catch::Approx(0.1f).margin(1e-6f));
}
