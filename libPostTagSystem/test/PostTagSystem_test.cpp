#include <gtest/gtest.h>

#include <vector>

#include "PostTagMultihistory.hpp"

namespace PostTagSystem {
TEST(PostTagSystem, simpleEvolution) {
  PostTagState state;
  state.headState = 0;
  state.tape = {0, 0, 0};

  PostTagMultihistory system;
  system.addEvolutionStartingFromState(state);
  ASSERT_EQ(system.stateCount(), 10);
}
}  // namespace PostTagSystem
