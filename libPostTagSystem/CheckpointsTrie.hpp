#ifndef LIBPOSTTAGSYSTEM_CHECKPOINTSTRIE_HPP_
#define LIBPOSTTAGSYSTEM_CHECKPOINTSTRIE_HPP_

#include <memory>
#include <optional>

#include "ChunkedState.hpp"

namespace PostTagSystem {
class CheckpointsTrie {
 public:
  CheckpointsTrie();
  // returns false if the value already exists, in which case the old key will remain
  bool insert(const ChunkedState& key, int value);
  std::optional<int> findValue(const ChunkedState& state) const;

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_CHECKPOINTSTRIE_HPP_
