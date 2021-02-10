Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["GeneratePostTagSystemHistory"]

PackageScope["generatePostTagSystemHistory"]

SetUsage @ "
GeneratePostTagSystemHistory[{initPhase$, initTape$}, maxEventCount$] computes the evolution of the Post tag system \
starting from a state with head initPhase$ and tape initTape$ for at most maxEventCount$ events, and returns an \
association containing information about that evolution.
If the system reaches a state with <= 8 tape cells before maxEventCount$ is reached, that state is returned instead.
";

SyntaxInformation[GeneratePostTagSystemHistory] = {"ArgumentsPattern" -> {init_, maxEventCount_}};

declareMessage[General::invalidStateFormat, "Initial state `init` in `expr` must be a pair {phase, tape}."];
declareMessage[General::invalidInitPhase, "Initial phase `initPhase` in `expr` must be 0, 1, or 2."];
declareMessage[General::invalidInitTape, "Initial tape `initTape` in `expr` must be a list of 0s and 1s."];
declareMessage[General::eventCountNotInteger, "Max event count `eventCount` in `expr` must be an integer."];
declareMessage[General::eventCountNegative, "Max event count `eventCount` in `expr` must not be negative."];
declareMessage[General::eventCountTooLarge, "Max event count `eventCount` in `expr` must be smaller than 2^63."];
declareMessage[General::eventCountUneven, "Max event count `eventCount` in `expr` must be a multiple of 8."];
declareMessage[General::invalidArgumentCount, "Expected `expected` arguments in `expr` instead of `actual`."];

expr : GeneratePostTagSystemHistory[args___] := ModuleScope[
  result = Catch[generatePostTagSystemHistory[args],
                 _ ? FailureQ,
                 message[GeneratePostTagSystemHistory, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

generatePostTagSystemHistory[{initHead : 0 | 1 | 2, initTape : {(0 | 1) ...}},
                        maxEventCount_Integer ? (0 <= # < 2^63 && Mod[#, 8] == 0 &)] := ModuleScope[
  cppOutput = cpp$evaluatePostTagSystem[initHead, initTape, maxEventCount];
  <|"EventCount" -> First[cppOutput], "FinalState" -> Through[{#[[2]] &, #[[3 ;; ]] &}[cppOutput]]|>
];

generatePostTagSystemHistory[init : Except[{_, _}], _] := throw[Failure["invalidStateFormat", <|"init" -> init|>]];

generatePostTagSystemHistory[{initHead : Except[0 | 1 | 2], _}, _] :=
  throw[Failure["invalidInitPhase", <|"initPhase" -> initHead|>]];

generatePostTagSystemHistory[{_, initTape : Except[{(0 | 1) ...}]}, _] :=
  throw[Failure["invalidInitTape", <|"initTape" -> initTape|>]];

generatePostTagSystemHistory[{_, _}, maxEventCount : Except[_Integer]] :=
  throw[Failure["eventCountNotInteger", <|"eventCount" -> maxEventCount|>]];

generatePostTagSystemHistory[{_, _}, maxEventCount_Integer ? (# < 0 &)] :=
  throw[Failure["eventCountNegative", <|"eventCount" -> maxEventCount|>]];

generatePostTagSystemHistory[{_, _}, maxEventCount_Integer ? (# >= 2^63 &)] :=
  throw[Failure["eventCountTooLarge", <|"eventCount" -> maxEventCount|>]];

generatePostTagSystemHistory[{_, _}, maxEventCount_Integer ? (0 <= # < 2^63 && Mod[#, 8] != 0 &)] :=
  throw[Failure["eventCountUneven", <|"eventCount" -> maxEventCount|>]];

generatePostTagSystemHistory[args___] :=
  throw[Failure["invalidArgumentCount", <|"expected" -> 2, "actual" -> Length[{args}]|>]];
