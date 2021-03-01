#include <gtest/gtest.h>

#include <vector>

#include "PostTagHistory.hpp"
#include "PostTagMultihistory.hpp"

namespace PostTagSystem {
TEST(PostTagSystem, simpleEvolution) {
  TagState state;
  state.headState = 0;
  state.tape = {0, 0, 0};

  PostTagMultihistory system;
  system.addEvolutionStartingFromState(state);
  ASSERT_EQ(system.stateCount(), 10);
}

TEST(PostTagSystem, chunkEvaluationTable) {
  PostTagHistory history;
  ASSERT_EQ(
      history
          .evaluate(
              PostTagHistory::NamedRule::Post, {{1, 0, 1, 1, 1, 0, 1, 1, 1}, 2}, PostTagHistory::EvaluationLimits(0))
          .finalState.tape[8],
      1);
  ASSERT_EQ(
      history
          .evaluate(
              PostTagHistory::NamedRule::Post, {{1, 0, 1, 1, 1, 0, 1, 1, 1}, 2}, PostTagHistory::EvaluationLimits(8))
          .finalState.tape.size(),
      10);
  ASSERT_EQ(
      history
          .evaluate(PostTagHistory::NamedRule::Post, {{1, 0, 1, 1, 1, 0, 1, 1}, 0}, PostTagHistory::EvaluationLimits(0))
          .finalState.tape.size(),
      8);
  ASSERT_EQ(
      history
          .evaluate(
              PostTagHistory::NamedRule::Post, {{1, 1, 1, 1, 1, 1, 1, 1, 0}, 2}, PostTagHistory::EvaluationLimits(8))
          .finalState.tape.size(),
      12);
  ASSERT_EQ(history.evaluate(PostTagHistory::NamedRule::Post, {{}, 0}, PostTagHistory::EvaluationLimits(8))
                .finalState.tape.size(),
            0);
}

TEST(PostTagHistory, checkpoints) {
  PostTagHistory history;
  const TagState init = {{0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}, 0};
  constexpr uint64_t lateCheckpointEventCount = 20858000;
  const TagState lateCheckpoint = {{0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}, 2};
  const auto lateEvaluationResult = history.evaluate(PostTagHistory::NamedRule::Post,
                                                     init,
                                                     PostTagHistory::EvaluationLimits(10 * lateCheckpointEventCount),
                                                     {{lateCheckpoint}, {false}});
  ASSERT_EQ(lateEvaluationResult.eventCount, lateCheckpointEventCount);
  ASSERT_EQ(lateEvaluationResult.finalState.headState, lateCheckpoint.headState);
  ASSERT_EQ(lateEvaluationResult.finalState.tape, lateCheckpoint.tape);

  constexpr uint64_t earlyCheckpointEventCount = 20857000;
  const TagState earlyCheckpoint = {{1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1,
                                     1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1},
                                    1};
  const auto earlyEvaluationResult = history.evaluate(PostTagHistory::NamedRule::Post,
                                                      init,
                                                      PostTagHistory::EvaluationLimits(10 * lateCheckpointEventCount),
                                                      {{lateCheckpoint, earlyCheckpoint}, {false}});
  ASSERT_EQ(earlyEvaluationResult.eventCount, earlyCheckpointEventCount);
  ASSERT_EQ(earlyEvaluationResult.finalState.headState, earlyCheckpoint.headState);
  ASSERT_EQ(earlyEvaluationResult.finalState.tape, earlyCheckpoint.tape);
}

TEST(PostTagSystem, rule002211) {
  PostTagHistory history;
  const TagState init = {{0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0}, 0};
  constexpr uint64_t eventCount = 4;
  const auto evaluationResult =
      history.evaluate(PostTagHistory::NamedRule::Rule002211, init, PostTagHistory::EvaluationLimits(eventCount));
  ASSERT_EQ(evaluationResult.eventCount, 4);
  ASSERT_EQ(evaluationResult.finalState.headState, 1);
  std::vector<bool> expectedTape = {0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0};
  ASSERT_EQ(evaluationResult.finalState.tape, expectedTape);

  const TagState anotherInit = {{0, 0, 0, 1, 1, 0, 1, 0, 0, 0}, 0};
  const auto anotherEvaluationResult =
      history.evaluate(PostTagHistory::NamedRule::Rule002211, anotherInit, PostTagHistory::EvaluationLimits(4));
  ASSERT_EQ(anotherEvaluationResult.eventCount, 4);
  ASSERT_EQ(anotherEvaluationResult.finalState.headState, 1);
  std::vector<bool> anotherExpectedTape = {0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1};
  ASSERT_EQ(anotherEvaluationResult.finalState.tape, anotherExpectedTape);
}

TEST(PostTagSystem, maxTapeLength) {
  PostTagHistory evaluator;
  const TagState init(20, 123, 0);
  PostTagHistory::EvaluationLimits limits;
  limits.maxTapeLength = 52;
  const auto lengthLimitResult = evaluator.evaluate(PostTagHistory::NamedRule::Post, init, limits);
  const auto eventLimitResult =
      evaluator.evaluate(PostTagHistory::NamedRule::Post, init, PostTagHistory::EvaluationLimits(912));
  ASSERT_EQ(lengthLimitResult.conclusionReason, PostTagHistory::ConclusionReason::MaxTapeLengthExceeded);
  ASSERT_EQ(lengthLimitResult.eventCount, 912);
  ASSERT_EQ(lengthLimitResult.finalState, eventLimitResult.finalState);
  ASSERT_EQ(lengthLimitResult.maxIntermediateTapeLength, eventLimitResult.maxIntermediateTapeLength);
}
}  // namespace PostTagSystem
