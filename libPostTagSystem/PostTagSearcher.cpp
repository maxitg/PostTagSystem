#include "PostTagSearcher.hpp"

#include "PostTagHistory.hpp"

namespace PostTagSystem {
class PostTagSearcher::Implementation {
 public:
  Implementation() {}

  std::vector<EvaluationResult> evaluateRange(uint64_t tapeLength,
                                              uint64_t tapeBegin,
                                              uint64_t tapeEnd,
                                              const EvaluationParameters& parameters) {
    return evaluateRange(TagState(tapeLength, tapeBegin, 0), TagState(tapeLength, tapeEnd, 0), parameters);
  }

  std::vector<EvaluationResult> evaluateRange(const TagState& begin,
                                              const TagState& end,
                                              const EvaluationParameters& parameters) {
    std::vector<TagState> states;
    constexpr uint8_t postPhaseCount = 3;
    for (auto state = begin; state != end; state.increment(postPhaseCount)) {
      states.push_back(state);
    }
    return evaluateGroup(states, parameters);
  }

  std::vector<EvaluationResult> evaluateGroup(const std::vector<SmallState>& headsAndTapes,
                                              const EvaluationParameters& parameters) {
    std::vector<TagState> states;
    for (const auto& smallState : headsAndTapes) {
      states.push_back(fromSmallState(smallState));
    }
    return evaluateGroup(states, parameters);
  }

  std::vector<EvaluationResult> evaluateGroup(const std::vector<TagState>& states,
                                              const EvaluationParameters& parameters) {
    // TODO(maxitg): Implement maxTapeLength parameters
    // TODO(maxitg): Implement groupTimeConstraintNs parameter
    // TODO(maxitg): Implement checkpoints parameter

    std::vector<EvaluationResult> results;
    results.reserve(states.size());
    PostTagHistory evaluator;
    for (const auto& init : states) {
      const auto singleInitResult =
          evaluator.evaluate(PostTagHistory::NamedRule::Post, init, parameters.maxEventCount, {{}, {true}});
      EvaluationResult result;
      result.eventCount = singleInitResult.eventCount;
      result.maxTapeLength = singleInitResult.maxTapeLength;
      result.finalTapeLength = singleInitResult.finalState.tape.size();
      result.initialState = init;
      result.finalState = singleInitResult.finalState;
      switch (singleInitResult.conclusionReason) {
        case PostTagHistory::ConclusionReason::Terminated:
          result.conclusionReason = ConclusionReason::Terminated;
          break;

        case PostTagHistory::ConclusionReason::ReachedCheckpoint:
          result.conclusionReason = ConclusionReason::ReachedCycle;
          break;

        case PostTagHistory::ConclusionReason::MaxEventCountExceeded:
          result.conclusionReason = ConclusionReason::MaxEventCountExceeded;
          break;

        default:
          result.conclusionReason = ConclusionReason::InvalidInput;
      }
      results.push_back(result);
    }
    return results;
  }

 private:
  TagState fromSmallState(const SmallState& smallState) {
    return TagState(smallState.tapeLength, smallState.tape, smallState.headState);
  }
};

bool PostTagSearcher::EvaluationResult::operator==(const PostTagSearcher::EvaluationResult& other) const {
  return conclusionReason == other.conclusionReason && eventCount == other.eventCount &&
         maxTapeLength == other.maxTapeLength && finalTapeLength == other.finalTapeLength &&
         initialState == other.initialState && finalState == other.finalState;
}

PostTagSearcher::PostTagSearcher() : implementation_(std::make_shared<Implementation>()) {}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateRange(
    const TagState& begin, const TagState& end, const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateRange(begin, end, parameters);
}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateRange(
    uint8_t tapeLength, uint64_t tapeBegin, uint64_t tapeEnd, const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateRange(tapeLength, tapeBegin, tapeEnd, parameters);
}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateGroup(
    const std::vector<TagState>& states, const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateGroup(states, parameters);
}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateGroup(
    const std::vector<PostTagSearcher::SmallState>& headsAndTapes,
    const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateGroup(headsAndTapes, parameters);
}
}  // namespace PostTagSystem
