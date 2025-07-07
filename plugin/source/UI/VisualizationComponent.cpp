#include "UI/VisualizationComponent.h"

// Include JUCE OpenGL first to avoid macro redefinition issues
#include <juce_opengl/juce_opengl.h>

// JUCE OpenGL includes all necessary OpenGL headers
// No need for GLEW or GLFW as JUCE provides the necessary OpenGL context

// Include standard OpenGL headers after JUCE
#if JUCE_OPENGL_ES
    #include <GLES3/gl3.h>
#elif JUCE_MAC
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

namespace {
    // Vertex shader source



    // Fragment shader source


}

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
        g.addColour(static_cast<double>(0.0f), juce::Colours::red);
        g.addColour(static_cast<double>(0.2f), juce::Colours::orange);
        g.addColour(static_cast<double>(0.4f), juce::Colours::yellow);
        g.addColour(static_cast<double>(0.6f), juce::Colours::green);
        g.addColour(static_cast<double>(0.8f), juce::Colours::blue);
        g.addColour(static_cast<double>(1.0f), juce::Colours::violet);
        return g;
    }
    
    const juce::ColourGradient createMinimalistGradient() {
        juce::ColourGradient g;
        g.addColour(static_cast<double>(0.0f), juce::Colour::fromFloatRGBA(0.9f, 0.9f, 0.9f, 0.8f));
        g.addColour(static_cast<double>(1.0f), juce::Colour::fromFloatRGBA(0.7f, 0.7f, 0.7f, 0.6f));
        return g;
    }
    
    const juce::ColourGradient createRetroGradient() {
        juce::ColourGradient g;
        g.addColour(static_cast<double>(0.0f), juce::Colours::red);
        g.addColour(static_cast<double>(0.5f), juce::Colours::yellow);
        g.addColour(static_cast<double>(1.0f), juce::Colours::lime);
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
    
    const juce::CriticalSection::ScopedLockType lock(particleLock);
    activeFeedbacks_.push_back(feedback);
}

void VisualizationComponent::updateVisualFeedbacks() {
    auto now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
(void)now; // suppress unused variable warning
    
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

// Initialize static shader source strings
const char* VisualizationComponent::vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 position;
    layout (location = 1) in vec4 color;
    layout (location = 2) in float size;
    out vec4 fragColor;
    void main() {
        gl_Position = vec4(position.x * 2.0 - 1.0, 1.0 - position.y * 2.0, 0.0, 1.0);
        gl_PointSize = size;
        fragColor = color;
    }
)";

const char* VisualizationComponent::fragmentShaderSource = R"(
    #version 330 core
    in vec4 fragColor;
    out vec4 outColor;
    void main() {
        outColor = fragColor;
        // Make particles round
        vec2 coord = gl_PointCoord - vec2(0.5);
        if (length(coord) > 0.5) {
            discard;
        }
    }
)";

VisualizationComponent::VisualizationComponent(juce::AudioProcessorValueTreeState& apvts)
    : juce::Component("VisualizationComponent"),
      juce::Timer(),
#if ! defined (JUCE_HEADLESS_TESTING)
      openGLContext(),
#endif
      openGLAvailable(false),
      apvts_(apvts),
      shaderProgram(),
      particleVBO(0),
      particleVAO(0),
      particleLock(),
      grains(),
      lastUpdateTime(juce::Time::getMillisecondCounterHiRes() / 1000.0) {
    setOpaque(true);
    setSize(800, 600);
    
#if ! defined (JUCE_HEADLESS_TESTING)
    // Try to initialize OpenGL with error handling
    try {
        juce::OpenGLPixelFormat format;
        format.depthBufferBits = 24;
        format.stencilBufferBits = 8;
        format.multisamplingLevel = 4;
        
        openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
        openGLContext.setPixelFormat(format);
        openGLContext.setRenderer(this);
        
        // Try to attach OpenGL context
        openGLContext.attachTo(*this);
        
        // Check if OpenGL context is properly attached
        if (openGLContext.isAttached()) {
            openGLContext.setContinuousRepainting(true);
            openGLAvailable = true;
            startTimerHz(60); // 60 FPS for visual updates
        } else {
            DBG("VisualizationComponent: Failed to attach OpenGL context, falling back to software rendering");
            openGLAvailable = false;
        }
    } catch (...) {
        DBG("VisualizationComponent: Exception during OpenGL initialization, falling back to software rendering");
        openGLAvailable = false;
    }
#else
    // In headless testing, always disable OpenGL
    openGLAvailable = false;
#endif
}

VisualizationComponent::~VisualizationComponent() {
    stopTimer();
    
#if ! defined (JUCE_HEADLESS_TESTING)
    // Release OpenGL resources if available
    if (openGLAvailable && openGLContext.isAttached()) {
        openGLContext.detach();
    }
#endif
}

#if ! defined (JUCE_HEADLESS_TESTING)
void VisualizationComponent::renderOpenGL() {
    jassert(juce::OpenGLHelpers::isContextActive());
    
    // Set up the viewport
    // Set up the viewport
    const float renderingScale = static_cast<float>(openGLContext.getRenderingScale());
    const auto width = static_cast<int>(renderingScale * static_cast<float>(getWidth()));
    const auto height = static_cast<int>(renderingScale * static_cast<float>(getHeight()));
    // Use getPrimaryDisplay instead of deprecated getMainDisplay, and use width/height directly
    auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    juce::Rectangle<int> screen(display->userArea.getX(), display->userArea.getY(), display->userArea.getWidth(), display->userArea.getHeight());
    juce::OpenGLHelpers::clear(juce::Colours::black);
    juce::gl::glViewport(0, 0, width, height);
    
    // Clear the background
    juce::OpenGLHelpers::clear(juce::Colours::black);
    
    // Update and render particles
    updateParticles();
    renderParticles();
    
    // Reset the element buffers so child Components draw correctly
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);
}
#endif


void VisualizationComponent::updateParticles() {
    const double now = currentTimeSeconds();
    const double deltaTime = now - lastUpdateTime;
    lastUpdateTime = now;
    
    // Update existing particles
    for (auto& grain : grains) {
        // Uniform descent
        grain.y += static_cast<float>(static_cast<double>(grain.velocityY) * deltaTime);
        
        // Apply damping to horizontal velocity
        grain.velocityX *= 0.99f;
        grain.x += static_cast<float>(static_cast<double>(grain.velocityX) * deltaTime);
        
        // Calculate age-based properties for pop animation
        float age = static_cast<float>((now - grain.startTime) / grain.maxAge);
        float popThreshold = 0.8f; // Start popping when 80% of life is gone
        
        if (age < popThreshold) {
            grain.currentSize = grain.size; // No size change until pop
            grain.currentAlpha = 1.0f - juce::jlimit(0.0f, 1.0f, age * 0.5f); // Gradual fade
        } else {
            float popProgress = juce::jmap(age, popThreshold, 1.0f, 0.0f, 1.0f);
            grain.currentSize = grain.size * (1.0f + popProgress * 0.5f); // Increase size by 50%
            grain.currentAlpha = 1.0f - popProgress; // Fade out completely during pop
        }
    }
    
    // Remove dead particles
    grains.erase(std::remove_if(grains.begin(), grains.end(),
                              [now](const VisualGrain& g) {
                                  return (now - g.startTime) > g.maxAge;
                              }),
               grains.end());
    
    // Add new grains from audio thread
    // addNewGrains(); // TODO: Implement this method
}

void VisualizationComponent::addGrainInfo(const GrainInfoForVis& info) {
    const double now = currentTimeSeconds();

    VisualGrain grain;
    grain.x = juce::jmap(info.pan, -1.0f, 1.0f, 0.0f, 1.0f); // Map pan to 0.0-1.0 for visualization
    grain.y = 0.0f; // Start at the top
    grain.pitch = info.pitch;
    grain.size = 6.0f + info.velocity * 10.0f; // Size based on velocity
    grain.velocityX = (static_cast<float>(rand() % 100 - 50) * 0.0005f); // Very subtle random horizontal velocity
    grain.velocityY = 0.5f; // Constant downward velocity for proportional travel
    grain.startTime = now;
    grain.maxAge = static_cast<double>(info.durationSeconds) * 2.0; // Visual life proportional to audio duration
    grain.colour = getColorForPitch(info.pitch, info.velocity); // Use existing color function
    grain.currentSize = grain.size;
    grain.currentAlpha = 1.0f;
    
    // Add to particles with thread safety
    const juce::CriticalSection::ScopedLockType lock(particleLock);
    grains.push_back(grain);
}



void VisualizationComponent::renderParticles() {
    if (!shaderProgram) {
        initializeShaders();
        if (!shaderProgram) {
            return; // Shader initialization failed
        }
    }
    
    // Update particle data
    updateParticleBuffers();
    
    // Draw particles
    {
        const juce::CriticalSection::ScopedLockType lock(particleLock);
        
        if (particleVBO != 0 && !grains.empty()) {
            // Use shader program
            shaderProgram->use();
            
            // Bind VAO
            openGLContext.extensions.glBindVertexArray(particleVAO);
            
            // Enable point sprites and point size control in shaders
            juce::gl::glEnable(juce::gl::GL_PROGRAM_POINT_SIZE);
            
            // Enable blending
            juce::gl::glEnable(juce::gl::GL_BLEND);
            juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
            
            // Draw points
            juce::gl::glDrawArrays(juce::gl::GL_POINTS, 0, static_cast<GLsizei>(grains.size()));
            
            // Clean up
            juce::gl::glDisable(juce::gl::GL_BLEND);
            juce::gl::glBindVertexArray(0);
        }
    }
}

void VisualizationComponent::resized() {
#if ! defined (JUCE_HEADLESS_TESTING)
    // Update OpenGL viewport when component is resized
    if (openGLAvailable && openGLContext.isAttached()) {
        // No need to call updateEmbeddedPosition; OpenGL viewport will be updated automatically in renderOpenGL
    }
#endif
    // In software mode, no special handling needed - repaint will handle new size
}

void VisualizationComponent::timerCallback() {
    if (isVisible()) {
        if (openGLAvailable) {
#if ! defined (JUCE_HEADLESS_TESTING)
            // Update particles and request OpenGL repaint
            updateParticles();
            openGLContext.triggerRepaint();
#endif
        } else {
            // In software mode, update particles and request normal repaint
            updateParticles();
            repaint();
        }
    }
}

void VisualizationComponent::paint(juce::Graphics& g)
{
    // When OpenGL is not available, provide software rendering fallback
    if (!openGLAvailable) {
        // Fill background
        g.fillAll(juce::Colour(0xff1a1a1a));
        
        // Draw particles using software rendering
        const juce::CriticalSection::ScopedLockType lock(particleLock);
        for (const auto& grain : grains) {
            // Convert normalized coordinates to screen coordinates
            float x = juce::jmap(grain.x, -1.0f, 1.0f, 0.0f, static_cast<float>(getWidth()));
            float y = juce::jmap(grain.y, 0.0f, 1.0f, static_cast<float>(getHeight()), 0.0f);
            
            // Set color and alpha
            g.setColour(grain.colour.withAlpha(grain.currentAlpha));
            
            // Draw particle as a circle
            float radius = grain.currentSize * 0.5f;
            g.fillEllipse(x - radius, y - radius, grain.currentSize, grain.currentSize);
        }
        
        // Draw a simple message indicating software mode
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.setFont(12.0f);
        g.drawText("Software Rendering", getLocalBounds().removeFromBottom(20), 
                   juce::Justification::centredRight, true);
    }
    
    // Always draw JUCE-based overlays (meters, feedback, etc.)
    drawVisualFeedbacks(g);
    drawMeters(g);
}

#if ! defined (JUCE_HEADLESS_TESTING)
void VisualizationComponent::newOpenGLContextCreated()
{
    if (!openGLAvailable) return;
    
    // Initialize OpenGL resources (shaders, buffers, etc.)
    initializeShaders();
    // Create VBO/VAO if not already created
    if (particleVBO == 0)
    {
        openGLContext.extensions.glGenBuffers(1, &particleVBO);
    }
    if (particleVAO == 0)
    {
        openGLContext.extensions.glGenVertexArrays(1, &particleVAO);
    }
}

void VisualizationComponent::openGLContextClosing()
{
#if ! defined (JUCE_HEADLESS_TESTING)
    if (!openGLAvailable) return;
    
    // Clean up OpenGL resources
    if (particleVBO != 0) {
        openGLContext.extensions.glDeleteBuffers(1, &particleVBO);
        particleVBO = 0;
    }
    if (particleVAO != 0) {
        openGLContext.extensions.glDeleteVertexArrays(1, &particleVAO);
        particleVAO = 0;
    }
    shaderProgram.reset();
#endif
}
#else
// Headless implementations - do nothing
void VisualizationComponent::newOpenGLContextCreated() {}
void VisualizationComponent::renderOpenGL() {}
#endif

void VisualizationComponent::drawMeters(juce::Graphics& g) {
    if (!showMeters_) return;

    const int meterWidth = 10;
    const int meterHeight = getHeight() - 20;
    const int meterX = getWidth() - meterWidth - 10;
    const int meterY = 10;

    // Draw left meter
    g.setColour(juce::Colours::grey);
    g.drawRect(meterX - meterWidth - 5, meterY, meterWidth, meterHeight);
    g.setColour(juce::Colours::green);
    g.fillRect(meterX - meterWidth - 5, meterY + static_cast<int>(meterHeight * (1.0f - leftMeterLevel_)), meterWidth, static_cast<int>(meterHeight * leftMeterLevel_));

    // Draw right meter
    g.setColour(juce::Colours::grey);
    g.drawRect(meterX, meterY, meterWidth, meterHeight);
    g.setColour(juce::Colours::green);
    g.fillRect(meterX, meterY + static_cast<int>(meterHeight * (1.0f - rightMeterLevel_)), meterWidth, static_cast<int>(meterHeight * rightMeterLevel_));
}

void VisualizationComponent::setGravity(float newGravity) {
    gravity_ = newGravity;
}

void VisualizationComponent::setColorScheme(const juce::ColourGradient& gradient) {
    colorGradient = gradient;
}

void VisualizationComponent::setTrailLength(int numFrames) {
    trailLength_ = numFrames;
}

void VisualizationComponent::setParticleSize(float minSize, float maxSize) {
    minParticleSize_ = minSize;
    maxParticleSize_ = maxSize;
}

void VisualizationComponent::initializeShaders() {
    shaderProgram = std::make_unique<juce::OpenGLShaderProgram>(openGLContext);
    if (!shaderProgram->addVertexShader(vertexShaderSource) || !shaderProgram->addFragmentShader(fragmentShaderSource) || !shaderProgram->link()) {
        shaderProgram.reset();
    }
}

void VisualizationComponent::setParticleOpacity(float opacity) {
    particleOpacity_ = opacity;
}

void VisualizationComponent::updateParticleBuffers() {
    if (grains.empty()) return;

    const juce::CriticalSection::ScopedLockType lock(particleLock);
    
    std::vector<float> vertexData;
    vertexData.reserve(grains.size() * 7);

    for (const auto& grain : grains) {
        vertexData.push_back(grain.x);
        vertexData.push_back(grain.y);
        vertexData.push_back(grain.colour.getFloatRed());
        vertexData.push_back(grain.colour.getFloatGreen());
        vertexData.push_back(grain.colour.getFloatBlue());
        vertexData.push_back(grain.colour.getFloatAlpha() * grain.currentAlpha);
        vertexData.push_back(grain.currentSize);
    }

    openGLContext.extensions.glBindVertexArray(particleVAO);
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, particleVBO);
    openGLContext.extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexData.size() * sizeof(float)), vertexData.data(), juce::gl::GL_DYNAMIC_DRAW);

    // Position
    openGLContext.extensions.glVertexAttribPointer(0, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 7 * sizeof(float), nullptr);
    openGLContext.extensions.glEnableVertexAttribArray(0);
    // Color
    openGLContext.extensions.glVertexAttribPointer(1, 4, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 7 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    openGLContext.extensions.glEnableVertexAttribArray(1);
    // Size
    openGLContext.extensions.glVertexAttribPointer(2, 1, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 7 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    openGLContext.extensions.glEnableVertexAttribArray(2);

    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindVertexArray(0);
}

}  // namespace audio_plugin

