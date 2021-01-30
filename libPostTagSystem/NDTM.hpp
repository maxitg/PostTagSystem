#ifndef LIBPOSTTAGSYSTEM_NDTM_HPP_
#define LIBPOSTTAGSYSTEM_NDTM_HPP_

#include <deque>
#include <vector>

namespace NDTM {
struct Rule {
  int oldHeadState;
  int oldTapeState;
  int newHeadState;
  int newTapeState;
  int displacement;
};

struct State {
  int headState;
  int headPosition;
  int headDisplacement;
  std::deque<int> tape;

  State() {
    headState = 1;
    headPosition = 0;
    headDisplacement = 0;
    tape = {0};
  }
};

enum class TerminationReason { Terminated, MaxEventsExceeded, MaxStatesExceeded };

struct LifetimeData {
  TerminationReason terminationReason;
  int maxEventCount;
  int totalStateCount;
};

LifetimeData evaluateNDTM(const std::vector<Rule>& rules, int maxEventCountLimit, int totalStateCountLimit);
}  // namespace NDTM

#endif  // LIBPOSTTAGSYSTEM_NDTM_HPP_
