#include "Pointilsynth/Oscillator.h" // Defines Pointilsynth::Oscillator
#include <gtest/gtest.h>

TEST(OscillatorTest, CanConstruct) {
    EXPECT_NO_THROW(std::make_unique<Pointilsynth::Oscillator>());
}
