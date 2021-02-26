#include "CheckpointsTrie.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace PostTagSystem {
TEST(CheckpointsTrie, singleInsertion) {
  CheckpointsTrie trie;
  trie.insert({{1, 2, 3}, 10, 2});
  ASSERT_TRUE(trie.contains({{1, 2, 3}, 10, 2}));
  ASSERT_FALSE(trie.contains({{1, 2, 4}, 10, 2}));
  ASSERT_FALSE(trie.contains({{2, 2, 3}, 10, 2}));
  ASSERT_FALSE(trie.contains({{1, 2, 3}, 2, 2}));
  ASSERT_FALSE(trie.contains({{1, 2, 3}, 10, 3}));
}

namespace {
void checkStateInsertion(const std::vector<ChunkedState>& insertedStates,
                         const std::vector<ChunkedState>& missingStates) {
  CheckpointsTrie trie;
  for (auto insertionIt = insertedStates.begin(); insertionIt != insertedStates.end(); ++insertionIt) {
    trie.insert(*insertionIt);
    for (auto checkIt = insertedStates.begin(); checkIt != insertionIt; ++checkIt) {
      ASSERT_TRUE(trie.contains(*checkIt));
    }
  }

  for (const auto& state : missingStates) {
    ASSERT_FALSE(trie.contains(state));
  }
}
}  // namespace

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
}  // namespace PostTagSystem
