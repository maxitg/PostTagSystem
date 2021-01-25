// Library definition for Wolfram LibraryLink

#ifndef LIBPOSTTAGSYSTEM_WOLFRAMLANGUAGEAPI_HPP_
#define LIBPOSTTAGSYSTEM_WOLFRAMLANGUAGEAPI_HPP_

#include "WolframHeaders/WolframLibrary.h"

EXTERN_C DLLEXPORT mint WolframLibrary_getVersion();

EXTERN_C DLLEXPORT int WolframLibrary_initialize(WolframLibraryData libData);

EXTERN_C DLLEXPORT void WolframLibrary_uninitialize(WolframLibraryData libData);

/** @brief Creates a new set object.
 */
EXTERN_C DLLEXPORT int systemInitialize(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result);

EXTERN_C DLLEXPORT int addEvolutionStartingFromState(WolframLibraryData libData,
                                                     mint argc,
                                                     MArgument* argv,
                                                     MArgument result);

/** @brief Obtains the total number of states.
 */
EXTERN_C DLLEXPORT int stateCount(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result);

EXTERN_C DLLEXPORT int stateSuccessors(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result);

EXTERN_C DLLEXPORT int state(WolframLibraryData libData, mint argc, MArgument* argv, MArgument result);

#endif  // LIBPOSTTAGSYSTEM_WOLFRAMLANGUAGEAPI_HPP_
