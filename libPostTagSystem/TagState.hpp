#ifndef LIBPOSTTAGSYSTEM_TAGSTATE_HPP_
#define LIBPOSTTAGSYSTEM_TAGSTATE_HPP_

#include <cstdint>
#include <vector>

namespace PostTagSystem {
struct TagState {
  std::vector<bool> tape;
  uint8_t headState;

  TagState() = default;
  TagState(const std::vector<bool>& tape, uint8_t headState);

  TagState(uint64_t tapeLength, uint64_t tapeContents, uint8_t headState);

  bool empty() const;

  bool operator==(const TagState& other) const;
  bool operator!=(const TagState& other) const;

  TagState& increment(uint8_t phaseCount);
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_TAGSTATE_HPP_
