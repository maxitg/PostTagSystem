#include "TagState.hpp"

namespace PostTagSystem {
PostTagState& PostTagState::operator++() {
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
