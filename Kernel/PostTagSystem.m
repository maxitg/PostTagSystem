Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystem"]

PackageScope["unloadLibrary"]
PackageScope["cpp$stateCount"]
PackageScope["cpp$stateSuccessors"]

(* this function is defined now, but only run the *next* time Kernel/init.m is called, before all symbols
are cleared. *)
unloadLibrary[] := If[StringQ[$libraryFile],
  Scan[LibraryFunctionUnload, $libraryFunctions];
  $libraryFunctions = Null;
  Quiet @ LibraryUnload[$libraryFile];
];

PostTagSystem::nolibposttagsystem = "libPostTagSystem (``) could not be found and is required.";

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
      {Integer,    (* system ID *)
       Integer,    (* head state *)
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

  cpp$stateSuccessors = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "stateSuccessors",
      {Integer},
      {Integer, 1}],
    $Failed]
};

PostTagSystem[initHead_, initTape_] := ModuleScope[
  evolution = CreateManagedLibraryExpression["PostTagSystem", PostTagSystemEvolution];
  id = ManagedLibraryExpressionID[evolution, "PostTagSystem"];
  cpp$systemInitialize[id];
  cpp$addEvolutionStartingFromState[id, initHead, initTape];
  evolution
];
