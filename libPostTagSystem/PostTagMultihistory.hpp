#ifndef LIBPOSTTAGSYSTEM_POSTTAGMULTIHISTORY_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGMULTIHISTORY_HPP_

#include <memory>
#include <vector>

#include "TagState.hpp"

namespace PostTagSystem {
class PostTagMultihistory {
 public:
  PostTagMultihistory();

  void addEvolutionStartingFromState(const TagState& state);

  size_t stateCount() const;

  const std::vector<int>& stateSuccessors() const;

  const TagState& state(int index) const;

  const std::vector<int> cycleSources() const;

  const std::vector<int>& initStates() const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGMULTIHISTORY_HPP_
