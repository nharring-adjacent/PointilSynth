#include "Pointilsynth/InertialHistoryManager.h"
#include <gtest/gtest.h>

TEST(InertialHistoryManagerTest, DecaysAndRemovesNotes) {
  InertialHistoryManager manager;
  manager.addNote(60, 1.0f, 0.0);

  manager.update(4.0, 4.0);  // one bar
  ASSERT_EQ(manager.getNumNotes(), 1u);
  EXPECT_NEAR(manager.getNote(0).currentInfluence, 0.5f, 1e-5f);

  manager.update(16.0, 4.0);  // four bars
  EXPECT_EQ(manager.getNumNotes(), 0u);
}
