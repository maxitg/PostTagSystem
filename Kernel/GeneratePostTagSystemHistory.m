Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["GeneratePostTagSystemHistory"]

PackageScope["generatePostTagSystemHistory"]

SetUsage @ "
GeneratePostTagSystemHistory[{initPhase$, initTape$}, maxEventCount$] computes the evolution of the Post tag system \
starting from a state with head initPhase$ and tape initTape$ for at most maxEventCount$ events, and returns an \
association containing information about that evolution.
GeneratePostTagSystemHistory[init$, maxEventCount$, checkpointList$] terminates the evolution once any of the states \
from checkpointList$ is reached. The states in checkpointList$ are specified using the same format as init$.
If the system reaches a state with <= 8 tape cells before maxEventCount$ is reached, that state is returned instead.
";

SyntaxInformation[GeneratePostTagSystemHistory] = {"ArgumentsPattern" -> {init_, maxEventCount_, checkpointList_.}};

declareMessage[General::invalidStateFormat, "Initial state `init` in `expr` must be a pair {phase, tape}."];
declareMessage[General::invalidInitPhase, "Initial phase `initPhase` in `expr` must be 0, 1, or 2."];
declareMessage[General::invalidInitTape, "Initial tape `initTape` in `expr` must be a list of 0s and 1s."];
declareMessage[General::eventCountNotInteger, "Max event count `eventCount` in `expr` must be an integer."];
declareMessage[General::eventCountNegative, "Max event count `eventCount` in `expr` must not be negative."];
declareMessage[General::eventCountTooLarge, "Max event count `eventCount` in `expr` must be smaller than 2^63."];
declareMessage[General::eventCountUneven, "Max event count `eventCount` in `expr` must be a multiple of 8."];
declareMessage[General::invalidCheckpoints,
               "The list of checkpoints `checkpoints` in `expr` must be a single state or a list of states."];
declareMessage[General::invalidArgumentCountRange,
               "Expected between `expectedMin` and `expectedMax` arguments in `expr` instead of `actual`."];

expr : GeneratePostTagSystemHistory[args___] := ModuleScope[
  result = Catch[generatePostTagSystemHistory[args],
                 _ ? FailureQ,
                 message[GeneratePostTagSystemHistory, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

$statePattern = {0 | 1 | 2, {(0 | 1) ...}};
$maxEventCountPattern = (_Integer ? (0 <= # < 2^63 && Mod[#, 8] == 0 &));

generatePostTagSystemHistory[{initHead : 0 | 1 | 2, initTape : {(0 | 1) ...}},
                             maxEventCount : $maxEventCountPattern,
                             checkpoints : {$statePattern ...} : {}] := ModuleScope[
  cppOutput = cpp$evaluatePostTagSystem[initHead,
                                        initTape,
                                        maxEventCount,
                                        First /@ checkpoints,
                                        Length /@ Last /@ checkpoints,
                                        Catenate[Last /@ checkpoints]];
  <|"EventCount" -> First[cppOutput], "FinalState" -> Through[{#[[2]] &, #[[3 ;; ]] &}[cppOutput]]|>
];

generatePostTagSystemHistory[init : $statePattern, maxEventCount : $maxEventCountPattern, checkpoint : $statePattern] :=
  generatePostTagSystemHistory[init, maxEventCount, {checkpoint}];

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

generatePostTagSystemHistory[{_, _}, _, checkpoints : Except[$statePattern | {$statePattern ...}]] :=
  throw[Failure["invalidCheckpoints", <|"checkpoints" -> checkpoints|>]];

generatePostTagSystemHistory[args___] :=
  throw[Failure["invalidArgumentCountRange", <|"expectedMin" -> 2, "expectedMax" -> 3, "actual" -> Length[{args}]|>]];
