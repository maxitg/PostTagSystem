#ifndef LIBPOSTTAGSYSTEM_CHUNKEDSTATE_HPP_
#define LIBPOSTTAGSYSTEM_CHUNKEDSTATE_HPP_

#include <deque>

namespace PostTagSystem {
struct ChunkedState {
  std::deque<uint8_t> chunks;
  uint8_t lastChunkSize;
  uint8_t phase;

  bool operator==(const ChunkedState& other) const {
    return phase == other.phase && lastChunkSize == other.lastChunkSize && chunks == other.chunks;
  }
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_CHUNKEDSTATE_HPP_
