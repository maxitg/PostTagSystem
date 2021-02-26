#include "PostTagSearcher.hpp"

#include "PostTagHistory.hpp"

namespace PostTagSystem {
class PostTagSearcher::Implementation {
 public:
  Implementation() {}

  std::vector<EvaluationResult> evaluateRange(const PostTagState& begin,
                                              const PostTagState& end,
                                              const EvaluationParameters& parameters) {
    PostTagHistory evaluator;
    // for (auto state = begin; state != end; ++state) {
    //   const auto singleHistoryResult =
    //       evaluator.evaluate(PostTagHistory::NamedRule::Post, state, parameters.maxEventCount);
    // }

    // TODO(maxitg): implement
    return {};
  }

  std::vector<EvaluationResult> evaluateRange(uint64_t tapeLength,
                                              uint64_t tapeBegin,
                                              uint64_t tapeEnd,
                                              const EvaluationParameters& parameters) {
    // TODO(maxitg): implement
    return {};
  }

  std::vector<EvaluationResult> evaluateGroup(const std::vector<PostTagState>& states,
                                              const EvaluationParameters& parameters) {
    // TODO(maxitg): implement
    return {};
  }

  std::vector<EvaluationResult> evaluateGroup(const std::vector<SmallState>& headsAndTapes,
                                              const EvaluationParameters& parameters) {
    // TODO(maxitg): implement
    return {};
  }
};

PostTagSearcher::PostTagSearcher() : implementation_(std::make_shared<Implementation>()) {}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateRange(
    const PostTagState& begin, const PostTagState& end, const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateRange(begin, end, parameters);
}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateRange(
    uint64_t tapeLength,
    uint64_t tapeBegin,
    uint64_t tapeEnd,
    const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateRange(tapeLength, tapeBegin, tapeEnd, parameters);
}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateGroup(
    const std::vector<PostTagState>& states, const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateGroup(states, parameters);
}

std::vector<PostTagSearcher::EvaluationResult> PostTagSearcher::evaluateGroup(
    const std::vector<PostTagSearcher::SmallState>& headsAndTapes,
    const PostTagSearcher::EvaluationParameters& parameters) {
  return implementation_->evaluateGroup(headsAndTapes, parameters);
}
}  // namespace PostTagSystem
