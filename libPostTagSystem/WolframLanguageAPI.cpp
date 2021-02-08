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

#include "PostTagMultihistory.hpp"
#include "TagSystemEvaluator.hpp"

namespace PostTagSystem {
namespace {
// These are global variables that keep all sets returned to Wolfram Language until they are no longer referenced.
// Pointers are not returned directly for security reasons.
using SystemID = mint;
// We use a pointer here because map key insertion (manageInstance) is separate from map value insertion
// (postTagMultihistoryInitialize). Until the value is inserted, the set is nullptr.
std::unordered_map<SystemID, std::unique_ptr<PostTagMultihistory>> postTagMultihistories_;

/** @brief Either acquires or a releases a set, depending on the mode.
 */
void managePostTagMultihistoryInstance([[maybe_unused]] WolframLibraryData libData, mbool mode, mint id) {
  if (mode == 0) {
    postTagMultihistories_.emplace(id, nullptr);
  } else {
    postTagMultihistories_.erase(id);
  }
}

int postTagMultihistoryInitialize(mint argc, const MArgument* argv) {
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
    postTagMultihistories_[thisSystemID] = std::make_unique<PostTagMultihistory>();
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

PostTagMultihistory& postTagMultihistoryFromID(const SystemID id) {
  const auto idIterator = postTagMultihistories_.find(id);
  if (idIterator != postTagMultihistories_.end()) {
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
    MArgument_setInteger(result, postTagMultihistoryFromID(MArgument_getInteger(argv[0])).stateCount());
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

template <typename IntegerType>
std::vector<IntegerType> getVector(WolframLibraryData libData, MTensor tensor) {
  const auto data = libData->MTensor_getIntegerData(tensor);
  const auto length = libData->MTensor_getFlattenedLength(tensor);
  std::vector<IntegerType> result;

  result.reserve(length);
  std::transform(
      data, data + length, std::back_inserter(result), [](mint value) { return static_cast<IntegerType>(value); });
  return result;
}

PostTagState getState(WolframLibraryData libData, const mint headState, const MTensor tape) {
  return {getVector<bool>(libData, tape), static_cast<uint8_t>(headState)};
}

int addEvolutionStartingFromState(WolframLibraryData libData, mint argc, MArgument* argv) {
  if (argc != 3) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    auto& system = postTagMultihistoryFromID(MArgument_getInteger(argv[0]));
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
    const auto& successors = postTagMultihistoryFromID(MArgument_getInteger(argv[0])).stateSuccessors();
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
    const auto& successors = postTagMultihistoryFromID(MArgument_getInteger(argv[0])).stateSuccessors();
    const auto successor = successors[MArgument_getInteger(argv[1]) - 1] + 1;
    MArgument_setInteger(result, successor);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

void putState(WolframLibraryData libData, const PostTagState& state, MTensor* output) {
  const mint dimensions[1] = {static_cast<mint>(state.tape.size() + 1)};
  libData->MTensor_new(MType_Integer, 1, dimensions, output);
  mint position[1];
  position[0] = 1;
  libData->MTensor_setInteger(*output, position, state.headState);
  for (size_t i = 0; i < state.tape.size(); ++i) {
    position[0] = i + 2;
    libData->MTensor_setInteger(*output, position, state.tape[i]);
  }
}

int state(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 2) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& state = postTagMultihistoryFromID(MArgument_getInteger(argv[0]))
                            .state(static_cast<int>(MArgument_getInteger(argv[1])) - 1);
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
    const auto& sources = postTagMultihistoryFromID(MArgument_getInteger(argv[0])).cycleSources();
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
    const auto& inits = postTagMultihistoryFromID(MArgument_getInteger(argv[0])).initStates();
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

std::unordered_map<SystemID, std::unique_ptr<TagSystemEvaluator>> tagSystemEvaluators_;

TagSystemEvaluator& tagSystemEvaluatorFromID(const SystemID id) {
  const auto idIterator = tagSystemEvaluators_.find(id);
  if (idIterator != tagSystemEvaluators_.end()) {
    return *idIterator->second;
  } else {
    throw LIBRARY_FUNCTION_ERROR;
  }
}

void manageTagSystemEvaluator([[maybe_unused]] WolframLibraryData libData, mbool mode, mint id) {
  if (mode == 0) {
    tagSystemEvaluators_.emplace(id, nullptr);
  } else {
    tagSystemEvaluators_.erase(id);
  }
}

TagSystemEvaluator::Rules getTagSystemEvaluatorRules(WolframLibraryData libData, MTensor tensor) {
  const auto data = libData->MTensor_getIntegerData(tensor);
  const auto length = libData->MTensor_getFlattenedLength(tensor);
  TagSystemEvaluator::Rules result;

  if (length == 1 && data[0] == 0) return TagSystemEvaluator::postRules();

  result.reserve(length);
  for (auto dataPointer = data; dataPointer < data + length; dataPointer += 3) {
    result.push_back(
        {static_cast<uint8_t>(*data), static_cast<uint8_t>(*(data + 1)), static_cast<uint8_t>(*(data + 2))});
  }
  return result;
}

int tagSystemEvaluatorInitialize(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 2) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const SystemID thisSystemID = MArgument_getInteger(argv[0]);
    const TagSystemEvaluator::Rules rules = getTagSystemEvaluatorRules(libData, MArgument_getMTensor(argv[1]));

    tagSystemEvaluators_[thisSystemID] = std::make_unique<TagSystemEvaluator>(rules);
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

int evaluateTagSystem(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  if (argc != 4) {
    return LIBRARY_FUNCTION_ERROR;
  }

  try {
    const auto& system = tagSystemEvaluatorFromID(MArgument_getInteger(argv[0]));
    const auto inState = getState(libData, MArgument_getInteger(argv[1]), MArgument_getMTensor(argv[2]));
    const auto outState = system.evaluate(inState, MArgument_getInteger(argv[3]));
    MTensor output;
    putState(libData, outState, &output);
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
  return (*libData->registerLibraryExpressionManager)("PostTagSystem",
                                                      PostTagSystem::managePostTagMultihistoryInstance) &&
         (*libData->registerLibraryExpressionManager)("TagSystem", PostTagSystem::manageTagSystemEvaluator);
}

EXTERN_C void WolframLibrary_uninitialize(WolframLibraryData libData) {
  (*libData->unregisterLibraryExpressionManager)("PostTagSystem");
  (*libData->unregisterLibraryExpressionManager)("TagSystem");
}

EXTERN_C int systemInitialize([[maybe_unused]] WolframLibraryData libData,
                              mint argc,
                              MArgument* argv,
                              [[maybe_unused]] MArgument result) {
  return PostTagSystem::postTagMultihistoryInitialize(argc, argv);
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

EXTERN_C int tagSystemEvaluatorInitialize(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::tagSystemEvaluatorInitialize(libData, argc, argv, result);
}

EXTERN_C int evaluateTagSystem(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::evaluateTagSystem(libData, argc, argv, result);
}
