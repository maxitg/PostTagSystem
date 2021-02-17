#include "PostTagHistory.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <queue>
#include <unordered_map>

namespace PostTagSystem {
class PostTagHistory::Implementation {
 private:
  struct ChunkOutput {
    uint16_t newTape;
    uint8_t newTapeSize;
    uint8_t newPhase;
  };

  struct ChunkedState {
    std::queue<uint8_t> chunks;
    uint8_t lastChunkSize;
    uint8_t phase;

    bool operator==(const ChunkedState& other) const {
      return phase == other.phase && lastChunkSize == other.lastChunkSize && chunks == other.chunks;
    }
  };

  struct ChunkedRule {
    uint8_t tapeAtomLength;
    uint8_t phaseCount;
    std::vector<uint8_t> outputTapes;
    std::vector<uint8_t> outputLengths;
    std::vector<uint8_t> outputPhases;

    uint32_t chunkCount() const { return 256 * phaseCount; }
  };

  const std::unordered_map<NamedRule, ChunkedRule> rules{
      {NamedRule::Post, {1, 3, {0, 0, 0, 3, 0, 1}, {1, 0, 1, 2, 1, 1}, {2, 0, 1, 1, 2, 0}}},
      {NamedRule::Rule002211, {2, 2, {0, 0, 0, 2, 9, 1, 0, 0}, {1, 0, 1, 1, 2, 1, 0, 0}, {1, 0, 0, 1, 1, 0, 0, 0}}}};

  struct ChunkEvaluationTable {
    std::vector<ChunkOutput> outputs;
    uint8_t phaseCount;
    uint8_t eventsAtOnce;
  };

  std::unordered_map<NamedRule, ChunkEvaluationTable> evaluationTables_;

 public:
  Implementation() {}

  EvaluationResult evaluate(const NamedRule& rule,
                            const PostTagState& init,
                            const uint64_t maxEvents,
                            const std::vector<PostTagState>& checkpoints) {
    const ChunkEvaluationTable chunkEvaluationTable = createChunkEvaluationTable(rule);
    if (maxEvents % chunkEvaluationTable.eventsAtOnce != 0) {
      return {{}, std::numeric_limits<uint8_t>::max()};
    }
    auto chunkedState = toChunkedState(init);
    std::vector<ChunkedState> chunkedCheckpoints;
    chunkedCheckpoints.reserve(checkpoints.size());
    for (const auto& checkpoint : checkpoints) {
      chunkedCheckpoints.push_back(toChunkedState(checkpoint));
    }
    const auto eventCount = evaluate(chunkEvaluationTable, &chunkedState, maxEvents, chunkedCheckpoints);
    return {fromChunkedStateDestructively(&chunkedState), eventCount};
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
    table.eventsAtOnce = 8 / rule.tapeAtomLength;
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
    for (uint8_t i = 0; i < 8; i += rule.tapeAtomLength) {
      uint8_t poppedBits = (shiftedInputTape >> (8 - rule.tapeAtomLength)) & (255 >> (8 - rule.tapeAtomLength));
      shiftedInputTape <<= rule.tapeAtomLength;
      uint8_t outputIndex = rule.phaseCount * poppedBits + phase;
      outputSize += rule.outputLengths[outputIndex] * rule.tapeAtomLength;
      output = (output << (rule.outputLengths[outputIndex] * rule.tapeAtomLength)) + rule.outputTapes[outputIndex];
      phase = rule.outputPhases[outputIndex];
    }
    return {output, outputSize, phase};
  }

  ChunkedState toChunkedState(const PostTagState& state) const {
    ChunkedState result;
    for (size_t i = 0; i < state.tape.size(); ++i) {
      if (i % 8 == 0) result.chunks.push(0);
      result.chunks.back() <<= 1;
      result.chunks.back() += state.tape[i];
    }
    result.lastChunkSize = state.tape.size() % 8;
    if (result.lastChunkSize == 0 && state.tape.size() != 0) result.lastChunkSize = 8;
    result.phase = state.headState;
    return result;
  }

  PostTagState fromChunkedStateDestructively(ChunkedState* chunkedState) const {
    std::vector<bool> tape;
    if (chunkedState->chunks.size()) {
      tape.reserve(8 * (chunkedState->chunks.size() - 1) + chunkedState->lastChunkSize);
      while (chunkedState->chunks.size() > 1) {
        pushBitsFromChunk(&tape, &chunkedState->chunks.front(), 8);
        chunkedState->chunks.pop();
      }
      pushBitsFromChunk(&tape, &chunkedState->chunks.front(), chunkedState->lastChunkSize);
      chunkedState->chunks.pop();
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
                    const uint64_t maxEvents,
                    const std::vector<ChunkedState>& checkpoints) const {
    uint64_t eventCount;
    for (eventCount = 0; eventCount < maxEvents && state->chunks.size() > 1;
         eventCount += evaluationTable.eventsAtOnce) {
      for (const auto& checkpoint : checkpoints) {
        if (checkpoint == *state) return eventCount;
      }
      evaluateOnce(evaluationTable, state);
    }
    return eventCount;
  }

  void evaluateOnce(const ChunkEvaluationTable& evaluationTable, ChunkedState* state) const {
    const auto nextChunkIndex = evaluationTable.phaseCount * state->chunks.front() + state->phase;
    const auto& chunkOutput = evaluationTable.outputs[nextChunkIndex];
    state->chunks.pop();
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
        state->chunks.push(0);
        state->lastChunkSize = 0;
      }
    }
  }
};

PostTagHistory::PostTagHistory() : implementation_(std::make_shared<Implementation>()) {}

PostTagHistory::EvaluationResult PostTagHistory::evaluate(const NamedRule& rule,
                                                          const PostTagState& init,
                                                          const uint64_t maxEvents,
                                                          const std::vector<PostTagState>& checkpoints) {
  return implementation_->evaluate(rule, init, maxEvents, checkpoints);
}
}  // namespace PostTagSystem
