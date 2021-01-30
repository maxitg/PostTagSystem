#include "NDTM.hpp"

#include <optional>
#include <unordered_set>

namespace NDTM {
class StateHasher {
 public:
  size_t operator()(const State& state) const {
    std::size_t result = 0;
    hash_combine(&result, state.headState);
    hash_combine(&result, state.headPosition);
    for (const auto tapeValue : state.tape) {
      hash_combine(&result, tapeValue);
    }
    return result;
  }

 private:
  // https://stackoverflow.com/a/2595226
  template <class T>
  static void hash_combine(std::size_t* seed, const T& value) {
    std::hash<T> hasher;
    *seed ^= hasher(value) + 0x9e3779b9 + (*seed << 6) + (*seed >> 2);
  }
};

class StateEquality {
 public:
  size_t operator()(const State& a, const State& b) const {
    if (a.headState != b.headState || a.headPosition != b.headPosition || a.headDisplacement != b.headDisplacement ||
        a.tape.size() != b.tape.size()) {
      return false;
    }

    const auto mismatchedIterators = std::mismatch(a.tape.begin(), a.tape.end(), b.tape.begin(), b.tape.end());
    return mismatchedIterators.first == a.tape.end() && mismatchedIterators.second == b.tape.end();
  }
};

std::optional<State> nextState(const Rule& rule, const State& init) {
  if (init.tape[init.headPosition] != rule.oldTapeState || init.headState != rule.oldHeadState) {
    return std::nullopt;
  }
  State newState = init;
  newState.tape[init.headPosition] = rule.newTapeState;
  newState.headState = rule.newHeadState;
  newState.headPosition = init.headPosition + rule.displacement;
  newState.headDisplacement = init.headDisplacement + rule.displacement;
  if (newState.headPosition == -1) {
    newState.tape.push_front(0);
    ++newState.headPosition;
  } else if (newState.headPosition == 1 && newState.tape[0] == 0) {
    newState.tape.pop_front();
    --newState.headPosition;
  }

  if (newState.headPosition == static_cast<int>(newState.tape.size())) {
    newState.tape.push_back(0);
  } else if (newState.headPosition == static_cast<int>(newState.tape.size()) - 2 &&
             newState.tape[static_cast<int>(newState.tape.size()) - 1] == 0) {
    newState.tape.pop_back();
  }
  return newState;
}

LifetimeData evaluateNDTM(const std::vector<Rule>& rules, int maxEventCountLimit, int totalStateCountLimit) {
  LifetimeData result;

  std::unordered_set<State, StateHasher, StateEquality> unevaluatedStates = {State()};
  std::unordered_set<State, StateHasher, StateEquality> allStates = {State()};

  int event;
  for (event = 0; !unevaluatedStates.empty() && event < maxEventCountLimit &&
                  static_cast<int>(allStates.size()) < totalStateCountLimit;
       ++event) {
    std::unordered_set<State, StateHasher, StateEquality> oldUnevaluatedStates = unevaluatedStates;
    unevaluatedStates.clear();
    for (const auto& state : oldUnevaluatedStates) {
      for (const auto& rule : rules) {
        const auto newState = nextState(rule, state);
        if (newState.has_value() && !allStates.count(newState.value())) {
          unevaluatedStates.insert(newState.value());
          allStates.insert(newState.value());
        }
      }
    }
  }

  if (unevaluatedStates.empty()) {
    result.terminationReason = TerminationReason::Terminated;
  } else if (event >= maxEventCountLimit) {
    result.terminationReason = TerminationReason::MaxEventsExceeded;
  } else {
    result.terminationReason = TerminationReason::MaxStatesExceeded;
  }

  result.maxEventCount = event;
  result.totalStateCount = static_cast<int>(allStates.size());

  return result;
}
}  // namespace NDTM
