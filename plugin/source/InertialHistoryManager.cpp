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
  notes_.push_back(note);
}

void InertialHistoryManager::update(double currentPpq, double ppqPerBar) {
  for (auto it = notes_.begin(); it != notes_.end();) {
    double ageInBars = (currentPpq - it->startPpq) / ppqPerBar;
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
  return 0.0f;
}
