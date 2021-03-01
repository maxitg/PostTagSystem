#include "PostTagSearcher.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "PostTagHistory.hpp"

namespace PostTagSystem {
TEST(PostTagSearcher, emptyCases) {
  PostTagSearcher searcher;
  ASSERT_EQ(searcher
                .evaluateRange({{0, 0, 0, 0, 1, 0, 1, 0, 1}, 1},
                               {{0, 0, 0, 0, 1, 0, 1, 0, 1}, 1},
                               PostTagSearcher::EvaluationParameters())
                .size(),
            0);

  ASSERT_EQ(searcher.evaluateRange(10, 51341, 51341, PostTagSearcher::EvaluationParameters()).size(), 0);

  ASSERT_EQ(searcher.evaluateGroup(std::vector<TagState>(), PostTagSearcher::EvaluationParameters()).size(), 0);

  ASSERT_EQ(searcher.evaluateGroup(std::vector<PostTagSearcher::SmallState>(), PostTagSearcher::EvaluationParameters())
                .size(),
            0);
}

void compareResults(const TagState& init,
                    const PostTagSearcher::EvaluationResult& result,
                    uint64_t eventLimit = std::numeric_limits<uint64_t>::max() - 7) {
  PostTagHistory singleHistoryEvaluator;
  const auto singleResult =
      singleHistoryEvaluator.evaluate(PostTagHistory::NamedRule::Post, init, eventLimit, {{}, {true}});

  if (singleResult.conclusionReason == PostTagHistory::ConclusionReason::InvalidInput) {
    ASSERT_EQ(result.conclusionReason, PostTagSearcher::ConclusionReason::InvalidInput);
  } else if (singleResult.conclusionReason == PostTagHistory::ConclusionReason::Terminated) {
    ASSERT_EQ(result.conclusionReason, PostTagSearcher::ConclusionReason::Terminated);
  } else if (singleResult.conclusionReason == PostTagHistory::ConclusionReason::ReachedCheckpoint) {
    ASSERT_EQ(result.conclusionReason, PostTagSearcher::ConclusionReason::ReachedCycle);
  } else if (singleResult.conclusionReason == PostTagHistory::ConclusionReason::MaxEventCountExceeded) {
    ASSERT_EQ(result.conclusionReason, PostTagSearcher::ConclusionReason::MaxEventCountExceeded);
  }

  ASSERT_EQ(result.finalState, singleResult.finalState);
  ASSERT_EQ(result.eventCount, singleResult.eventCount);
  ASSERT_EQ(result.maxTapeLength, singleResult.maxTapeLength);
  ASSERT_EQ(result.finalTapeLength, singleResult.finalState.tape.size());
  ASSERT_EQ(result.initialState, init);
}

TEST(PostTagSearcher, rangeOfTagStates) {
  PostTagSearcher searcher;
  const auto result = searcher.evaluateRange({{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0},
                                             {{0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0}, 1},
                                             PostTagSearcher::EvaluationParameters());
  ASSERT_EQ(result.size(), 106);

  compareResults(TagState({0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1}, 0), result[48]);
  compareResults(TagState({0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1}, 1), result[49]);
  compareResults(TagState({0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1}, 0), result[54]);
}

TEST(PostTagSearcher, rangeOfSmallStates) {
  PostTagSearcher searcher;
  const auto smallStateResults = searcher.evaluateRange(13, 725, 760, PostTagSearcher::EvaluationParameters());
  const auto fullStateResults = searcher.evaluateRange({{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0},
                                                       {{0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0}, 0},
                                                       PostTagSearcher::EvaluationParameters());
  ASSERT_EQ(smallStateResults, fullStateResults);
}

TEST(PostTagSearcher, listOfTagStates) {
  PostTagSearcher searcher;
  const auto rangeResult = searcher.evaluateRange({{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0},
                                                  {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0}, 1},
                                                  PostTagSearcher::EvaluationParameters());
  const auto groupResult = searcher.evaluateGroup({{{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0},
                                                   {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 1},
                                                   {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 2},
                                                   {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0}, 0}},
                                                  PostTagSearcher::EvaluationParameters());
  ASSERT_EQ(rangeResult, groupResult);
}

TEST(PostTagSearcher, listOfSmallStates) {
  PostTagSearcher searcher;
  const auto rangeResult = searcher.evaluateRange(20, 1322, 1324, PostTagSearcher::EvaluationParameters());
  const auto groupResult = searcher.evaluateGroup(
      std::vector<PostTagSearcher::SmallState>(
          {{1322, 20, 0}, {1322, 20, 1}, {1322, 20, 2}, {1323, 20, 0}, {1323, 20, 1}, {1323, 20, 2}}),
      PostTagSearcher::EvaluationParameters());
}

TEST(PostTagSearcher, eventLimit) {
  PostTagSearcher searcher;
  PostTagSearcher::EvaluationParameters parameters;
  parameters.maxEventCount = 104;
  const auto result = searcher.evaluateRange(
      {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0}, {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 2}, parameters);
  ASSERT_EQ(result.size(), 2);

  compareResults(TagState({0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0), result[0], 104);
  compareResults(TagState({0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 1), result[1], 104);
}
}  // namespace PostTagSystem
