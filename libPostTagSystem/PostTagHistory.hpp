#ifndef LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_

#include <limits>
#include <memory>
#include <vector>

#include "TagState.hpp"

namespace PostTagSystem {
class PostTagHistory {
 public:
  enum class ConclusionReason {
    InvalidInput,
    Terminated,
    ReachedExplicitCheckpoint,
    ReachedAutomaticCheckpoint,
    ReachedPreviousInitCheckpoint,
    MaxEventCountExceeded,
    MaxTapeLengthExceeded,
    TimeConstraintExceeded,
    NotEvaluated
  };

  struct EvaluationResult {
    ConclusionReason conclusionReason;
    TagState finalState;
    uint64_t eventCount;
    uint64_t maxIntermediateTapeLength;
  };

  enum class NamedRule { Post = 0, Rule002211 = 1, Rule000010111 = 2 };

  static constexpr uint64_t eventCountsMultipleDisabled = 0;
  struct AutomaticCheckpointParameters {
    bool powerOfTwoEventCounts;
    uint64_t eventCountsMultiple;
  };

  struct CheckpointSpec {
    std::vector<TagState> states;
    AutomaticCheckpointParameters flags;
  };

  struct EvaluationLimits {
    uint64_t maxEventCount = std::numeric_limits<uint64_t>::max() - 7;
    uint64_t maxTapeLength = std::numeric_limits<uint64_t>::max();
    int64_t maxTimeNs = std::numeric_limits<int64_t>::max();

    EvaluationLimits() = default;
    explicit EvaluationLimits(uint64_t maxEventCountInput) : maxEventCount(maxEventCountInput) {}
  };

  PostTagHistory();
  EvaluationResult evaluate(const NamedRule& rule,
                            const TagState& init,
                            const EvaluationLimits& limits,
                            const CheckpointSpec& checkpointSpec = CheckpointSpec());

  std::vector<EvaluationResult> evaluate(const NamedRule& rule,
                                         const std::vector<TagState>& inits,
                                         const EvaluationLimits& limits,
                                         const CheckpointSpec& checkpointSpec = CheckpointSpec());

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
