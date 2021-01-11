Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystem"]

PackageScope["unloadLibrary"]

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
  $cpp$systemInitialize = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "systemInitialize",
      {Integer}, (* system ID *)
      "Void"],
    $Failed]
};

PostTagSystem[] := ModuleScope[
  evolution = CreateManagedLibraryExpression["PostTagSystem", PostTagSystemEvolution];
  id = ManagedLibraryExpressionID[evolution, "PostTagSystem"];
  $cpp$systemInitialize[id];
  evolution
];
