Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystem"]
PackageExport["NDTMEvaluate"]
PackageExport["NDTMEvaluateSimple"]

PackageScope["unloadLibrary"]
PackageScope["cpp$stateCount"]
PackageScope["cpp$stateSuccessors"]
PackageScope["cpp$state"]
PackageScope["cpp$cycleSources"]
PackageScope["cpp$initStates"]

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

  cpp$ndtmEvaluate = If[$libraryFile =!= $Failed,
    LibraryFunctionLoad[
      $libraryFile,
      "ndtmEvaluate",
      {{Integer, 2},
       Integer,
       Integer},
      {Integer, 1}],
    $Failed]
};

SetUsage @ "
PostTagSystem[{h$, t$}] computes the evolution of the Post tag system starting from a state with head h$ and tape t$.
PostTagSystem[i$] produces an evolution object for multiple inits i$.
PostTagSystem[i$, e$] adds init i$ to the evolution object e$.
";

SyntaxInformation[PostTagSystem] = {"ArgumentsPattern" -> {init_, evolution_.}};

PostTagSystem[{initHead_Integer, initTape_}] := ModuleScope[
  evolution = CreateManagedLibraryExpression["PostTagSystem", PostTagSystemEvolution];
  id = ManagedLibraryExpressionID[evolution, "PostTagSystem"];
  cpp$systemInitialize[id];
  cpp$addEvolutionStartingFromState[id, initHead, initTape];
  evolution
];

PostTagSystem[states_] := ModuleScope[
  evolution = CreateManagedLibraryExpression["PostTagSystem", PostTagSystemEvolution];
  id = ManagedLibraryExpressionID[evolution, "PostTagSystem"];
  cpp$systemInitialize[id];
  cpp$addEvolutionStartingFromState[id, #, #2] & @@@ states;
  evolution
];

PostTagSystem[{initHead_, initTape_}, evolution : PostTagSystemEvolution[id_Integer]] := ModuleScope[
  cpp$addEvolutionStartingFromState[id, initHead, initTape];
  evolution
]

SetUsage @ "
NDTMEvaluate[rules$, eventLimit$, stateLimit$] evaluates an NDTM and returns the max event count and a total state
count.
";

SyntaxInformation[NDTMEvaluate] = {"ArgumentsPattern" -> {rules_, maxEventCountLimit_, totalStateCountLimit_}};
NDTMEvaluate[rules_, maxEventCountLimit_, totalStateCountLimit_] := ModuleScope[
  output = cpp$ndtmEvaluate[Catenate /@ List @@@ rules, maxEventCountLimit, totalStateCountLimit];
  resultAssociation = <|"MaxEventCount" -> output[[2]], "StateCount" -> output[[3]]|>;
  Switch[output[[1]],
    0, resultAssociation,
    1, Failure["MaxEventsExceeded", resultAssociation],
    2, Failure["MaxStatesExceeded", resultAssociation]
  ]
];

SetUsage @ "
NDTMEvaluateSimple[rules$, eventLimit$, stateLimit$] evaluates an NDTM and returns the max event count and a total
state count in a simple way.
";

SyntaxInformation[NDTMEvaluateSimple] = {"ArgumentsPattern" -> {rules_, maxEventCountLimit_, totalStateCountLimit_}};
NDTMEvaluateSimple[rules_, maxEventCountLimit_, totalStateCountLimit_] := ModuleScope[
  output = cpp$ndtmEvaluate[Catenate /@ List @@@ rules, maxEventCountLimit, totalStateCountLimit];
  result = {output[[2]], output[[3]]};
  Switch[output[[1]],
    0, result,
    1, None,
    2, TooBig
  ]
];
