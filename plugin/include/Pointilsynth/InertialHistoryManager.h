#pragma once

#include <vector>

struct InertialNote {
  int noteNumber{};
  float initialInfluence{};
  float currentInfluence{};
  double startPpq{};
  double ageInBars{};
};

class InertialHistoryManager {
public:
  void addNote(int noteNumber, float velocity, double currentPpq);
  void update(double currentPpq, double ppqPerBar);
  float getModulationValue();

  size_t getNumNotes() const { return notes_.size(); }
  const InertialNote& getNote(size_t index) const { return notes_.at(index); }

private:
  std::vector<InertialNote> notes_;
};
