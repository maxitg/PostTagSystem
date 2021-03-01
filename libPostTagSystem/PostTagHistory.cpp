#include "PostTagHistory.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <queue>
#include <unordered_map>

#include "CheckpointsTrie.hpp"
#include "ChunkedState.hpp"

namespace PostTagSystem {
class PostTagHistory::Implementation {
 private:
  struct ChunkOutput {
    uint16_t newTape;
    uint8_t newTapeSize;
    uint8_t newPhase;
  };

  struct ChunkedRule {
    uint8_t inputLength;
    uint8_t phaseCount;
    std::vector<uint8_t> outputTapes;
    std::vector<uint8_t> outputLengths;
    std::vector<uint8_t> outputPhases;

    uint32_t chunkCount() const { return 256 * phaseCount; }
  };

  const std::unordered_map<NamedRule, ChunkedRule> rules{
      {NamedRule::Post, {1, 3, {0, 0, 0, 3, 0, 1}, {1, 0, 1, 2, 1, 1}, {2, 0, 1, 1, 2, 0}}},
      {NamedRule::Rule002211, {2, 2, {0, 0, 0, 2, 9, 1, 0, 0}, {2, 0, 2, 2, 4, 2, 0, 0}, {1, 0, 0, 1, 1, 0, 0, 0}}},
      {NamedRule::Rule000010111, {2, 1, {0, 0, 5, 3}, {1, 3, 3, 3}, {0, 0, 0, 0}}}};

  struct ChunkEvaluationTable {
    std::vector<ChunkOutput> outputs;
    uint8_t phaseCount;
    uint8_t eventsAtOnce;
  };

  std::unordered_map<NamedRule, ChunkEvaluationTable> evaluationTables_;

 public:
  Implementation() {}

  EvaluationResult evaluate(const NamedRule& rule,
                            const TagState& init,
                            const EvaluationLimits& limits,
                            const CheckpointSpec& checkpointSpec) {
    const ChunkEvaluationTable chunkEvaluationTable = createChunkEvaluationTable(rule);
    if (limits.maxEventCount % chunkEvaluationTable.eventsAtOnce != 0) {
      return {ConclusionReason::InvalidInput, {{}, std::numeric_limits<uint8_t>::max()}, 0, 0};
    }
    auto chunkedState = toChunkedState(init);
    CheckpointsTrie explicitCheckpointsTrie;
    for (const auto& checkpoint : checkpointSpec.states) {
      explicitCheckpointsTrie.insert(toChunkedState(checkpoint));
    }
    uint64_t maxIntermediateTapeLength = tapeLength(chunkedState);
    ConclusionReason conclusionReason;
    const auto eventCount = evaluate(chunkEvaluationTable,
                                     &chunkedState,
                                     &conclusionReason,
                                     &maxIntermediateTapeLength,
                                     limits,
                                     explicitCheckpointsTrie,
                                     checkpointSpec.flags);
    return {conclusionReason, fromChunkedStateDestructively(&chunkedState), eventCount, maxIntermediateTapeLength};
  }

 private:
  ChunkEvaluationTable createChunkEvaluationTable(const NamedRule& rule) {
    auto foundTable = evaluationTables_.find(rule);
    if (foundTable == evaluationTables_.end()) {
      foundTable = evaluationTables_.insert({rule, createChunkEvaluationTable(rules.at(rule))}).first;
    }
    return foundTable->second;
  }

  ChunkEvaluationTable createChunkEvaluationTable(const ChunkedRule& rule) {
    ChunkEvaluationTable table;
    table.eventsAtOnce = 8 / rule.inputLength;
    table.phaseCount = rule.phaseCount;
    table.outputs.resize(rule.chunkCount());
    uint8_t inputTape = std::numeric_limits<uint8_t>::max();
    do {
      ++inputTape;
      for (uint8_t inputPhase = 0; inputPhase < rule.phaseCount; ++inputPhase) {
        table.outputs[rule.phaseCount * inputTape + inputPhase] = createChunkOutput(rule, inputTape, inputPhase);
      }
    } while (inputTape != std::numeric_limits<uint8_t>::max());
    return table;
  }

  ChunkOutput createChunkOutput(const ChunkedRule& rule, const uint8_t inputTape, const uint8_t inputPhase) const {
    uint16_t output = 0;
    uint8_t outputSize = 0;
    auto phase = inputPhase;
    auto shiftedInputTape = inputTape;
    for (uint8_t i = 0; i < 8; i += rule.inputLength) {
      uint8_t poppedBits = (shiftedInputTape >> (8 - rule.inputLength)) & (255 >> (8 - rule.inputLength));
      shiftedInputTape <<= rule.inputLength;
      uint8_t outputIndex = rule.phaseCount * poppedBits + phase;
      outputSize += rule.outputLengths[outputIndex];
      output = (output << (rule.outputLengths[outputIndex])) + rule.outputTapes[outputIndex];
      phase = rule.outputPhases[outputIndex];
    }
    return {output, outputSize, phase};
  }

  ChunkedState toChunkedState(const TagState& state) const {
    ChunkedState result;
    for (size_t i = 0; i < state.tape.size(); ++i) {
      if (i % 8 == 0) result.chunks.push_back(0);
      result.chunks.back() <<= 1;
      result.chunks.back() += state.tape[i];
    }
    result.lastChunkSize = state.tape.size() % 8;
    if (result.lastChunkSize == 0 && state.tape.size() != 0) result.lastChunkSize = 8;
    result.phase = state.headState;
    return result;
  }

  TagState fromChunkedStateDestructively(ChunkedState* chunkedState) const {
    std::vector<bool> tape;
    if (chunkedState->chunks.size()) {
      tape.reserve(8 * (chunkedState->chunks.size() - 1) + chunkedState->lastChunkSize);
      while (chunkedState->chunks.size() > 1) {
        pushBitsFromChunk(&tape, &chunkedState->chunks.front(), 8);
        chunkedState->chunks.pop_front();
      }
      pushBitsFromChunk(&tape, &chunkedState->chunks.front(), chunkedState->lastChunkSize);
      chunkedState->chunks.pop_front();
    }
    return {tape, chunkedState->phase};
  }

  static void pushBitsFromChunk(std::vector<bool>* tape, uint8_t* chunk, const uint8_t chunkSize) {
    *chunk <<= (8 - chunkSize);
    for (uint8_t bitIndex = 0; bitIndex < chunkSize; ++bitIndex) {
      tape->push_back(*chunk & (1 << 7));
      *chunk <<= 1;
    }
  }

  uint64_t evaluate(const ChunkEvaluationTable& evaluationTable,
                    ChunkedState* state,
                    ConclusionReason* conclusionReason,
                    uint64_t* maxIntermediateTapeLength,
                    const EvaluationLimits& limits,
                    const CheckpointsTrie& explicitCheckpoints,
                    const CheckpointSpecFlags& checkpointFlags) const {
    CheckpointsTrie automaticCheckpoints;
    uint64_t eventCount;
    for (eventCount = 0; eventCount < limits.maxEventCount && state->chunks.size() > 1;
         eventCount += evaluationTable.eventsAtOnce) {
      if (*maxIntermediateTapeLength > limits.maxTapeLength) {
        *conclusionReason = ConclusionReason::MaxTapeLengthExceeded;
        return eventCount;
      }
      if (explicitCheckpoints.contains(*state)) {
        *conclusionReason = ConclusionReason::ReachedExplicitCheckpoint;
        return eventCount;
      }
      if (automaticCheckpoints.contains(*state)) {
        *conclusionReason = ConclusionReason::ReachedAutomaticCheckpoint;
        return eventCount;
      }
      if (checkpointFlags.powerOfTwoEventCounts && !isPowerOfTwo(eventCount)) {
        automaticCheckpoints.insert(*state);
      }
      evaluateOnce(evaluationTable, state);
      *maxIntermediateTapeLength = std::max(*maxIntermediateTapeLength, tapeLength(*state));
    }
    if (eventCount == limits.maxEventCount) {
      *conclusionReason = ConclusionReason::MaxEventCountExceeded;
    } else {
      *conclusionReason = ConclusionReason::Terminated;
    }
    return eventCount;
  }

  static inline bool isPowerOfTwo(const uint64_t number) { return number & (number - 1); }

  uint64_t tapeLength(const ChunkedState& state) const {
    return std::max(0, static_cast<int>(state.chunks.size()) - 1) * 8 + state.lastChunkSize;
  }

  void evaluateOnce(const ChunkEvaluationTable& evaluationTable, ChunkedState* state) const {
    const auto nextChunkIndex = evaluationTable.phaseCount * state->chunks.front() + state->phase;
    const auto& chunkOutput = evaluationTable.outputs[nextChunkIndex];
    state->chunks.pop_front();
    state->phase = chunkOutput.newPhase;
    auto remainingBitCountToStore = chunkOutput.newTapeSize;
    while (remainingBitCountToStore > 0) {
      auto bitCountToStoreNow = std::min(remainingBitCountToStore, static_cast<uint8_t>(8 - state->lastChunkSize));

      auto bitCountNotToStoreNow = remainingBitCountToStore - bitCountToStoreNow;
      auto bitsToStoreNow = static_cast<uint8_t>(
          static_cast<uint16_t>(chunkOutput.newTape >> bitCountNotToStoreNow << (16 - bitCountToStoreNow)) >>
          (16 - bitCountToStoreNow));

      state->chunks.back() = (state->chunks.back() << bitCountToStoreNow) + bitsToStoreNow;
      state->lastChunkSize += bitCountToStoreNow;

      remainingBitCountToStore -= bitCountToStoreNow;
      if (remainingBitCountToStore) {
        state->chunks.push_back(0);
        state->lastChunkSize = 0;
      }
    }
  }
};

PostTagHistory::PostTagHistory() : implementation_(std::make_shared<Implementation>()) {}

PostTagHistory::EvaluationResult PostTagHistory::evaluate(const NamedRule& rule,
                                                          const TagState& init,
                                                          const EvaluationLimits& limits,
                                                          const CheckpointSpec& checkpoints) {
  return implementation_->evaluate(rule, init, limits, checkpoints);
}
}  // namespace PostTagSystem
