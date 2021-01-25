#include "WolframLanguageAPI.hpp"

// NOLINTNEXTLINE(build/c++11)
#include <chrono>  // <chrono> is banned in Chromium, so cpplint flags it https://stackoverflow.com/a/33653404/905496
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
  PostTagState result;
  result.headState = headState;
  result.tape = std::vector<uint8_t>(tapeData, tapeData + libData->MTensor_getFlattenedLength(tape));
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
