#include <gtest/gtest.h>

#include <vector>

#include "PostTagHistory.hpp"
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

TEST(PostTagSystem, chunkEvaluationTable) {
  PostTagHistory history;
  ASSERT_EQ(history.evaluate({{1, 0, 1, 1, 1, 0, 1, 1, 1}, 2}, 0).tape[8], 1);
  ASSERT_EQ(history.evaluate({{1, 0, 1, 1, 1, 0, 1, 1, 1}, 2}, 8).tape.size(), 10);
  ASSERT_EQ(history.evaluate({{1, 0, 1, 1, 1, 0, 1, 1}, 0}, 0).tape.size(), 8);
  ASSERT_EQ(history.evaluate({{1, 1, 1, 1, 1, 1, 1, 1, 0}, 2}, 8).tape.size(), 12);
  ASSERT_EQ(history.evaluate({{}, 0}, 8).tape.size(), 0);
}
}  // namespace PostTagSystem
