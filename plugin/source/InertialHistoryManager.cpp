#include "Pointilsynth/InertialHistoryManager.h"

#include <algorithm>
#include <cmath>

void InertialHistoryManager::addNote(int noteNumber,
                                     float velocity,
                                     double currentPpq) {
  InertialNote note;
  note.noteNumber = noteNumber;
  note.initialInfluence = velocity;
  note.currentInfluence = velocity;
  note.startPpq = currentPpq;
  note.ageInBars = 0.0;
  notes_.push_back(note);
}

void InertialHistoryManager::update(double currentPpq, double ppqPerBar) {
  for (auto it = notes_.begin(); it != notes_.end();) {
    double ageInBars = (currentPpq - it->startPpq) / ppqPerBar;
    it->ageInBars = ageInBars;
    it->currentInfluence =
        it->initialInfluence * static_cast<float>(std::pow(0.5, ageInBars));
    if (it->currentInfluence < it->initialInfluence * 0.125f) {
      it = notes_.erase(it);
    } else {
      ++it;
    }
  }
}

float InertialHistoryManager::getModulationValue() {
  float totalModulation = 0.0f;
  for (const auto& note : notes_) {
    float lfo = std::sin(static_cast<float>(note.ageInBars) * 2.0f *
                         static_cast<float>(M_PI)) *
                note.currentInfluence;
    totalModulation += lfo;
  }
  return totalModulation;
}
