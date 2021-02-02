#include "PostTagHistory.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <queue>

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
  };

  static constexpr uint16_t chunkCount = 768;
  using ChunkEvaluationTable = std::array<ChunkOutput, chunkCount>;

  static constexpr uint8_t outputTapes[] = {0, 0, 0, 3, 1, 0};
  static constexpr uint8_t outputLengths[] = {1, 1, 0, 2, 1, 1};
  static constexpr uint8_t outputPhases[] = {1, 2, 0, 2, 0, 1};

  const ChunkEvaluationTable chunkEvaluationTable_;

 public:
  Implementation() : chunkEvaluationTable_(createChunkEvaluationTable()) {}

  State evaluate(const State& init, const uint64_t maxEvents) const {
    if (maxEvents % 8 != 0) {
      return {{}, std::numeric_limits<uint8_t>::max()};
    }
    auto chunkedState = toChunkedState(init);
    evaluateToTermination(&chunkedState, maxEvents / 8);
    return fromChunkedStateDestructively(&chunkedState);
  }

 private:
  ChunkEvaluationTable createChunkEvaluationTable() {
    ChunkEvaluationTable table;
    for (uint8_t inputTape = 0; inputTape < std::numeric_limits<uint8_t>::max(); ++inputTape) {
      for (uint8_t inputPhase = 0; inputPhase < 3; ++inputPhase) {
        table[3 * inputTape + inputPhase] = createChunkOutput(inputTape, inputPhase);
      }
    }
    return table;
  }

  ChunkOutput createChunkOutput(const uint8_t inputTape, const uint8_t inputPhase) {
    uint16_t output = 0;
    uint8_t outputSize = 0;
    auto phase = inputPhase;
    auto shiftedInputTape = inputTape;
    for (uint8_t i = 0; i < 8; ++i) {
      bool poppedBit = (shiftedInputTape >> 7) & 1;
      shiftedInputTape <<= 1;
      uint8_t outputIndex = 3 * poppedBit + phase;
      outputSize += outputLengths[outputIndex];
      output = (output << outputLengths[outputIndex]) + outputTapes[outputIndex];
      phase = outputPhases[outputIndex];
    }
    return {output, outputSize, phase};
  }

  ChunkedState toChunkedState(const State& state) const {
    ChunkedState result;
    for (size_t i = 0; i < state.tape.size(); ++i) {
      if (i % 8 == 0) result.chunks.push(0);
      result.chunks.back() <<= 1;
      result.chunks.back() += state.tape[i];
    }
    result.lastChunkSize = state.tape.size() % 8;
    result.phase = state.phase;
    return result;
  }

  State fromChunkedStateDestructively(ChunkedState* chunkedState) const {
    std::vector<bool> tape;
    tape.reserve(8 * (chunkedState->chunks.size() - 1) + chunkedState->lastChunkSize);
    while (chunkedState->chunks.size() > 1) {
      pushBitsFromChunk(&tape, &chunkedState->chunks.front(), 8);
      chunkedState->chunks.pop();
    }
    pushBitsFromChunk(&tape, &chunkedState->chunks.front(), chunkedState->lastChunkSize);
    chunkedState->chunks.pop();
    return {tape, chunkedState->phase};
  }

  static void pushBitsFromChunk(std::vector<bool>* tape, uint8_t* chunk, const uint8_t chunkSize) {
    *chunk <<= (8 - chunkSize);
    for (uint8_t bitIndex = 0; bitIndex < chunkSize; ++bitIndex) {
      tape->push_back(*chunk & (1 << 7));
      *chunk <<= 1;
    }
  }

  void evaluateToTermination(ChunkedState* state, const uint64_t maxEvents) const {
    for (uint64_t i = 0; i < maxEvents; ++i) {
      if (state->chunks.size() == 1) return;
      evaluateOnce(state);
    }
  }

  void evaluateOnce(ChunkedState* state) const {
    const auto nextChunkIndex = 3 * state->chunks.front() + state->phase;
    const auto& chunkOutput = chunkEvaluationTable_[nextChunkIndex];
    state->chunks.pop();
    state->phase = chunkOutput.newPhase;
    auto remainingBitCountToStore = chunkOutput.newTapeSize;
    while (remainingBitCountToStore > 0) {
      auto bitCountToStoreNow = std::min(remainingBitCountToStore, static_cast<uint8_t>(8 - state->lastChunkSize));
      auto bitsToStoreNow = static_cast<uint16_t>(chunkOutput.newTape >> (remainingBitCountToStore - bitCountToStoreNow)
                                                                             << (16 - bitCountToStoreNow)) >>
                            (16 - bitCountToStoreNow);

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

PostTagHistory::State PostTagHistory::evaluate(const State& init, const uint64_t maxEvents) const {
  return implementation_->evaluate(init, maxEvents);
}
}  // namespace PostTagSystem
