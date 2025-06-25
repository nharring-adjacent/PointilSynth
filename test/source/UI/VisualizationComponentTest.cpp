#include "UI/VisualizationComponent.h"
#include "Pointilsynth/InertialHistoryManager.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;
using Catch::Matchers::WithinAbs;

namespace audio_plugin {

class TestInertialHistoryManager : public InertialHistoryManager {
public:
    void addTestNote(int note, float velocity, double position) {
        InertialNote newNote;
        newNote.noteNumber = note;
        newNote.velocity = velocity;
        newNote.positionInBars = position;
        newNote.currentInfluence = 1.0f;
        newNote.ageInBars = 0.0f;
        
        const ScopedLock sl(lock);
        notes.push_back(newNote);
    }
    
    void clearNotes() {
        const ScopedLock sl(lock);
        notes.clear();
    }
};

struct VisualizationComponentTestFixture {
    VisualizationComponentTestFixture() {
        // Initialize OpenGL for testing
        openGLContext.attachTo(*component);
    }
    
    ~VisualizationComponentTestFixture() {
        openGLContext.detach();
    }
    
    ScopedJuceInitialiser_GUI libraryInitialiser;
    AbstractFifo fifo{8};
    std::array<GrainInfoForVis, 8> buffer{};
    TestInertialHistoryManager historyManager;
    std::unique_ptr<VisualizationComponent> component{std::make_unique<VisualizationComponent>(fifo, buffer.data())};
    OpenGLContext openGLContext;
    
    void simulateGrain(int note, float pan, float pitch, float age = 0.0f) {
        GrainInfoForVis grain{};
        grain.noteNumber = note;
        grain.pan = pan;
        grain.pitch = pitch;
        grain.age = age;
        
        int start1, size1, start2, size2;
        fifo.prepareToWrite(1, start1, size1, start2, size2);
        if (size1 > 0) buffer[start1] = grain;
        if (size2 > 0) buffer[start2] = grain;
        fifo.finishedWrite(size1 + size2);
    }
};

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Construction and Initialization") {
    SECTION("Can be constructed") {
        REQUIRE(component != nullptr);
    }
    
    SECTION("Has default visual settings") {
        component->setSize(800, 600);
        component->paint(Graphics(component->getLocalBounds()));
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Grain Visualization") {
    component->setSize(800, 600);
    
    SECTION("Processes grains from FIFO") {
        simulateGrain(60, 0.0f, 0.5f);
        component->paint(Graphics(component->getLocalBounds()));
        
        // Should process the grain from the FIFO
        REQUIRE_FALSE(component->isEmpty());
    }
    
    SECTION("Handles multiple grains") {
        for (int i = 0; i < 5; ++i) {
            simulateGrain(60 + i, i * 0.2f - 0.5f, 0.5f);
        }
        component->paint(Graphics(component->getLocalBounds()));
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Inertial History Integration") {
    component->setSize(800, 600);
    component->setInertialHistoryManager(&historyManager);
    
    SECTION("Shows active notes in history") {
        historyManager.addTestNote(60, 1.0f, 0.0);
        historyManager.addTestNote(64, 0.5f, 1.0);
        
        component->paint(Graphics(component->getLocalBounds()));
    }
    
    SECTION("Connects grains to notes") {
        historyManager.addTestNote(60, 1.0f, 0.0);
        simulateGrain(60, 0.0f, 0.5f);
        
        component->paint(Graphics(component->getLocalBounds()));
    }
    
    SECTION("Updates with note decay") {
        historyManager.addTestNote(60, 1.0f, 0.0);
        component->paint(Graphics(component->getLocalBounds()));
        
        // Simulate some time passing
        historyManager.clearNotes();
        component->paint(Graphics(component->getLocalBounds()));
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Visual Settings") {
    component->setSize(800, 600);
    
    SECTION("Particle size changes") {
        component->setParticleSize(5.0f, 15.0f);
        simulateGrain(60, 0.0f, 0.5f);
        component->paint(Graphics(component->getLocalBounds()));
    }
    
    SECTION("Opacity changes") {
        component->setParticleOpacity(0.7f);
        simulateGrain(60, 0.0f, 0.5f);
        component->paint(Graphics(component->getLocalBounds()));
    }
    
    SECTION("Gravity changes") {
        component->setGravity(0.2f);
        simulateGrain(60, 0.0f, 0.5f);
        component->paint(Graphics(component->getLocalBounds()));
    }
    
    SECTION("Visual presets") {
        component->setVisualPreset(VisualizationComponent::VisualPreset::Minimalist);
        simulateGrain(60, 0.0f, 0.5f);
        component->paint(Graphics(component->getLocalBounds()));
        
        component->setVisualPreset(VisualizationComponent::VisualPreset::Retro);
        simulateGrain(64, 0.5f, 0.7f);
        component->paint(Graphics(component->getLocalBounds()));
        
        component->setVisualPreset(VisualizationComponent::VisualPreset::Default);
        component->paint(Graphics(component->getLocalBounds()));
    }
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Metering") {
    component->setSize(800, 600);
    component->setShowMeters(true);
    
    // Test meter updates
    component->setMeterValues(0.5f, 0.7f);
    component->paint(Graphics(component->getLocalBounds()));
    
    // Test peak hold
    component->setMeterValues(0.3f, 0.2f);
    component->paint(Graphics(component->getLocalBounds()));
    
    // Test meter disable
    component->setShowMeters(false);
    component->paint(Graphics(component->getLocalBounds()));
}

TEST_CASE_METHOD(VisualizationComponentTestFixture, "Visual Feedback") {
    component->setSize(800, 600);
    
    // Trigger visual feedback
    component->triggerVisualFeedback(60, 1.0f);
    component->paint(Graphics(component->getLocalBounds()));
    
    // Test multiple feedbacks
    component->triggerVisualFeedback(64, 0.8f);
    component->triggerVisualFeedback(67, 0.6f);
    component->paint(Graphics(component->getLocalBounds()));
    
    // Test feedback animation
    for (int i = 0; i < 10; ++i) {
        component->paint(Graphics(component->getLocalBounds()));
    }
}

}  // namespace audio_plugin
