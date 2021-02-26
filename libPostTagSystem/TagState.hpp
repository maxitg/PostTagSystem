#ifndef LIBPOSTTAGSYSTEM_TAGSTATE_HPP_
#define LIBPOSTTAGSYSTEM_TAGSTATE_HPP_

#include <vector>

namespace PostTagSystem {
struct TagState {
  std::vector<bool> tape;
  uint8_t headState;

  bool empty() const { return tape.empty(); }
};

struct PostTagState : TagState {
  const uint8_t phaseCount;
  PostTagState& operator++();
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_TAGSTATE_HPP_
