#ifndef LIBPOSTTAGSYSTEM_POSTTAGMULTIHISTORY_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGMULTIHISTORY_HPP_

#include <memory>
#include <vector>

namespace PostTagSystem {
struct PostTagState {
  std::vector<uint8_t> tape;
  int8_t headState;

  bool empty() const { return tape.empty(); }
};

class PostTagMultihistory {
 public:
  PostTagMultihistory();

  void addEvolutionStartingFromState(const PostTagState& state);

  size_t stateCount() const;

  const std::vector<int>& stateSuccessors() const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGMULTIHISTORY_HPP_
