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
  };

  PostTagHistory();
  EvaluationResult evaluate(const PostTagState& init,
                            uint64_t maxEvents,
                            const std::vector<PostTagState>& checkpoints = {}) const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
