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
  std::vector<int> values_;   // each suffix corresponds to one value, even if empty

 public:
  bool insert(const ChunkedState& key, const int value) {
    const uint16_t lastChunkSizePhaseIndex = 256 * key.lastChunkSize + key.phase;

    const auto fixedChunkCountIt = metadataMap_.find(key.chunks.size());
    if (fixedChunkCountIt == metadataMap_.end()) {
      reverseSuffixes_.push_back(std::vector<uint8_t>(key.chunks.rbegin(), key.chunks.rend()));
      values_.push_back(value);
      metadataMap_.insert(
          {key.chunks.size(), {{lastChunkSizePhaseIndex, fromReverseSuffixesIndex(reverseSuffixes_.size() - 1)}}});
      return true;
    }

    const auto fixedMetadataIt = fixedChunkCountIt->second.find(lastChunkSizePhaseIndex);
    if (fixedMetadataIt == fixedChunkCountIt->second.end()) {
      reverseSuffixes_.push_back(std::vector<uint8_t>(key.chunks.rbegin(), key.chunks.rend()));
      values_.push_back(value);
      fixedChunkCountIt->second.insert(
          {lastChunkSizePhaseIndex, fromReverseSuffixesIndex(reverseSuffixes_.size() - 1)});
      return true;
    }

    return insertChunks(&fixedMetadataIt->second, key.chunks.begin(), key.chunks.end(), value);
  }

  std::optional<int> findValue(const ChunkedState& state) const {
    const auto fixedChunkCountIt = metadataMap_.find(state.chunks.size());
    if (fixedChunkCountIt == metadataMap_.end()) return std::nullopt;
    const auto fixedMetadataIt = fixedChunkCountIt->second.find(256 * state.lastChunkSize + state.phase);
    if (fixedMetadataIt == fixedChunkCountIt->second.end()) return std::nullopt;
    return findValueInChunks(fixedMetadataIt->second, state.chunks.begin(), state.chunks.end());
  }

 private:
  using ChunksIterator = std::deque<uint8_t>::const_iterator;
  bool insertChunks(int64_t* index, ChunksIterator chunksBegin, ChunksIterator chunksEnd, const int value) {
    if (chunksBegin == chunksEnd) return false;  // it's a total match, don't insert a value

    if (*index >= 0) {
      const auto nextChunkIt = trieNodes_[*index].find(*chunksBegin);
      if (nextChunkIt == trieNodes_[*index].end()) {
        reverseSuffixes_.push_back(std::vector<uint8_t>(std::reverse_iterator<ChunksIterator>(chunksEnd),
                                                        std::reverse_iterator<ChunksIterator>(chunksBegin) - 1));
        values_.push_back(value);
        trieNodes_[*index].insert({*chunksBegin, fromReverseSuffixesIndex(reverseSuffixes_.size() - 1)});
      } else {
        insertChunks(&nextChunkIt->second, chunksBegin + 1, chunksEnd, value);
      }
    } else {
      *index = pushChunk(*index);
      insertChunks(index, chunksBegin, chunksEnd, value);
    }
    return true;
  }

  int64_t pushChunk(int64_t negativeIndex) {
    const auto firstValue = reverseSuffixes_.at(toReverseSuffixesIndex(negativeIndex)).back();
    reverseSuffixes_.at(toReverseSuffixesIndex(negativeIndex)).pop_back();
    trieNodes_.push_back({{firstValue, negativeIndex}});
    return trieNodes_.size() - 1;
  }

  static inline int64_t toReverseSuffixesIndex(int64_t negativeIndex) { return -negativeIndex - 1; }

  static inline int64_t fromReverseSuffixesIndex(int64_t positiveIndex) { return -(positiveIndex + 1); }

  std::optional<int> findValueInChunks(int64_t index, ChunksIterator chunksBegin, ChunksIterator chunksEnd) const {
    if (chunksBegin == chunksEnd) {
      if (index >= 0) throw std::runtime_error("trie keeps branching at the leaf, something is wrong");
      return values_.at(toReverseSuffixesIndex(index));
    }

    if (index >= 0) {
      const auto nextChunkIt = trieNodes_[index].find(*chunksBegin);
      if (nextChunkIt == trieNodes_[index].end()) {
        return std::nullopt;
      } else {
        return findValueInChunks(nextChunkIt->second, chunksBegin + 1, chunksEnd);
      }
    } else {
      const auto firstMismatch = std::mismatch(chunksBegin,
                                               chunksEnd,
                                               reverseSuffixes_[toReverseSuffixesIndex(index)].rbegin(),
                                               reverseSuffixes_[toReverseSuffixesIndex(index)].rend());
      if (firstMismatch.first == chunksEnd) {
        return values_.at(fromReverseSuffixesIndex(index));
      } else {
        return std::nullopt;
      }
    }
  }
};

CheckpointsTrie::CheckpointsTrie() { implementation_ = std::make_shared<Implementation>(); }

bool CheckpointsTrie::insert(const ChunkedState& key, const int value) { return implementation_->insert(key, value); }

std::optional<int> CheckpointsTrie::findValue(const ChunkedState& state) const {
  return implementation_->findValue(state);
}

}  // namespace PostTagSystem
