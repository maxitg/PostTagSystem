Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["GenerateTagSystemHistory"]

PackageScope["generateTagSystemHistory"]

SetUsage @ "
GenerateTagSystemHistory[system$, {initPhase$, initTape$}, maxEventCount$] computes the evolution of the named tag \
system$ starting from a state with head initPhase$ and tape initTape$ for at most maxEventCount$ events, and returns \
an association containing information about that evolution.
GeneratePostTagSystemHistory[system$, init$, maxEventCount$, checkpointList$] terminates the evolution once any of the \
states from checkpointList$ is reached. The states in checkpointList$ are specified using the same format as init$.
If the system reaches a state with <= 8 bits of tape cells before maxEventCount$ is reached, that state is returned \
instead.
";

SyntaxInformation[GenerateTagSystemHistory] =
  {"ArgumentsPattern" -> {system_, init_, maxEventCount_, checkpointList_.}};

declareMessage[General::invalidSystem, "Tag system `system` in `expr` should be one of `systemList`."];
declareMessage[General::invalidStateFormat, "Initial state `init` in `expr` must be a pair {phase, tape}."];
declareMessage[General::invalidInitPhase, "Initial phase `initPhase` in `expr` must be 0, 1, or 2."];
declareMessage[General::invalidInitTape, "Initial tape `initTape` in `expr` must be a list of 0s and 1s."];
declareMessage[General::eventCountNotInteger, "Max event count `eventCount` in `expr` must be an integer."];
declareMessage[General::eventCountNegative, "Max event count `eventCount` in `expr` must not be negative."];
declareMessage[General::eventCountTooLarge, "Max event count `eventCount` in `expr` must be smaller than 2^63."];
declareMessage[General::eventCountUneven,
               "Max event count `eventCount` in `expr` must be a multiple of `stepsAtATime`."];
declareMessage[General::invalidCheckpoints,
               "The list of checkpoints `checkpoints` in `expr` must be a single state or a list of states."];
declareMessage[General::invalidArgumentCountRange,
               "Expected between `expectedMin` and `expectedMax` arguments in `expr` instead of `actual`."];

expr : GenerateTagSystemHistory[args___] := ModuleScope[
  result = Catch[generateTagSystemHistory[args],
                 _ ? FailureQ,
                 message[GenerateTagSystemHistory, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

$systems = <|"Post" -> 0, "002211" -> 1|>;

With[{systems = Keys[$systems]},
  FE`Evaluate[FEPrivate`AddSpecialArgCompletion["GenerateTagSystemHistory" -> {systems, 0, 0, 0}]]
];

$systemPattern = Alternatives @@ Keys[$systems];
$statePattern = {0 | 1 | 2, {(0 | 1) ...}};

$stepsAtATime = <|"Post" -> 8, "002211" -> 4|>;
maxEventCountPattern[system_] := (_Integer ? (0 <= # < 2^63 && Mod[#, $stepsAtATime[system]] == 0 &));

generateTagSystemHistory[system : $systemPattern,
                         {initHead : 0 | 1 | 2, initTape : {(0 | 1) ...}},
                         maxEventCount_,
                         checkpoints : {$statePattern ...} : {}] /;
    MatchQ[maxEventCount, maxEventCountPattern[system]] := ModuleScope[
  cppOutput = cpp$evaluatePostTagSystem[$systems[system],
                                        initHead,
                                        initTape,
                                        maxEventCount,
                                        First /@ checkpoints,
                                        Length /@ Last /@ checkpoints,
                                        Catenate[Last /@ checkpoints]];
  <|"EventCount" -> First[cppOutput], "FinalState" -> Through[{#[[2]] &, #[[3 ;; ]] &}[cppOutput]]|>
];

generateTagSystemHistory[
      system : $systemPattern, init : $statePattern, maxEventCount_, checkpoint : $statePattern] /;
    MatchQ[maxEventCount, maxEventCountPattern[system]] :=
  generateTagSystemHistory[system, init, maxEventCount, {checkpoint}];

generateTagSystemHistory[system : Except[$systemPattern], ___] :=
  throw[Failure["invalidSystem", <|"system" -> system, "systemList" -> Keys[$systems]|>]];

generateTagSystemHistory[$systemPattern, init : Except[{_, _}], ___] :=
  throw[Failure["invalidStateFormat", <|"init" -> init|>]];

generateTagSystemHistory[$systemPattern, {initHead : Except[0 | 1 | 2], _}, ___] :=
  throw[Failure["invalidInitPhase", <|"initPhase" -> initHead|>]];

generateTagSystemHistory[$systemPattern, {_, initTape : Except[{(0 | 1) ...}]}, ___] :=
  throw[Failure["invalidInitTape", <|"initTape" -> initTape|>]];

generateTagSystemHistory[$systemPattern, {_, _}, maxEventCount : Except[_Integer], ___] :=
  throw[Failure["eventCountNotInteger", <|"eventCount" -> maxEventCount|>]];

generateTagSystemHistory[$systemPattern, {_, _}, maxEventCount_Integer ? (# < 0 &), ___] :=
  throw[Failure["eventCountNegative", <|"eventCount" -> maxEventCount|>]];

generateTagSystemHistory[$systemPattern, {_, _}, maxEventCount_Integer ? (# >= 2^63 &), ___] :=
  throw[Failure["eventCountTooLarge", <|"eventCount" -> maxEventCount|>]];

generateTagSystemHistory[system : $systemPattern, {_, _}, maxEventCount_Integer ? (0 <= # < 2^63 &), ___] /;
    Mod[maxEventCount, $stepsAtATime[system]] != 0 :=
  throw[Failure["eventCountUneven", <|"eventCount" -> maxEventCount, "stepsAtATime" -> $stepsAtATime[system]|>]];

generateTagSystemHistory[$systemPattern, {_, _}, _, checkpoints : Except[$statePattern | {$statePattern ...}], ___] :=
  throw[Failure["invalidCheckpoints", <|"checkpoints" -> checkpoints|>]];

generateTagSystemHistory[args___] :=
  throw[Failure["invalidArgumentCountRange", <|"expectedMin" -> 3, "expectedMax" -> 4, "actual" -> Length[{args}]|>]];
