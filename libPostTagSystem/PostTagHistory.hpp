#ifndef LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_

#include <limits>
#include <memory>
#include <vector>

#include "PostTagState.h"

namespace PostTagSystem {
class PostTagHistory {
 public:
  struct EvaluationResult {
    PostTagState finalState;
    uint64_t eventCount;
    uint64_t maxTapeLength;
  };

  enum class NamedRule { Post = 0, Rule002211 = 1, Rule000010111 = 2 };

  struct CheckpointSpec {
    std::vector<PostTagState> states;
    bool powerOfTwoEventCounts;
  };

  PostTagHistory();
  EvaluationResult evaluate(const NamedRule& rule,
                            const PostTagState& init,
                            uint64_t maxEvents,
                            const CheckpointSpec& checkpointSpec = CheckpointSpec());

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
