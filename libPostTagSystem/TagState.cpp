#include "TagState.hpp"

namespace PostTagSystem {
TagState::TagState(const std::vector<bool>& tape, uint8_t headState) : tape(tape), headState(headState) {}

TagState::TagState(uint64_t tapeLength, uint64_t tapeContents, uint8_t headState) {
  this->headState = headState;
  tape.resize(tapeLength);
  for (auto tapeIt = tape.rbegin(); tapeIt != tape.rend(); ++tapeIt) {
    *tapeIt = tapeContents & 1;
    tapeContents >>= 1;
  }
}

bool TagState::empty() const { return tape.empty(); }

bool TagState::operator==(const TagState& other) const { return headState == other.headState && tape == other.tape; }

bool TagState::operator!=(const TagState& other) const { return !(*this == other); }

TagState& TagState::increment(uint8_t phaseCount) {
  ++headState;
  if (headState == phaseCount) {
    headState = 0;
    int tapeIndex;
    for (tapeIndex = static_cast<int>(tape.size() - 1); tapeIndex >= 0; --tapeIndex) {
      if (tape[tapeIndex] == 0) {
        tape[tapeIndex] = 1;
        break;
      } else {
        tape[tapeIndex] = 0;
      }
    }
    if (tapeIndex < 0) {
      tape = std::vector<bool>(tape.size() + 1, 0);
      tape[0] = 1;
    }
  }
  return *this;
}
}  // namespace PostTagSystem
