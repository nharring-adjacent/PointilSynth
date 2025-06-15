#include "Pointilsynth/GrainEnvelope.h"
#include <gtest/gtest.h>

TEST(GrainEnvelopeTest, CanConstruct) {
  EXPECT_NO_THROW(std::make_unique<GrainEnvelope>());
}
