#ifndef LIBPOSTTAGSYSTEM_TAGSYSTEMEVALUATOR_HPP_
#define LIBPOSTTAGSYSTEM_TAGSYSTEMEVALUATOR_HPP_

#include <limits>
#include <memory>
#include <vector>

#include "PostTagState.h"

namespace PostTagSystem {
class TagSystemEvaluator {
 public:
  struct RuleOutput {
    uint8_t tapeContents;
    uint8_t tapeLength;
    uint8_t phase;
  };

  using Rules = std::vector<RuleOutput>;

  static Rules postRules() { return {{0, 1, 2}, {0, 0, 0}, {0, 1, 1}, {3, 2, 1}, {0, 1, 2}, {1, 1, 0}}; }

  explicit TagSystemEvaluator(const Rules& rules);
  PostTagState evaluate(const PostTagState& init, uint64_t maxEvents) const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_TAGSYSTEMEVALUATOR_HPP_
