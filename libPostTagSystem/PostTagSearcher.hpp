#ifndef LIBPOSTTAGSYSTEM_POSTTAGSEARCHER_HPP_
#define LIBPOSTTAGSYSTEM_POSTTAGSEARCHER_HPP_

#include <memory>
#include <vector>

#include "TagState.hpp"

namespace PostTagSystem {
class PostTagSearcher {
 public:
  PostTagSearcher();

  enum class TerminationReason {
    Terminated,
    ReachedCycle,
    ReachedKnownCheckpoint,
    MaxTapeLengthExceeded,
    MaxEventCountExceeded,
    TimeConstraintExceeded
  };

  struct EvaluationResult {
    TerminationReason terminationReason;
    uint64_t eventCount;
    uint64_t maxTapeLength;
    uint64_t finalTapeLength;
  };

  // Note that the system run 8 events at a time. That means, the tape lengths don't change one at a time, they can
  // change upto 8 at a time, so it's not enough to keep only checkpoints of lengths 1000 to catch everything
  using Checkpoints = std::vector<PostTagState>;

  struct EvaluationParameters {
    uint64_t maxTapeLength;
    uint64_t maxEventCount;
    // Unlike tape length and event count constraints, time constraint applies to the entire range/group.
    // If the time constraint is exceeded the current EvaluationResult and all the remaining ones will have
    // TimeConstraintExceeded termination reason. The aborted evaluations will have EvaluationResult filled in with
    // values obtained so far. The remaining ones will be filled with zeros.
    uint64_t groupTimeConstraintNs;
    Checkpoints checkpoints;
  };

  // The functions below use two tries. One for the input checkpoints which is shared among all of them. The other trie
  // is unique for each init and is used for cycle detection.
  // We might be able to share the second trie between different inits in the future, but that is not implemented yet
  // (trie is not thread-safe for writing)

  // Computes results for states between begin and the one preceeding end.
  // The head state is incremeneted first, and once it reaches 3, it is reset to zero with an incremented tape.
  std::vector<EvaluationResult> evaluateRange(const PostTagState& begin,
                                              const PostTagState& end,
                                              const EvaluationParameters& parameters);

  // Sets phases to zero and reads in tapeBegin and tapeEnd as binary digits.
  std::vector<EvaluationResult> evaluateRange(uint64_t tapeLength,
                                              uint64_t tapeBegin,
                                              uint64_t tapeEnd,
                                              const EvaluationParameters& parameters);

  // Evaluates for every state in states as an init.
  std::vector<EvaluationResult> evaluateGroup(const std::vector<PostTagState>& states,
                                              const EvaluationParameters& parameters);

  struct SmallState {
    uint64_t tape;
    uint8_t headState;
  };

  // Uses a more convenient representation for small states.
  std::vector<EvaluationResult> evaluateGroup(const std::vector<SmallState>& headsAndTapes,
                                              const EvaluationParameters& parameters);

 private:
  class Implementation;
  std::shared_ptr<Implementation> implementation_;
};
}  // namespace PostTagSystem

#endif  // LIBPOSTTAGSYSTEM_POSTTAGSEARCHER_HPP_
