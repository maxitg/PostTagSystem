#include "NDTM.hpp"

#include <algorithm>
#include <optional>
#include <queue>
#include <set>
#include <unordered_set>

namespace NDTM {
/*class StateHasher {
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
};*/

bool stateCompare(const State& a, const State& b) {
  if (a.headPosition != b.headPosition) {
    return a.headPosition < b.headPosition;
  } else if (a.headState != b.headState) {
    return a.headState < b.headState;
  } else {
    for (int i = 0; i < static_cast<int>(a.tape.size()); ++i) {
      if (a.tape[i] != b.tape[i]) {
        return a.tape[i] < b.tape[i];
      }
    }
  }
  return false;
}

std::optional<State> nextState(const Rule& rule, const State& init) {
  if (init.tape[init.headPosition] != rule.oldTapeState || init.headState != rule.oldHeadState) {
    return std::nullopt;
  }
  State newState = init;
  newState.tape[init.headPosition] = rule.newTapeState;
  newState.headState = rule.newHeadState;
  newState.headPosition = init.headPosition + rule.displacement;
  return newState;
}

LifetimeData evaluateNDTM(const std::vector<Rule>& rules, int maxEventCountLimit, int totalStateCountLimit) {
  LifetimeData result;

  std::set<State, decltype(stateCompare)*> allStates(stateCompare);
  allStates.insert(State(maxEventCountLimit));
  std::queue<std::set<State, decltype(stateCompare)*>::iterator> unevaluatedStates;
  unevaluatedStates.push(allStates.begin());

  int event;
  for (event = 0; !unevaluatedStates.empty() && event < maxEventCountLimit &&
                  static_cast<int>(allStates.size()) < totalStateCountLimit;
       ++event) {
    int countToEvaluate = static_cast<int>(unevaluatedStates.size());
    for (int i = 0; i < countToEvaluate; ++i) {
      const auto state = *(unevaluatedStates.front());
      unevaluatedStates.pop();
      for (const auto& rule : rules) {
        const auto newState = nextState(rule, state);
        if (newState.has_value()) {
          auto insertedState = allStates.insert(newState.value());
          if (insertedState.second) {
            unevaluatedStates.push(insertedState.first);
            if (static_cast<int>(allStates.size()) >= totalStateCountLimit) break;
          }
        }
      }
      if (static_cast<int>(allStates.size()) >= totalStateCountLimit) break;
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
