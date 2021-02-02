#ifndef LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_

#include <limits>
#include <memory>
#include <vector>

namespace PostTagSystem {
class PostTagHistory {
 public:
  struct State {
    const std::vector<bool> tape;
    const uint8_t phase;
  };

  PostTagHistory();
  State evaluate(const State& init, uint64_t maxEvents = std::numeric_limits<uint64_t>::max()) const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGHISTORY_HPP_
