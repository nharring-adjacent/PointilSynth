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
