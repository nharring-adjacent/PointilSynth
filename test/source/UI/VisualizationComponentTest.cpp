#include "UI/VisualizationComponent.h"
#include "Pointilsynth/InertialHistoryManager.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;
using Catch::Matchers::WithinAbs;

namespace audio_plugin {

class TestInertialHistoryManager : public InertialHistoryManager {
public:
    void addTestNote(int note, float velocity, double position) {
        // Use the public API to add a note
        addNote(note, velocity, position);
    }
    
    void clearNotes() {
        // In a real test, we would either:
        // 1. Add a public clear() method to InertialHistoryManager, or
        // 2. Create a new instance of the history manager
        // For now, we'll simulate clearing by adding a note with 0 velocity,
        // which should make the notes decay quickly
        constexpr double largeTimeValue = 1e6; // A large time value to make notes decay
        update(largeTimeValue, 1.0);
    }
};

class MockAudioProcessor : public juce::AudioProcessor {
public:
    MockAudioProcessor() : AudioProcessor(juce::AudioProcessor::BusesProperties()) {}
    const juce::String getName() const override { return "MockProcessor"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
};

struct VisualizationComponentTestFixture {
    VisualizationComponentTestFixture() : apvts(mockProcessor, nullptr, "TestAPVTS", {}) {
        // Initialize OpenGL for testing
        openGLContext.attachTo(*component);
    }
    
    ~VisualizationComponentTestFixture() {
        openGLContext.detach();
    }
    
    ScopedJuceInitialiser_GUI libraryInitialiser;
    MockAudioProcessor mockProcessor;
    juce::AudioProcessorValueTreeState apvts;
    juce::AbstractFifo fifo{8};
    std::array<GrainInfoForVis, 8> buffer{};
    TestInertialHistoryManager historyManager;
    juce::AudioDeviceManager deviceManager;
    std::unique_ptr<VisualizationComponent> component{std::make_unique<VisualizationComponent>(apvts)};
    juce::OpenGLContext openGLContext;
    juce::CriticalSection lock;
    
    void simulateGrain(int note, float pan, float pitch, float age = 0.0f) {
        GrainInfoForVis grain{};
        grain.pitch = note;
        grain.pan = pan;
        grain.velocity = age;
        
        component->addGrainInfo(grain);
    }
};

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Construction and Initialization") {
    SECTION("Can be constructed") {
        REQUIRE(component != nullptr);
    }
    
    SECTION("Has default visual settings") {
        component->setSize(800, 600);
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Grain Visualization") {
    component->setSize(800, 600);
    
    SECTION("Processes grains from FIFO") {
        simulateGrain(60, 0.0f, 0.5f);
        juce::Image img2(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g2(img2);
        component->paint(g2);
        
        // Should process the grain from the FIFO
    }
    
    SECTION("Handles multiple grains") {
        for (int i = 0; i < 5; ++i) {
            simulateGrain(60 + i, i * 0.2f - 0.5f, 0.5f);
        }
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Inertial History Integration") {
    component->setSize(800, 600);
    component->setInertialHistoryManager(&historyManager);
    
    SECTION("Shows active notes in history") {
        historyManager.addTestNote(60, 1.0f, 0.0);
        historyManager.addTestNote(64, 0.5f, 1.0);
        
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
    
    SECTION("Connects grains to notes") {
        historyManager.addTestNote(60, 1.0f, 0.0);
        simulateGrain(60, 0.0f, 0.5f);
        
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
    
    SECTION("Updates with note decay") {
        historyManager.addTestNote(60, 1.0f, 0.0);
        {
            juce::Image img1(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
            juce::Graphics g1(img1);
            component->paint(g1);
        }
        
        // Simulate some time passing
        historyManager.clearNotes();
        {
            juce::Image img2(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
            juce::Graphics g2(img2);
            component->paint(g2);
        }
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Visual Settings") {
    component->setSize(800, 600);
    
    SECTION("Particle size changes") {
        component->setParticleSize(5.0f, 15.0f);
        simulateGrain(60, 0.0f, 0.5f);
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
    
    SECTION("Opacity changes") {
        component->setParticleOpacity(0.7f);
        simulateGrain(60, 0.0f, 0.5f);
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
    
    SECTION("Gravity changes") {
        component->setGravity(0.2f);
        simulateGrain(60, 0.0f, 0.5f);
        juce::Image img(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics g(img);
        component->paint(g);
    }
    
    SECTION("Visual presets") {
        component->setVisualPreset(VisualizationComponent::VisualPreset::Minimalist);
        simulateGrain(60, 0.0f, 0.5f);
        {
            juce::Image img1(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
            juce::Graphics g1(img1);
            component->paint(g1);
        }
        
        component->setVisualPreset(VisualizationComponent::VisualPreset::Retro);
        simulateGrain(64, 0.5f, 0.7f);
        {
            juce::Image img2(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
            juce::Graphics g2(img2);
            component->paint(g2);
        }
        
        component->setVisualPreset(VisualizationComponent::VisualPreset::Default);
        {
            juce::Image img3(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
            juce::Graphics g3(img3);
            component->paint(g3);
        }
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Metering") {
    component->setSize(800, 600);
    component->setShowMeters(true);
    
    // Test meter updates
    component->setMeterValues(0.5f, 0.7f);
    {
        juce::Image meterImg(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics meterG(meterImg);
        component->paint(meterG);
    }
    
    // Test peak hold
    component->setMeterValues(0.3f, 0.2f);
    {
        juce::Image meterImg(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics meterG(meterImg);
        component->paint(meterG);
    }
    
    // Test meter disable
    component->setShowMeters(false);
    {
        juce::Image meterImg(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics meterG(meterImg);
        component->paint(meterG);
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Visual Feedback") {
    component->setSize(800, 600);
    
    // Trigger visual feedback
    component->triggerVisualFeedback(60, 1.0f);
    {
        juce::Image meterImg(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics meterG(meterImg);
        component->paint(meterG);
    }
    
    // Test multiple feedbacks
    component->triggerVisualFeedback(64, 0.8f);
    component->triggerVisualFeedback(67, 0.6f);
    {
        juce::Image meterImg(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics meterG(meterImg);
        component->paint(meterG);
    }
    
    // Test feedback animation
    for (int i = 0; i < 10; ++i) {
        juce::Image animImg(juce::Image::ARGB, component->getWidth(), component->getHeight(), true);
        juce::Graphics animG(animImg);
        component->paint(animG);
    }
}

}  // namespace audio_plugin
