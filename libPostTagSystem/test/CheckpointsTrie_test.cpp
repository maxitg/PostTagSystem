#include "CheckpointsTrie.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace PostTagSystem {
namespace {
void checkStateInsertion(const std::vector<ChunkedState>& insertedStates,
                         const std::vector<ChunkedState>& missingStates) {
  CheckpointsTrie trie;
  for (auto insertionIt = insertedStates.begin(); insertionIt != insertedStates.end(); ++insertionIt) {
    trie.insert(*insertionIt);
    for (auto checkIt = insertedStates.begin(); checkIt != insertionIt + 1; ++checkIt) {
      ASSERT_TRUE(trie.contains(*checkIt));
    }
  }

  for (const auto& state : missingStates) {
    ASSERT_FALSE(trie.contains(state));
  }
}
}  // namespace

TEST(CheckpointsTrie, singleInsertion) {
  checkStateInsertion({{{1, 2, 3}, 10, 2}},
                      {{{1, 2, 4}, 10, 2}, {{2, 2, 3}, 10, 2}, {{1, 2, 3}, 2, 2}, {{1, 2, 3}, 10, 3}});
}

TEST(CheckpointsTrie, multipleInsertion) {
  checkStateInsertion({{{1, 2, 3}, 10, 2},
                       {{1, 2, 3}, 20, 2},
                       {{2, 2, 3}, 10, 2},
                       {{3, 2, 3}, 10, 2},
                       {{1, 2, 3, 4}, 10, 2},
                       {{2, 4, 3}, 10, 2},
                       {{1, 2, 4}, 10, 2}},
                      {{{4, 2, 3}, 10, 2},
                       {{1, 2, 3}, 30, 2},
                       {{1, 2, 3}, 20, 3},
                       {{1, 2, 3, 4, 5}, 10, 2},
                       {{1, 3, 3}, 10, 2},
                       {{1, 2, 5}, 10, 2},
                       {{1, 2, 3, 3}, 10, 2}});
}

TEST(CheckpointsTrie, emptyStates) { checkStateInsertion({{{}, 0, 2}}, {{{}, 0, 3}}); }
}  // namespace PostTagSystem
