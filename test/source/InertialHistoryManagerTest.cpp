#include "Pointilsynth/InertialHistoryManager.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE("DecaysAndRemovesNotes", "[InertialHistoryManagerTest]") {
  InertialHistoryManager manager;
  manager.addNote(60, 1.0f, 0.0);

  manager.update(4.0, 4.0);  // one bar
  REQUIRE(manager.getNumNotes() == 1u);
  REQUIRE(manager.getNote(0).currentInfluence ==
          Catch::Approx(0.5f).margin(1e-5f));

  manager.update(16.0, 4.0);  // four bars
  REQUIRE(manager.getNumNotes() == 0u);
}

TEST_CASE("ModulationValueOscillatesAndDecays",
          "[InertialHistoryManagerTest]") {
  InertialHistoryManager manager;
  manager.addNote(60, 1.0f, 0.0);

  manager.update(1.0, 4.0);  // quarter note
  float first = manager.getModulationValue();
  REQUIRE(std::abs(first) > 0.001f);

  manager.update(3.0, 4.0);  // three quarter notes
  float second = manager.getModulationValue();
  REQUIRE(std::abs(second) > 0.001f);
  REQUIRE(first * second < 0.0f);  // oscillation should change sign

  manager.update(7.0, 4.0);  // nearly two bars
  float third = manager.getModulationValue();
  REQUIRE(std::abs(third) < std::abs(first));

  manager.update(16.0, 4.0);  // four bars, note removed
  float final = manager.getModulationValue();
  REQUIRE(final == Catch::Approx(0.0f).margin(1e-5f));
}
