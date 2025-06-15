#include "GrainEnvelope.h"
#include <gtest/gtest.h>

TEST(StandaloneGrainEnvelopeTest, HannShapeReturnsExpectedValues) {
  GrainEnvelope env;
  env.setShape(GrainEnvelope::Shape::Hann);
  const int duration = 100;
  EXPECT_FLOAT_EQ(env.getAmplitude(0, duration), 0.0f);
  EXPECT_NEAR(env.getAmplitude(duration / 2, duration), 1.0f, 1e-6f);
  EXPECT_NEAR(env.getAmplitude(duration - 1, duration), 0.0f, 1e-2f);
}

TEST(StandaloneGrainEnvelopeTest, TrapezoidShapeReturnsExpectedValues) {
  GrainEnvelope env;
  env.setShape(GrainEnvelope::Shape::Trapezoid);
  const int duration = 100;
  EXPECT_FLOAT_EQ(env.getAmplitude(0, duration), 0.0f);
  EXPECT_FLOAT_EQ(env.getAmplitude(10, duration), 1.0f);
  EXPECT_FLOAT_EQ(env.getAmplitude(50, duration), 1.0f);
  EXPECT_NEAR(env.getAmplitude(99, duration), 0.1f, 1e-6f);
}
