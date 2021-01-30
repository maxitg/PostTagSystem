#include "WolframLanguageAPI.hpp"

#include <algorithm>
// NOLINTNEXTLINE(build/c++11)
#include <chrono>  // <chrono> is banned in Chromium, so cpplint flags it https://stackoverflow.com/a/33653404/905496
#include <deque>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "NDTM.hpp"
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

int state(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 2) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& state =
        systemFromID(MArgument_getInteger(argv[0])).state(static_cast<int>(MArgument_getInteger(argv[1])) - 1);
    MTensor output;
    const mint dimensions[1] = {static_cast<mint>(state.tape.size() + 1)};
    libData->MTensor_new(MType_Integer, 1, dimensions, &output);
    mint position[1];
    position[0] = 1;
    libData->MTensor_setInteger(output, position, state.headState);
    for (size_t i = 0; i < state.tape.size(); ++i) {
      position[0] = i + 2;
      libData->MTensor_setInteger(output, position, state.tape[i]);
    }
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

EXTERN_C int state(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::state(libData, argc, argv, result);
}

EXTERN_C int cycleSources(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::cycleSources(libData, argc, argv, result);
}

EXTERN_C int initStates(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::initStates(libData, argc, argv, result);
}

namespace NDTM {
int ndtmEvaluate(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 3) {
    return LIBRARY_FUNCTION_ERROR;
  }

  auto rulesTensor = MArgument_getMTensor(argv[0]);
  auto ruleData = libData->MTensor_getIntegerData(rulesTensor);
  auto ruleCount = libData->MTensor_getFlattenedLength(rulesTensor) / 5;
  std::vector<Rule> rules(ruleCount);
  for (int i = 0; i < ruleCount; ++i) {
    rules[i].oldHeadState = static_cast<int>(*(ruleData + 5 * i));
    rules[i].oldTapeState = static_cast<int>(*(ruleData + 5 * i + 1));
    rules[i].newHeadState = static_cast<int>(*(ruleData + 5 * i + 2));
    rules[i].newTapeState = static_cast<int>(*(ruleData + 5 * i + 3));
    rules[i].displacement = static_cast<int>(*(ruleData + 5 * i + 4));
  }

  LifetimeData lifetimeResult = evaluateNDTM(
      rules, static_cast<int>(MArgument_getInteger(argv[1])), static_cast<int>(MArgument_getInteger(argv[2])));

  MTensor output;
  const mint dimensions[1] = {3};
  libData->MTensor_new(MType_Integer, 1, dimensions, &output);
  mint position[1];
  position[0] = 1;
  libData->MTensor_setInteger(output, position, static_cast<int>(lifetimeResult.terminationReason));
  position[0] = 2;
  libData->MTensor_setInteger(output, position, lifetimeResult.maxEventCount);
  position[0] = 3;
  libData->MTensor_setInteger(output, position, lifetimeResult.totalStateCount);
  MArgument_setMTensor(result, output);

  return LIBRARY_NO_ERROR;
}
}  // namespace NDTM

EXTERN_C int ndtmEvaluate(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return NDTM::ndtmEvaluate(libData, argc, argv, result);
}
