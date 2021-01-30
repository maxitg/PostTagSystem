#include <gtest/gtest.h>

#include <vector>

#include "NDTM.hpp"
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

namespace NDTM {
TEST(ndtm, simple) {
  auto result = evaluateNDTM({{1, 0, 1, 0, 1}}, 1, 6);
  ASSERT_EQ(result.terminationReason, TerminationReason::MaxEventsExceeded);
  ASSERT_EQ(result.maxEventCount, 1);
  ASSERT_EQ(result.totalStateCount, 2);
}

TEST(ndtm, multiEvent) {
  auto result = evaluateNDTM({{1, 0, 1, 1, 1}, {1, 1, 1, 1, 1}}, 10000, 100000);
  ASSERT_EQ(result.terminationReason, TerminationReason::MaxEventsExceeded);
  ASSERT_EQ(result.maxEventCount, 10000);
  ASSERT_EQ(result.totalStateCount, 10001);
}
}  // namespace NDTM
