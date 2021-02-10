Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageScope["unloadLibrary"]

PackageScope["cpp$systemInitialize"]
PackageScope["cpp$addEvolutionStartingFromState"]
PackageScope["cpp$stateCount"]
PackageScope["cpp$stateSuccessor"]
PackageScope["cpp$stateSuccessors"]
PackageScope["cpp$state"]
PackageScope["cpp$cycleSources"]
PackageScope["cpp$initStates"]
PackageScope["cpp$evaluatePostTagSystem"]

(* this function is defined now, but only run the *next* time Kernel/init.m is called, before all symbols
are cleared. *)
unloadLibrary[] := If[StringQ[$libraryFile],
  Scan[LibraryFunctionUnload, $libraryFunctions];
  $libraryFunctions = Null;
  Quiet @ LibraryUnload[$libraryFile];
];

$libraryFile = $PostTagSystemLibraryPath;

If[!StringQ[$libraryFile] || !FileExistsQ[$libraryFile],
  Message[PostTagSystem::nolibposttagsystem, $libraryFile];
  $libraryFile = $Failed;
];

(* Load libPostTagSystem functions *)
$libraryFunctions = {
  cpp$systemInitialize = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "systemInitialize",
      {Integer}, (* system ID *)
      "Void"],
    $Failed],

  cpp$addEvolutionStartingFromState = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "addEvolutionStartingFromState",
      {Integer,       (* system ID *)
       Integer,       (* head state *)
       {Integer, 1}}, (* tape state *)
      "Void"],
    $Failed],

  cpp$stateCount = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "stateCount",
      {Integer},
      Integer],
    $Failed],

  cpp$stateSuccessor = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "stateSuccessor",
      {Integer, Integer},
      Integer],
    $Failed],

  cpp$stateSuccessors = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "stateSuccessors",
      {Integer},
      {Integer, 1}],
    $Failed],

  cpp$state = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "state",
      {Integer,  (* system ID *)
       Integer}, (* state ID *)
      {Integer, 1}], (* {headState, tape[[1]], tape[[2]], ...} *)
    $Failed],

  cpp$cycleSources = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "cycleSources",
      {Integer},
      {Integer, 1}],
    $Failed],

  cpp$initStates = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
     $libraryFile,
     "initStates",
     {Integer},
     {Integer, 1}],
    $Failed],

  cpp$evaluatePostTagSystem = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "evaluatePostTagSystem",
      {Integer,      (* head state *)
       {Integer, 1}, (* tape state *)
       Integer},     (* event count *)
      {Integer, 1}], (* {eventCount, headState, tape[[1]], tape[[2]], ...} *)
    $Failed]
};
