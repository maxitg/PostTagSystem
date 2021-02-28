#ifndef LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_

#include <limits>
#include <memory>
#include <vector>

#include "TagState.hpp"

namespace PostTagSystem {
class PostTagHistory {
 public:
  enum class ConclusionReason { InvalidInput, Terminated, ReachedCheckpoint, MaxEventCountExceeded };

  struct EvaluationResult {
    ConclusionReason conclusionReason;
    TagState finalState;
    uint64_t eventCount;
    uint64_t maxTapeLength;
  };

  enum class NamedRule { Post = 0, Rule002211 = 1, Rule000010111 = 2 };

  struct CheckpointSpecFlags {
    bool powerOfTwoEventCounts;
  };

  struct CheckpointSpec {
    std::vector<TagState> states;
    CheckpointSpecFlags flags;
  };

  PostTagHistory();
  EvaluationResult evaluate(const NamedRule& rule,
                            const TagState& init,
                            uint64_t maxEvents,
                            const CheckpointSpec& checkpointSpec = CheckpointSpec());

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
