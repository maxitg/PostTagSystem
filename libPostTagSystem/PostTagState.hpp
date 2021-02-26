#ifndef LIBPOSTTAGSYSTEM_POSTTAGSTATE_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGSTATE_HPP_

#include <vector>

namespace PostTagSystem {
struct PostTagState {
  std::vector<bool> tape;
  uint8_t headState;

  bool empty() const { return tape.empty(); }
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGSTATE_HPP_
