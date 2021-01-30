#ifndef LIBPOSTTAGSYSTEM_NDTM_HPP_
#define LIBPOSTTAGSYSTEM_NDTM_HPP_

#include <array>
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
  std::vector<int> tape;

  State(int size) {
    headState = 1;
    headPosition = size + 1;
    tape = std::vector<int>(2*size + 1, 0);
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
