#include "PostTagSearcher.hpp"

#include <gtest/gtest.h>

#include <vector>

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

PostTagSearcher::EvaluationParameters defaultParameters() {
  PostTagSearcher::EvaluationParameters parameters;
  parameters.checkpoints = {};
  parameters.groupTimeConstraintNs = std::numeric_limits<uint64_t>::max();
  parameters.maxEventCount = std::numeric_limits<uint64_t>::max() - 7;
  parameters.maxTapeLength = std::numeric_limits<uint64_t>::max();
  return parameters;
}

TEST(PostTagSearcher, rangeOfTagStates) {
  PostTagSearcher searcher;
  const auto result = searcher.evaluateRange(
      {{0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, 0}, {{0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0}, 1}, defaultParameters());
  ASSERT_EQ(result.size(), 106);
}
}  // namespace PostTagSystem
