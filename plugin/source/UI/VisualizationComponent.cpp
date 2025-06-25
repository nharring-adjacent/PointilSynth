#include "UI/VisualizationComponent.h"
#include <cmath>
#include <algorithm>
#include "Pointilsynth/InertialHistoryManager.h"

namespace audio_plugin {

namespace {
inline double currentTimeSeconds() {
    return juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

juce::Colour getColorForPitch(float pitch, float velocity) {
    // Map pitch to hue (0.0-1.0)
    float hue = juce::jmap(pitch, 21.0f, 108.0f, 0.0f, 1.0f);
    // Map velocity to saturation and brightness
    float saturation = juce::jmap(velocity, 0.0f, 1.0f, 0.3f, 1.0f);
    float brightness = juce::jmap(velocity, 0.0f, 1.0f, 0.5f, 1.0f);
    return juce::Colour::fromHSV(hue, saturation, brightness, 0.8f);
}
// Preset configurations
namespace {
    const juce::ColourGradient createDefaultGradient() {
        juce::ColourGradient g;
        g.addColour(0.0f, juce::Colours::red);
        g.addColour(0.2f, juce::Colours::orange);
        g.addColour(0.4f, juce::Colours::yellow);
        g.addColour(0.6f, juce::Colours::green);
        g.addColour(0.8f, juce::Colours::blue);
        g.addColour(1.0f, juce::Colours::violet);
        return g;
    }
    
    const juce::ColourGradient createMinimalistGradient() {
        juce::ColourGradient g;
        g.addColour(0.0f, juce::Colour::fromFloatRGBA(0.9f, 0.9f, 0.9f, 0.8f));
        g.addColour(1.0f, juce::Colour::fromFloatRGBA(0.7f, 0.7f, 0.7f, 0.6f));
        return g;
    }
    
    const juce::ColourGradient createRetroGradient() {
        juce::ColourGradient g;
        g.addColour(0.0f, juce::Colours::red);
        g.addColour(0.5f, juce::Colours::yellow);
        g.addColour(1.0f, juce::Colours::lime);
        return g;
    }
}

}  // namespace

// Visual preset implementations
void VisualizationComponent::setupDefaultPreset() {
    setParticleSize(3.0f, 15.0f);
    setParticleOpacity(0.8f);
    setGravity(0.05f);
    setTrailLength(20);
    setColorScheme(createDefaultGradient());
}

void VisualizationComponent::setupMinimalistPreset() {
    setParticleSize(2.0f, 8.0f);
    setParticleOpacity(0.6f);
    setGravity(0.01f);
    setTrailLength(10);
    setColorScheme(createMinimalistGradient());
}

void VisualizationComponent::setupRetroPreset() {
    setParticleSize(4.0f, 20.0f);
    setParticleOpacity(0.9f);
    setGravity(0.1f);
    setTrailLength(5);
    setColorScheme(createRetroGradient());
}

void VisualizationComponent::setVisualPreset(VisualPreset preset) {
    currentPreset_ = preset;
    
    switch (preset) {
        case VisualPreset::Minimalist: setupMinimalistPreset(); break;
        case VisualPreset::Retro: setupRetroPreset(); break;
        case VisualPreset::ParticleWave: /* Setup particle wave */ break;
        case VisualPreset::Spectral: /* Setup spectral */ break;
        case VisualPreset::Default:
        default: setupDefaultPreset(); break;
    }
}

// Metering and visual feedback
void VisualizationComponent::setMeterValues(float left, float right) {
    leftMeterLevel_ = juce::jlimit(0.0f, 1.0f, left);
    rightMeterLevel_ = juce::jlimit(0.0f, 1.0f, right);
    
    // Update peaks if this is a new max
    leftMeterPeak_ = std::max(leftMeterPeak_, leftMeterLevel_);
    rightMeterPeak_ = std::max(rightMeterPeak_, rightMeterLevel_);
    lastPeakUpdate_ = juce::Time::currentTimeMillis();
}

void VisualizationComponent::updateMeterPeaks() {
    auto now = juce::Time::currentTimeMillis();
    if (now - lastPeakUpdate_ > 1000) { // 1 second peak hold
        leftMeterPeak_ *= 0.9f;
        rightMeterPeak_ *= 0.9f;
    }
}

void VisualizationComponent::triggerVisualFeedback(int note, float velocity) {
    VisualFeedback feedback;
    feedback.positionX = juce::jmap(float(note), 0.0f, 127.0f, 0.0f, 1.0f);
    feedback.positionY = 0.5f;
    feedback.radius = 0.0f;
    feedback.alpha = 0.8f * velocity;
    feedback.color = getColorForPitch(note, velocity);
    
    const juce::SpinLock::ScopedLockType lock(particleLock);
    activeFeedbacks_.push_back(feedback);
}

void VisualizationComponent::updateVisualFeedbacks() {
    auto now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    
    for (auto it = activeFeedbacks_.begin(); it != activeFeedbacks_.end(); ) {
        it->radius += 0.1f;
        it->alpha -= 0.01f;
        
        if (it->alpha <= 0.0f) {
            it = activeFeedbacks_.erase(it);
        } else {
            ++it;
        }
    }
}

void VisualizationComponent::drawVisualFeedbacks(juce::Graphics& g) {
    for (const auto& feedback : activeFeedbacks_) {
        auto color = feedback.color.withAlpha(feedback.alpha);
        g.setColour(color);
        
        float x = feedback.positionX * getWidth();
        float y = feedback.positionY * getHeight();
        float diameter = feedback.radius * 100.0f;
        
        g.drawEllipse(x - diameter/2, y - diameter/2, diameter, diameter, 2.0f);
    }
}

VisualizationComponent::VisualizationComponent(juce::AbstractFifo& fifo,
                                             GrainInfoForVis* buffer)
    : fifo_(fifo), buffer_(buffer) {
    openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(true);
    
    // Initialize OpenGL resources
    openGLContext.executeOnGLThread([this](juce::OpenGLContext&) {
        initializeGL();
    }, false);
    
    startTimerHz(60);  // Update physics at 60 FPS
}

VisualizationComponent::~VisualizationComponent() {
    stopTimer();
    openGLContext.setContinuousRepainting(false);
    openGLContext.detach();
}

void VisualizationComponent::initializeGL() {
    // Initialize shaders, VBOs, and other OpenGL resources
    // This runs on the OpenGL thread
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 position;
        layout (location = 1) in float size;
        layout (location = 2) in vec4 color;
        
        out vec4 fragColor;
        out float fragSize;
        
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(position, 0.0, 1.0);
            fragColor = color;
            fragSize = size;
            gl_PointSize = size;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec4 fragColor;
        in float fragSize;
        out vec4 outColor;
        
        void main() {
            vec2 coord = gl_PointCoord * 2.0 - 1.0;
            float dist = length(coord);
            if (dist > 1.0) discard;
            
            // Soft particle with glow
            float alpha = smoothstep(1.0, 0.2, dist);
            outColor = vec4(fragColor.rgb, fragColor.a * alpha);
        }
    )";
    
    // Compile shaders and link program
    // [Shader compilation code...]
}

void VisualizationComponent::renderOpenGL() {
    juce::OpenGLHelpers::clear(juce::Colours::black);
    
    // Set up projection matrix
    auto desktopScale = (float)openGLContext.getRenderingScale();
    auto w = (float)getWidth();
    auto h = (float)getHeight();
    
    // Set up blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Bind shader program and set uniforms
    // [Shader binding and uniform setup...]
    
    // Update and render particles
    updateParticles();
    renderParticles();
    
    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        DBG("OpenGL error: " << std::hex << err);
    }
}

void VisualizationComponent::updateParticles() {
    const double now = currentTimeSeconds();
    const double deltaTime = now - lastUpdateTime;
    lastUpdateTime = now;
    
    // Update existing particles
    for (auto& grain : grains) {
        // Apply physics (gravity, velocity, etc.)
        grain.velocityY += gravity * deltaTime;
        grain.x += grain.velocityX * deltaTime;
        grain.y += grain.velocityY * deltaTime;
        
        // Apply damping
        grain.velocityX *= 0.99f;
        grain.velocityY *= 0.99f;
        
        // Calculate age-based properties
        float age = static_cast<float>((now - grain.startTime) / grain.maxAge);
        grain.currentSize = grain.size * (1.0f - age * 0.5f); // Shrink over time
        grain.currentAlpha = 1.0f - juce::jlimit(0.0f, 1.0f, age * 1.2f); // Fade out
    }
    
    // Remove dead particles
    grains.erase(std::remove_if(grains.begin(), grains.end(),
                              [now](const VisualGrain& g) {
                                  return (now - g.startTime) > g.maxAge;
                              }),
               grains.end());
    
    // Add new grains from audio thread
    addNewGrains();
}

void VisualizationComponent::addNewGrains() {
    int start1, size1, start2, size2;
    const double now = currentTimeSeconds();
    
    while (true) {
        fifo_.prepareToRead(1, start1, size1, start2, size2);
        if (size1 == 0) break;
        
        const auto& info = buffer_[start1];
        
        VisualGrain grain;
        grain.pan = info.pan;
        grain.pitch = info.pitch;
        grain.size = 6.0f + info.velocity * 10.0f; // Size based on velocity
        grain.velocityX = (rand() % 100 - 50) * 0.01f; // Random horizontal velocity
        grain.velocityY = (rand() % 50) * -0.01f; // Upward velocity
        grain.startTime = now;
        grain.maxAge = static_cast<double>(info.durationSeconds) * 2.0; // Longer visual life
        grain.colour = getColorForPitch(info.pitch, info.velocity);
        grain.currentSize = grain.size;
        grain.currentAlpha = 1.0f;
        
        // Add to particles with thread safety
        const juce::SpinLock::ScopedLockType lock(particleLock);
        grains.push_back(grain);
        
        fifo_.finishedRead(size1);
    }
}

void VisualizationComponent::renderParticles() {
    // Use instanced rendering or point sprites for better performance
    // This is a simplified version - in practice, you'd use VBOs and instancing
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    // Begin drawing particles
    juce::OpenGLHelpers::ScopedSaveShaderState shaderState;
    
    // Draw note history first (behind particles)
    if (inertialHistoryManager_) {
        drawNoteHistory();
    }
    
    // Draw particles
    {
        const juce::SpinLock::ScopedLockType lock(particleLock);
        
        // Draw particles
        for (const auto& grain : grains) {
            float x = juce::jmap(grain.x, -1.0f, 1.0f, 0.0f, static_cast<float>(getWidth()));
            float y = juce::jmap(grain.y, 0.0f, 1.0f, static_cast<float>(getHeight()), 0.0f);
            
            // Set color with alpha
            juce::Colour c = grain.colour.withAlpha(grain.currentAlpha);
            glColor4f(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
            
            // Draw point
            glPointSize(grain.currentSize);
            glBegin(GL_POINTS);
            glVertex2f(x, y);
            glEnd();
            
            // Draw trail if enabled
            if (!grain.trailPositions.empty()) {
                drawParticleTrail(grain);
            }
        }
    }
    
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void VisualizationComponent::drawNoteHistory() {
    if (!inertialHistoryManager_) return;
    
    const int numNotes = inertialHistoryManager_->getNumNotes();
    if (numNotes == 0) return;
    
    const float width = static_cast<float>(getWidth());
    const float height = static_cast<float>(getHeight());
    
    // Draw note history background
    glBegin(GL_QUADS);
    glColor4f(0.05f, 0.05f, 0.1f, 0.3f);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height * 0.15f); // Use top 15% for note history
    glVertex2f(0, height * 0.15f);
    glEnd();
    
    // Draw piano roll lines
    glBegin(GL_LINES);
    glColor4f(0.3f, 0.3f, 0.4f, 0.5f);
    for (int i = 0; i < 12; ++i) {
        float y = juce::jmap(static_cast<float>(i) / 11.0f, 0.0f, 1.0f, 
                           height * 0.15f, 0.0f);
        glVertex2f(0, y);
        glVertex2f(width, y);
    }
    glEnd();
    
    // Draw active notes
    for (int i = 0; i < numNotes; ++i) {
        const auto& note = inertialHistoryManager_->getNote(i);
        
        // Map note number to y position (C1 to C8)
        float noteY = juce::jmap(static_cast<float>(note.noteNumber % 12), 
                               11.0f, 0.0f, 
                               0.0f, height * 0.15f);
        
        // Map age to x position (right to left)
        float age = static_cast<float>(note.ageInBars);
        float x = width * (1.0f - std::min(age / 4.0f, 1.0f));
        
        // Draw note influence
        float size = 8.0f + 16.0f * note.currentInfluence;
        juce::Colour noteColor = juce::Colour::fromHSV(
            static_cast<float>(note.noteNumber) / 127.0f, 
            0.8f, 
            0.9f,
            0.8f * note.currentInfluence);
            
        glColor4f(noteColor.getFloatRed(), noteColor.getFloatGreen(), 
                 noteColor.getFloatBlue(), noteColor.getFloatAlpha());
        
        glBegin(GL_QUADS);
        glVertex2f(x - size/2, noteY - size/2);
        glVertex2f(x + size/2, noteY - size/2);
        glVertex2f(x + size/2, noteY + size/2);
        glVertex2f(x - size/2, noteY + size/2);
        glEnd();
        
        // Draw connection to grains
        drawNoteConnections(note);
    }
}

void VisualizationComponent::drawNoteConnections(const InertialNote& note) {
    const float width = static_cast<float>(getWidth());
    const float height = static_cast<float>(getHeight());
    
    // Find all grains that originated from this note
    for (const auto& grain : grains) {
        if (grain.sourceNoteNumber == note.noteNumber) {
            float noteY = juce::jmap(static_cast<float>(note.noteNumber % 12), 
                                   11.0f, 0.0f, 
                                   0.0f, height * 0.15f);
            float grainX = juce::jmap(grain.x, -1.0f, 1.0f, 0.0f, width);
            float grainY = juce::jmap(grain.y, 0.0f, 1.0f, height, 0.0f);
            
            // Only draw connection if grain is below the note history area
            if (grainY > height * 0.15f) {
                float age = static_cast<float>(grain.ageInSeconds / grain.maxAge);
                float alpha = 0.3f * (1.0f - age * 0.5f);
                
                glBegin(GL_LINES);
                glColor4f(0.5f, 0.5f, 0.8f, alpha);
                glVertex2f(grainX, grainY);
                glVertex2f(grainX, height * 0.15f);
                glEnd();
            }
        }
    }
}

void VisualizationComponent::drawParticleTrail(const VisualGrain& grain) {
    if (grain.trailPositions.size() < 2) return;
    
    glBegin(GL_LINE_STRIP);
    float alphaStep = 1.0f / grain.trailPositions.size();
    float currentAlpha = grain.currentAlpha * 0.7f;
    
    for (size_t i = 0; i < grain.trailPositions.size(); ++i) {
        const auto& pos = grain.trailPositions[i];
        float alpha = currentAlpha * (1.0f - (i * alphaStep));
        juce::Colour c = grain.colour.withAlpha(alpha);
        glColor4f(c.getFloatRed(), c.getFloatGreen(), 
                 c.getFloatBlue(), c.getFloatAlpha());
        glVertex2f(pos.first, pos.second);
    }
    glEnd();
}

void VisualizationComponent::updateVisualFeedbacks() {
    // Update visual feedback animations
    }
}

void VisualizationComponent::resized() {
    // Update OpenGL viewport when component is resized
    if (openGLContext.isAttached()) {
        openGLContext.updateEmbeddedPosition(getLocalBounds());
    }
}

void VisualizationComponent::timerCallback() {
    // Request a repaint which will trigger renderOpenGL()
    if (isVisible()) {
        openGLContext.triggerRepaint();
    }
}

}  // namespace audio_plugin
