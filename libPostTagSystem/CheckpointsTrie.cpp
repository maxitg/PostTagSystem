#include "CheckpointsTrie.hpp"

#include <algorithm>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

namespace PostTagSystem {
class CheckpointsTrie::Implementation {
 private:
  using MetadataMap = std::unordered_map<uint64_t, std::unordered_map<uint16_t, int64_t>>;
  using TrieNodes = std::vector<std::unordered_map<uint8_t, int64_t>>;
  using Suffixes = std::vector<std::vector<uint8_t>>;

  // metadataMap_[chunksCount][256 * lastChunkSize + phase] -> index
  // non-negative index is from trieNodes_, negative index is from suffixes_
  MetadataMap metadataMap_;
  TrieNodes trieNodes_;
  Suffixes reverseSuffixes_;  // suffixes are written in reverse order to optimize trie nodes extension

 public:
  void insert(const ChunkedState& state) {
    const uint16_t lastChunkSizePhaseIndex = 256 * state.lastChunkSize + state.phase;

    const auto fixedChunkCountIt = metadataMap_.find(state.chunks.size());
    if (fixedChunkCountIt == metadataMap_.end()) {
      reverseSuffixes_.push_back(std::vector<uint8_t>(state.chunks.rbegin(), state.chunks.rend()));
      metadataMap_.insert(
          {state.chunks.size(), {{lastChunkSizePhaseIndex, fromReverseSuffixesIndex(reverseSuffixes_.size() - 1)}}});
      return;
    }

    const auto fixedMetadataIt = fixedChunkCountIt->second.find(lastChunkSizePhaseIndex);
    if (fixedMetadataIt == fixedChunkCountIt->second.end()) {
      reverseSuffixes_.push_back(std::vector<uint8_t>(state.chunks.rbegin(), state.chunks.rend()));
      fixedChunkCountIt->second.insert(
          {lastChunkSizePhaseIndex, fromReverseSuffixesIndex(reverseSuffixes_.size() - 1)});
      return;
    }

    insertChunks(&fixedMetadataIt->second, state.chunks.begin(), state.chunks.end());
  }

  bool contains(const ChunkedState& state) const {
    const auto fixedChunkCountIt = metadataMap_.find(state.chunks.size());
    if (fixedChunkCountIt == metadataMap_.end()) return false;
    const auto fixedMetadataIt = fixedChunkCountIt->second.find(256 * state.lastChunkSize + state.phase);
    if (fixedMetadataIt == fixedChunkCountIt->second.end()) return false;
    return containsChunks(fixedMetadataIt->second, state.chunks.begin(), state.chunks.end());
  }

 private:
  using ChunksIterator = std::deque<uint8_t>::const_iterator;
  void insertChunks(int64_t* index, ChunksIterator chunksBegin, ChunksIterator chunksEnd) {
    if (chunksBegin == chunksEnd) return;

    if (*index >= 0) {
      const auto nextChunkIt = trieNodes_[*index].find(*chunksBegin);
      if (nextChunkIt == trieNodes_[*index].end()) {
        reverseSuffixes_.push_back(std::vector<uint8_t>(std::reverse_iterator<ChunksIterator>(chunksEnd),
                                                        std::reverse_iterator<ChunksIterator>(chunksBegin) - 1));
        trieNodes_[*index].insert({*chunksBegin, fromReverseSuffixesIndex(reverseSuffixes_.size() - 1)});
      } else {
        insertChunks(&nextChunkIt->second, chunksBegin + 1, chunksEnd);
      }
    } else {
      *index = pushChunk(*index);
      insertChunks(index, chunksBegin, chunksEnd);
    }
  }

  int64_t pushChunk(int64_t negativeIndex) {
    const auto firstValue = reverseSuffixes_[toReverseSuffixesIndex(negativeIndex)].back();
    reverseSuffixes_[toReverseSuffixesIndex(negativeIndex)].pop_back();
    trieNodes_.push_back({{firstValue, negativeIndex}});
    return trieNodes_.size() - 1;
  }

  static inline int64_t toReverseSuffixesIndex(int64_t negativeIndex) { return -negativeIndex - 1; }

  static inline int64_t fromReverseSuffixesIndex(int64_t positiveIndex) { return -(positiveIndex + 1); }

  bool containsChunks(int64_t index, ChunksIterator chunksBegin, ChunksIterator chunksEnd) const {
    if (chunksBegin == chunksEnd) return true;

    if (index >= 0) {
      const auto nextChunkIt = trieNodes_[index].find(*chunksBegin);
      if (nextChunkIt == trieNodes_[index].end()) {
        return false;
      } else {
        return containsChunks(nextChunkIt->second, chunksBegin + 1, chunksEnd);
      }
    } else {
      const auto firstMismatch = std::mismatch(chunksBegin,
                                               chunksEnd,
                                               reverseSuffixes_[toReverseSuffixesIndex(index)].rbegin(),
                                               reverseSuffixes_[toReverseSuffixesIndex(index)].rend());
      return firstMismatch.first == chunksEnd;
    }
  }
};

CheckpointsTrie::CheckpointsTrie() { implementation_ = std::make_shared<Implementation>(); }

void CheckpointsTrie::insert(const ChunkedState& state) { implementation_->insert(state); }

bool CheckpointsTrie::contains(const ChunkedState& state) const { return implementation_->contains(state); }

}  // namespace PostTagSystem
