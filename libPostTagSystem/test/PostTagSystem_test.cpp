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
  State init{};
  auto result = evaluateNDTM({{1, 0, 1, 0, 1}}, 1, 6);
  ASSERT_EQ(result.terminationReason, TerminationReason::MaxEventsExceeded);
  ASSERT_EQ(result.maxEventCount, 1);
  ASSERT_EQ(result.totalStateCount, 2);
}

TEST(ndtm, multiEvent) {
  State init{};
  auto result = evaluateNDTM({{1, 0, 1, 0, 1}}, 3, 6);
  ASSERT_EQ(result.terminationReason, TerminationReason::MaxEventsExceeded);
  ASSERT_EQ(result.maxEventCount, 3);
  ASSERT_EQ(result.totalStateCount, 4);
}
}  // namespace NDTM
