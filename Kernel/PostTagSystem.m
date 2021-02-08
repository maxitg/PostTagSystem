Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystem"]

PostTagSystem::nolibposttagsystem = "libPostTagSystem (``) could not be found and is required.";

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
