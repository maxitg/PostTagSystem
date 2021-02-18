#include "WolframLanguageAPI.hpp"

#include <algorithm>
// NOLINTNEXTLINE(build/c++11)
#include <chrono>  // <chrono> is banned in Chromium, so cpplint flags it https://stackoverflow.com/a/33653404/905496
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "PostTagHistory.hpp"
#include "PostTagMultihistory.hpp"

namespace PostTagSystem {
namespace {
// These are global variables that keep all sets returned to Wolfram Language until they are no longer referenced.
// Pointers are not returned directly for security reasons.
using SystemID = mint;
// We use a pointer here because map key insertion (manageInstance) is separate from map value insertion
// (systemInitialize). Until the value is inserted, the set is nullptr.
std::unordered_map<SystemID, std::unique_ptr<PostTagMultihistory>> systems_;

/** @brief Either acquires or a releases a set, depending on the mode.
 */
void manageInstance([[maybe_unused]] WolframLibraryData libData, mbool mode, mint id) {
  if (mode == 0) {
    systems_.emplace(id, nullptr);
  } else {
    systems_.erase(id);
  }
}

int systemInitialize(mint argc, const MArgument* argv) {
  if (argc != 1) {
    return LIBRARY_FUNCTION_ERROR;
  }

  SystemID thisSystemID;
  try {
    thisSystemID = MArgument_getInteger(argv[0]);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    systems_[thisSystemID] = std::make_unique<PostTagMultihistory>();
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

PostTagMultihistory& systemFromID(const SystemID id) {
  const auto idIterator = systems_.find(id);
  if (idIterator != systems_.end()) {
    return *idIterator->second;
  } else {
    throw LIBRARY_FUNCTION_ERROR;
  }
}

int stateCount(mint argc, MArgument* argv, MArgument result) {
  if (argc != 1) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    MArgument_setInteger(result, systemFromID(MArgument_getInteger(argv[0])).stateCount());
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

PostTagState getState(WolframLibraryData libData, const mint headState, const MTensor tape) {
  const auto tapeData = libData->MTensor_getIntegerData(tape);
  const auto tapeLength = libData->MTensor_getFlattenedLength(tape);
  PostTagState result;
  result.headState = static_cast<uint8_t>(headState);

  result.tape.reserve(tapeLength);
  std::transform(tapeData, tapeData + tapeLength, std::back_inserter(result.tape), [](mint token) {
    return static_cast<uint8_t>(token);
  });
  return result;
}

std::vector<PostTagState> getStateVector(WolframLibraryData libData,
                                         const MTensor heads,
                                         const MTensor tapeLengths,
                                         const MTensor catenatedTapes) {
  const auto stateCount = libData->MTensor_getFlattenedLength(heads);
  const auto headsData = libData->MTensor_getIntegerData(heads);
  const auto lengthsData = libData->MTensor_getIntegerData(tapeLengths);
  const auto catenatedTapesData = libData->MTensor_getIntegerData(catenatedTapes);

  std::vector<PostTagState> result;
  result.reserve(stateCount);
  size_t catenatedTapesPosition = 0;
  for (mint stateIndex = 0; stateIndex < stateCount; ++stateIndex) {
    std::vector<bool> tape;
    tape.reserve(lengthsData[stateIndex]);
    const auto tapeEnd = catenatedTapesPosition + lengthsData[stateIndex];
    for (; catenatedTapesPosition < tapeEnd; ++catenatedTapesPosition) {
      tape.push_back(catenatedTapesData[catenatedTapesPosition]);
    }
    result.push_back({tape, static_cast<uint8_t>(headsData[stateIndex])});
  }

  return result;
}

int addEvolutionStartingFromState(WolframLibraryData libData, mint argc, MArgument* argv) {
  if (argc != 3) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    auto& system = systemFromID(MArgument_getInteger(argv[0]));
    const auto state = getState(libData, MArgument_getInteger(argv[1]), MArgument_getMTensor(argv[2]));
    system.addEvolutionStartingFromState(state);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

int stateSuccessors(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 1) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& successors = systemFromID(MArgument_getInteger(argv[0])).stateSuccessors();
    MTensor output;
    const mint dimensions[1] = {static_cast<mint>(successors.size())};
    libData->MTensor_new(MType_Integer, 1, dimensions, &output);
    mint position[1];
    for (size_t i = 0; i < successors.size(); ++i) {
      position[0] = i + 1;
      libData->MTensor_setInteger(output, position, successors[i] + 1);
    }
    MArgument_setMTensor(result, output);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

int stateSuccessor([[maybe_unused]] WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 2) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& successors = systemFromID(MArgument_getInteger(argv[0])).stateSuccessors();
    const auto successor = successors[MArgument_getInteger(argv[1]) - 1] + 1;
    MArgument_setInteger(result, successor);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

void putState(WolframLibraryData libData,
              const PostTagState& state,
              MTensor* output,
              std::vector<uint64_t> prefix = {}) {
  const mint dimensions[1] = {static_cast<mint>(state.tape.size() + prefix.size() + 1)};
  libData->MTensor_new(MType_Integer, 1, dimensions, output);
  mint position[1] = {0};
  for (size_t i = 0; i < prefix.size(); ++i) {
    ++position[0];
    libData->MTensor_setInteger(*output, position, prefix[i]);
  }
  ++position[0];
  libData->MTensor_setInteger(*output, position, state.headState);
  for (size_t i = 0; i < state.tape.size(); ++i) {
    ++position[0];
    libData->MTensor_setInteger(*output, position, state.tape[i]);
  }
}

int state(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 2) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& state =
        systemFromID(MArgument_getInteger(argv[0])).state(static_cast<int>(MArgument_getInteger(argv[1])) - 1);
    MTensor output;
    putState(libData, state, &output);
    MArgument_setMTensor(result, output);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

int cycleSources(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 1) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& sources = systemFromID(MArgument_getInteger(argv[0])).cycleSources();
    MTensor output;
    const mint dimensions[1] = {static_cast<mint>(sources.size())};
    libData->MTensor_new(MType_Integer, 1, dimensions, &output);
    mint position[1];
    for (size_t i = 0; i < sources.size(); ++i) {
      position[0] = i + 1;
      libData->MTensor_setInteger(output, position, sources[i] + 1);
    }
    MArgument_setMTensor(result, output);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

int initStates(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 1) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& inits = systemFromID(MArgument_getInteger(argv[0])).initStates();
    MTensor output;
    const mint dimensions[1] = {static_cast<mint>(inits.size())};
    libData->MTensor_new(MType_Integer, 1, dimensions, &output);
    mint position[1];
    for (size_t i = 0; i < inits.size(); ++i) {
      position[0] = i + 1;
      libData->MTensor_setInteger(output, position, inits[i] + 1);
    }
    MArgument_setMTensor(result, output);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

PostTagHistory historyEvaluator_;

int evaluatePostTagSystem(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 7) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const PostTagHistory::NamedRule systemCode = static_cast<PostTagHistory::NamedRule>(MArgument_getInteger(argv[0]));
    const auto inState = getState(libData, MArgument_getInteger(argv[1]), MArgument_getMTensor(argv[2]));
    const auto checkpoints = getStateVector(
        libData, MArgument_getMTensor(argv[4]), MArgument_getMTensor(argv[5]), MArgument_getMTensor(argv[6]));
    const auto outState = historyEvaluator_.evaluate(systemCode, inState, MArgument_getInteger(argv[3]), checkpoints);
    MTensor output;
    putState(libData, outState.finalState, &output, {outState.eventCount, outState.maxTapeLength});
    MArgument_setMTensor(result, output);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}
}  // namespace
}  // namespace PostTagSystem

EXTERN_C mint WolframLibrary_getVersion() { return WolframLibraryVersion; }

EXTERN_C int WolframLibrary_initialize(WolframLibraryData libData) {
  return (*libData->registerLibraryExpressionManager)("PostTagSystem", PostTagSystem::manageInstance);
}

EXTERN_C void WolframLibrary_uninitialize(WolframLibraryData libData) {
  (*libData->unregisterLibraryExpressionManager)("PostTagSystem");
}

EXTERN_C int systemInitialize([[maybe_unused]] WolframLibraryData libData,
                              mint argc,
                              MArgument* argv,
                              [[maybe_unused]] MArgument result) {
  return PostTagSystem::systemInitialize(argc, argv);
}

EXTERN_C int stateCount([[maybe_unused]] WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::stateCount(argc, argv, result);
}

EXTERN_C int addEvolutionStartingFromState(WolframLibraryData libData,
                                           mint argc,
                                           MArgument* argv,
                                           [[maybe_unused]] MArgument result) {
  return PostTagSystem::addEvolutionStartingFromState(libData, argc, argv);
}

EXTERN_C int stateSuccessors(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::stateSuccessors(libData, argc, argv, result);
}

EXTERN_C int stateSuccessor(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::stateSuccessor(libData, argc, argv, result);
}

EXTERN_C int state(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::state(libData, argc, argv, result);
}

EXTERN_C int cycleSources(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::cycleSources(libData, argc, argv, result);
}

EXTERN_C int initStates(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::initStates(libData, argc, argv, result);
}

EXTERN_C int evaluatePostTagSystem(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::evaluatePostTagSystem(libData, argc, argv, result);
}
