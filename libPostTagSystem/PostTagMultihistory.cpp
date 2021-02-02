#include "PostTagMultihistory.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace PostTagSystem {
namespace {
// Hashes the values of the matches, not the pointer itself.
class PostTagStateHasher {
 public:
  size_t operator()(const PostTagState& state) const {
    std::size_t result = 0;
    hash_combine(&result, state.headState);
    for (const bool tapeValue : state.tape) {
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

class PostTagStateEquality {
 public:
  size_t operator()(const PostTagState& a, const PostTagState& b) const {
    if (a.headState != b.headState || a.tape.size() != b.tape.size()) {
      return false;
    }

    const auto mismatchedIterators = std::mismatch(a.tape.begin(), a.tape.end(), b.tape.begin(), b.tape.end());
    return mismatchedIterators.first == a.tape.end() && mismatchedIterators.second == b.tape.end();
  }
};
}  // namespace

class PostTagMultihistory::Implementation {
 private:
  std::vector<PostTagState> states_;
  std::vector<int> nextStates_;
  std::unordered_map<PostTagState, int, PostTagStateHasher, PostTagStateEquality> statesIndex_;

  std::vector<int> initStates_;

 public:
  void addEvolutionStartingFromState(const PostTagState& state) {
    PostTagState currentState = state;
    while (!statesIndex_.count(currentState)) {
      states_.push_back(currentState);
      statesIndex_[currentState] = static_cast<int>(states_.size() - 1);
      if (currentState.empty()) {
        nextStates_.push_back(-1);  // The empty state is the only one that does not have a successor
      } else {
        currentState = nextState(currentState);
        nextStates_.push_back(statesIndex_.count(currentState) ? statesIndex_[currentState]
                                                               : static_cast<int>(states_.size()));
      }
    }
    initStates_.push_back(statesIndex_[state]);
  }

  size_t stateCount() const { return states_.size(); }

  const std::vector<int>& stateSuccessors() const { return nextStates_; }

  const PostTagState& state(const int index) const { return states_[index]; }

  const std::vector<int> cycleSources() const {
    std::vector<bool> isEverVisited(states_.size(), false);
    std::vector<int> cycleSources;
    for (int i = 0; i < static_cast<int>(states_.size()); ++i) {
      if (!isEverVisited[i]) {
        std::unordered_set<int> currentlyVisitedStates;
        tryToFindCycleSource(i, &isEverVisited, &currentlyVisitedStates, &cycleSources);
      }
    }
    return cycleSources;
  }

  const std::vector<int>& initStates() const { return initStates_; }

 private:
  PostTagState nextState(const PostTagState& state) {
    PostTagState newState;
    newState.tape.insert(newState.tape.begin(), state.tape.begin() + 1, state.tape.end());
    if (state.tape.front() == 0) {
      newState.headState = (state.headState + 1) % 3;
      if (state.headState != 2) {
        newState.tape.push_back(0);
      }
    } else {
      newState.headState = (state.headState + 2) % 3;
      switch (state.headState) {
        case 0:
          newState.tape.push_back(1);
          newState.tape.push_back(1);
          break;

        case 1:
          newState.tape.push_back(1);
          break;

        case 2:
          newState.tape.push_back(0);
          break;

        default:
          break;
      }
    }
    return newState;
  }

  void tryToFindCycleSource(const int stateIndex,
                            std::vector<bool>* isEverVisited,
                            std::unordered_set<int>* currentlyVisitedStates,
                            std::vector<int>* result) const {
    if (stateIndex < 0) return;
    if (currentlyVisitedStates->count(stateIndex)) result->push_back(stateIndex);
    if ((*isEverVisited)[stateIndex]) return;
    (*isEverVisited)[stateIndex] = true;
    currentlyVisitedStates->insert(stateIndex);
    tryToFindCycleSource(nextStates_[stateIndex], isEverVisited, currentlyVisitedStates, result);
  }
};

PostTagMultihistory::PostTagMultihistory() : implementation_(std::make_shared<Implementation>()) {}

void PostTagMultihistory::addEvolutionStartingFromState(const PostTagState& state) {
  implementation_->addEvolutionStartingFromState(state);
}

size_t PostTagMultihistory::stateCount() const { return implementation_->stateCount(); }

const std::vector<int>& PostTagMultihistory::stateSuccessors() const { return implementation_->stateSuccessors(); }

const PostTagState& PostTagMultihistory::state(int index) const { return implementation_->state(index); }

const std::vector<int> PostTagMultihistory::cycleSources() const { return implementation_->cycleSources(); }

const std::vector<int>& PostTagMultihistory::initStates() const { return implementation_->initStates(); }

}  // namespace PostTagSystem
