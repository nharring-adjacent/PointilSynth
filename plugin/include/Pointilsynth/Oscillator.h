#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h> // For juce::Random
#include <cmath> // For std::sin, std::fmod

namespace Pointilsynth
{

class Oscillator
{
public:
    enum class Waveform
    {
        Sine,
        Saw,
        Square,
        Noise
    };

    Oscillator() :
        currentWaveform(Waveform::Sine),
        sampleRate(44100.0),
        frequency(440.0f),
        tableSize(128) // Default table size for JUCE oscillator
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = 512;
        spec.numChannels = 1;

        // Initialise with sine by default
        osc.initialise([](float x){ return std::sin(x); }, tableSize);
        osc.prepare(spec);
        osc.setFrequency(frequency, true);
    }

    void setWaveform(Waveform newWaveform)
    {
        currentWaveform = newWaveform;
        // Re-initialise the oscillator with the correct function for the new waveform
        // For Noise, we don't use the juce::dsp::Oscillator in the same way.
        if (currentWaveform == Waveform::Sine)
        {
            osc.initialise([](float x){ return std::sin(x); }, tableSize);
        }
        else if (currentWaveform == Waveform::Saw)
        {
            // Naive saw: f(x) = x / pi. JUCE dsp::Oscillator expects function over 2*pi range, scaled.
            // The phase argument 'x' to the lambda in juce::dsp::Oscillator
            // is expected to be the current phase * 2 * PI.
            // So, x goes from 0 to 2*PI.
            // A sawtooth wave normally goes from -1 to 1.
            // x / PI - 1 would map [0, 2PI] to [-1, 1]
            osc.initialise([](float x){ return (x / juce::MathConstants<float>::pi) - 1.0f; }, tableSize);
        }
        else if (currentWaveform == Waveform::Square)
        {
            // x goes from 0 to 2*PI.
            // Square: +1 for first half (0 to PI), -1 for second half (PI to 2*PI)
            osc.initialise([](float x){ return (x < juce::MathConstants<float>::pi) ? 1.0f : -1.0f; }, tableSize);
        }
        // For Noise, osc is not used, so no specific initialisation needed here for it.
        // Ensure frequency is set after re-initialising, as initialise might reset it.
        // However, setFrequency is already called in setSampleRate and constructor.
        // If only waveform changes, frequency should persist.
        // Let's call setFrequency again to be safe, as per JUCE examples.
        if (currentWaveform != Waveform::Noise) {
            osc.setFrequency(frequency, true);
        }
    }

    void setSampleRate(double newSampleRate)
    {
        if (sampleRate != newSampleRate)
        {
            sampleRate = newSampleRate;

            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = 512;
            spec.numChannels = 1;
            osc.prepare(spec);

            // Re-apply frequency and re-initialize waveform function context
            // because `prepare` might reset some internal state or require it.
            setWaveform(currentWaveform); // This will call initialise and setFrequency for relevant waveforms
        }
    }

    void setFrequency(float newFrequency)
    {
        if (frequency != newFrequency)
        {
            frequency = newFrequency;
            if (currentWaveform != Waveform::Noise)
            {
                osc.setFrequency(frequency, true);
            }
        }
    }

    float getNextSample()
    {
        switch (currentWaveform)
        {
            case Waveform::Sine:
            case Waveform::Saw:
            case Waveform::Square:
                // The juce::dsp::Oscillator's processSample method takes no arguments
                // and advances its internal phase, returning the next sample.
                return osc.processSample(0.0f); // Argument is for FM, 0.0f for no FM
            case Waveform::Noise:
                // Generate random float between -1.0 and 1.0
                return random.nextFloat() * 2.0f - 1.0f;
            default:
                return 0.0f;
        }
    }

private:
    juce::dsp::Oscillator<float> osc;
    Waveform currentWaveform;
    double sampleRate;
    float frequency;
    juce::Random random;

    int tableSize; // Store table size for re-initialisation
};

} // namespace Pointilsynth
