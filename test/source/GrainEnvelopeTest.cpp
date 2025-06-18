#include "Pointilsynth/GrainEnvelope.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("CanConstruct", "[GrainEnvelopeTest]") {
  REQUIRE_NOTHROW(std::make_unique<GrainEnvelope>());
}
