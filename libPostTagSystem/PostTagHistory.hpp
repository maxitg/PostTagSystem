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

  struct CheckpointSpecFlags {
    bool powerOfTwoEventCounts;
  };

  struct CheckpointSpec {
    std::vector<TagState> states;
    CheckpointSpecFlags flags;
  };

  struct EvaluationLimits {
    uint64_t maxEventCount = std::numeric_limits<uint64_t>::max() - 7;
    uint64_t maxTapeLength = std::numeric_limits<uint64_t>::max();
    uint64_t maxTimeNs = std::numeric_limits<uint64_t>::max();

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
