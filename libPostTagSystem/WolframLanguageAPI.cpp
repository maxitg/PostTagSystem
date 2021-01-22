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

namespace PostTagSystem {
namespace {
// These are global variables that keep all sets returned to Wolfram Language until they are no longer referenced.
// Pointers are not returned directly for security reasons.
using SystemID = mint;
// We use a pointer here because map key insertion (manageInstance) is separate from map value insertion
// (systemInitialize). Until the value is inserted, the set is nullptr.
std::unordered_map<SystemID, std::unique_ptr<std::vector<int>>> systems_;

/** @brief Either acquires or a releases a set, depending on the mode.
 */
void manageInstance([[maybe_unused]] WolframLibraryData libData, mbool mode, mint id) {
  if (mode == 0) {
    systems_.emplace(id, nullptr);
  } else {
    systems_.erase(id);
  }
}

mint getData(const mint* data, const mint& length, const mint& index) {
  if (index >= length || index < 0) {
    throw LIBRARY_FUNCTION_ERROR;
  } else {
    return data[index];
  }
}

int systemInitialize([[maybe_unused]] WolframLibraryData libData,
                     mint argc,
                     const MArgument* argv,
                     [[maybe_unused]] MArgument result) {
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
    systems_[thisSystemID] = std::make_unique<std::vector<int>>();
  } catch (...) {
    return LIBRARY_FUNCTION_ERROR;
  }

  return LIBRARY_NO_ERROR;
}

std::function<bool()> shouldAbort(WolframLibraryData libData) {
  return [libData]() { return static_cast<bool>(libData->AbortQ()); };
}

std::vector<int>& systemFromID(const SystemID id) {
  const auto idIterator = systems_.find(id);
  if (idIterator != systems_.end()) {
    return *idIterator->second;
  } else {
    throw LIBRARY_FUNCTION_ERROR;
  }
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

EXTERN_C int systemInitialize(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result) {
  return PostTagSystem::systemInitialize(libData, argc, argv, result);
}
