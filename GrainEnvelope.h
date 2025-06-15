#pragma once

#include <cmath>
#include <numbers>

class GrainEnvelope {
public:
  enum class Shape { Trapezoid, Hann };

  void setShape(Shape newShape) { shape_ = newShape; }

  float getAmplitude(int currentSample, int totalDuration) const {
    if (currentSample < 0 || totalDuration <= 0 ||
        currentSample >= totalDuration) {
      return 0.0f;
    }

    switch (shape_) {
      case Shape::Hann: {
        float n = static_cast<float>(currentSample);
        float N = static_cast<float>(totalDuration);
        return 0.5f *
               (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * n / N));
      }
      case Shape::Trapezoid: {
        float pos = static_cast<float>(currentSample) /
                    static_cast<float>(totalDuration);
        if (pos < 0.1f) {
          return pos / 0.1f;
        } else if (pos < 0.9f) {
          return 1.0f;
        }
        return 1.0f - (pos - 0.9f) / 0.1f;
      }
    }
    return 0.0f;
  }

private:
  Shape shape_{Shape::Trapezoid};
};
