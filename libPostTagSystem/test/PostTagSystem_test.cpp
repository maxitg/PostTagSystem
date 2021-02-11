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
  ASSERT_EQ(history.evaluate({{1, 0, 1, 1, 1, 0, 1, 1, 1}, 2}, 0).finalState.tape[8], 1);
  ASSERT_EQ(history.evaluate({{1, 0, 1, 1, 1, 0, 1, 1, 1}, 2}, 8).finalState.tape.size(), 10);
  ASSERT_EQ(history.evaluate({{1, 0, 1, 1, 1, 0, 1, 1}, 0}, 0).finalState.tape.size(), 8);
  ASSERT_EQ(history.evaluate({{1, 1, 1, 1, 1, 1, 1, 1, 0}, 2}, 8).finalState.tape.size(), 12);
  ASSERT_EQ(history.evaluate({{}, 0}, 8).finalState.tape.size(), 0);
}

TEST(PostTagHistory, checkpoints) {
  PostTagHistory history;
  const PostTagState init = {{0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}, 0};
  constexpr uint64_t lateCheckpointEventCount = 20858000;
  const PostTagState lateCheckpoint = {{0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}, 2};
  const auto lateEvaluationResult = history.evaluate(init, 10 * lateCheckpointEventCount, {lateCheckpoint});
  ASSERT_EQ(lateEvaluationResult.eventCount, lateCheckpointEventCount);
  ASSERT_EQ(lateEvaluationResult.finalState.headState, lateCheckpoint.headState);
  ASSERT_EQ(lateEvaluationResult.finalState.tape, lateCheckpoint.tape);

  constexpr uint64_t earlyCheckpointEventCount = 20857000;
  const PostTagState earlyCheckpoint = {{1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1,
                                         1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1},
                                        1};
  const auto earlyEvaluationResult =
      history.evaluate(init, 10 * lateCheckpointEventCount, {lateCheckpoint, earlyCheckpoint});
  ASSERT_EQ(earlyEvaluationResult.eventCount, earlyCheckpointEventCount);
  ASSERT_EQ(earlyEvaluationResult.finalState.headState, earlyCheckpoint.headState);
  ASSERT_EQ(earlyEvaluationResult.finalState.tape, earlyCheckpoint.tape);
}
}  // namespace PostTagSystem
