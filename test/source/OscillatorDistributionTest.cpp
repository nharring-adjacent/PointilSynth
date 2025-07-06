#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_graphics/juce_graphics.h>

// Simple test to verify the test framework is working
TEST_CASE("Basic test framework verification", "[core]") {
    REQUIRE(1 + 1 == 2);
}

// Mock Oscillator class for testing
class MockOscillator {
public:
    enum Waveform {
        SINE = 0,
        SAW,
        SQUARE,
        NOISE
    };
    
    void setWaveform(Waveform w) { waveform_ = w; }
    Waveform getWaveform() const { return waveform_; }
    
private:
    Waveform waveform_ = SINE;
};

// Mock Grain class for testing
class MockGrain {
public:
    MockOscillator oscillator;
};

// Simple test for oscillator waveform setting
TEST_CASE("Oscillator waveform test", "[oscillator]") {
    MockOscillator osc;
    
    SECTION("Default waveform is SINE") {
        REQUIRE(osc.getWaveform() == MockOscillator::SINE);
    }
    
    SECTION("Can set and get waveform") {
        osc.setWaveform(MockOscillator::SAW);
        REQUIRE(osc.getWaveform() == MockOscillator::SAW);
        
        osc.setWaveform(MockOscillator::SQUARE);
        REQUIRE(osc.getWaveform() == MockOscillator::SQUARE);
    }
}

// Simple test for grain with oscillator
TEST_CASE("Grain oscillator test", "[grain][oscillator]") {
    MockGrain grain;
    
    SECTION("Default grain oscillator is SINE") {
        REQUIRE(grain.oscillator.getWaveform() == MockOscillator::SINE);
    }
    
    SECTION("Can set grain oscillator waveform") {
        grain.oscillator.setWaveform(MockOscillator::NOISE);
        REQUIRE(grain.oscillator.getWaveform() == MockOscillator::NOISE);
    }
}

// Test for oscillator distribution (simplified)
TEST_CASE("Oscillator distribution test", "[oscillator][distribution]") {
    const int numOscillators = 4;
    std::array<int, numOscillators> counts = {0};
    
    // Simulate generating grains with different oscillators
    const int numGrains = 100;
    for (int i = 0; i < numGrains; ++i) {
        MockOscillator osc;
        osc.setWaveform(static_cast<MockOscillator::Waveform>(i % numOscillators));
        counts[osc.getWaveform()]++;
    }
    
    // Verify all oscillator types were used
    for (int i = 0; i < numOscillators; ++i) {
        REQUIRE(counts[i] > 0);
    }
}

// Test for color mapping (simplified)
TEST_CASE("Color mapping test", "[color][mapping]") {
    juce::Colour testColor = juce::Colours::red;
    
    // Simple test to verify color components
    REQUIRE(testColor.getRed() == 255);
    REQUIRE(testColor.getGreen() == 0);
    REQUIRE(testColor.getBlue() == 0);
}
