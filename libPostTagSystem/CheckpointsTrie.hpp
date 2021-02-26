#ifndef LIBPOSTTAGSYSTEM_CHECKPOINTSTRIE_HPP_
#define LIBPOSTTAGSYSTEM_CHECKPOINTSTRIE_HPP_

#include <memory>

#include "ChunkedState.hpp"

namespace PostTagSystem {
class CheckpointsTrie {
 public:
  CheckpointsTrie();
  void insert(const ChunkedState& state);
  bool contains(const ChunkedState& state) const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_CHECKPOINTSTRIE_HPP_
