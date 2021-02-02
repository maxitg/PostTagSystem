Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystemFinalState"]

SetUsage @ "
PostTagSystemFinalState[{initPhase$, initTape$}, maxEventCount$] computes the evolution of the Post tag system \
starting from a state with head initPhase$ and tape initTape$ for at most maxEventCount$ events.
If the system reaches a state with <= 8 tape cells before maxEventCount$ is reached, that state is returned instead.
";

SyntaxInformation[PostTagSystemFinalState] = {"ArgumentsPattern" -> {init_, maxEventCount_}};

PostTagSystemFinalState::invalidStateFormat =
  "Initial state `1` at position `2` in `3` must be a pair {phase, tape}.";
PostTagSystemFinalState::invalidInitPhase = "Initial phase `1` at position `2` in `3` must be 0, 1, or 2.";
PostTagSystemFinalState::invalidInitTape = "Initial tape `1` at position `2` in `3` must be a list of 0s and 1s.";
PostTagSystemFinalState::eventCountNotInteger = "Max event count `1` at position `2` in `3` must be an integer.";
PostTagSystemFinalState::eventCountNegative = "Max event count `1` at position `2` in `3` must not be negative.";
PostTagSystemFinalState::eventCountTooLarge = "Max event count `1` at position `2` in `3` must be smaller than 2^63.";
PostTagSystemFinalState::eventCountUneven = "Max event count `1` at position `2` in `3` must be a multiple of 8.";

expr : PostTagSystemFinalState[args___] := ModuleScope[
  result = Catch[postTagSystemFinalState[args]];
  If[FailureQ[result], Switch[result[[1]],
    "invalidStateFormat",
      Message[PostTagSystemFinalState::invalidStateFormat, result[[2, "Init"]], 1, HoldForm[expr]],
    "invalidInitPhase",
      Message[PostTagSystemFinalState::invalidInitPhase, result[[2, "InitPhase"]], {1, 1}, HoldForm[expr]],
    "invalidInitTape",
      Message[PostTagSystemFinalState::invalidInitTape, result[[2, "InitTape"]], {1, 2}, HoldForm[expr]],
    "eventCountNotInteger",
      Message[PostTagSystemFinalState::eventCountNotInteger, result[[2, "EventCount"]], 2, HoldForm[expr]],
    "eventCountNegative",
      Message[PostTagSystemFinalState::eventCountNegative, result[[2, "EventCount"]], 2, HoldForm[expr]],
    "eventCountTooLarge",
      Message[PostTagSystemFinalState::eventCountTooLarge, result[[2, "EventCount"]], 2, HoldForm[expr]],
    "eventCountUneven",
      Message[PostTagSystemFinalState::eventCountUneven, result[[2, "EventCount"]], 2, HoldForm[expr]]
  ]];
  result /; !FailureQ[result]
];

postTagSystemFinalState[{initHead : 0 | 1 | 2, initTape : {(0 | 1) ...}},
                        maxEventCount_Integer ? (0 <= # < 2^63 && Mod[#, 8] == 0 &)] :=
  Through[{First, Rest}[cpp$postTagSystemFinalState[initHead, initTape, maxEventCount]]]

postTagSystemFinalState[init : Except[{_, _}], _] := Throw[Failure["invalidStateFormat", <|"Init" -> init|>]];

postTagSystemFinalState[{initHead : Except[0 | 1 | 2], _}, _] :=
  Throw[Failure["invalidInitPhase", <|"InitPhase" -> initHead|>]];

postTagSystemFinalState[{_, initTape : Except[{(0 | 1) ...}]}, _] :=
  Throw[Failure["invalidInitTape", <|"InitTape" -> initTape|>]];

postTagSystemFinalState[{_, _}, maxEventCount : Except[_Integer]] :=
  Throw[Failure["eventCountNotInteger", <|"EventCount" -> maxEventCount|>]];

postTagSystemFinalState[{_, _}, maxEventCount_Integer ? (# < 0 &)] :=
  Throw[Failure["eventCountNegative", <|"EventCount" -> maxEventCount|>]];

postTagSystemFinalState[{_, _}, maxEventCount_Integer ? (# >= 2^63 &)] :=
  Throw[Failure["eventCountTooLarge", <|"EventCount" -> maxEventCount|>]];

postTagSystemFinalState[{_, _}, maxEventCount_Integer ? (0 <= # < 2^63 && Mod[#, 8] != 0 &)] :=
  Throw[Failure["eventCountUneven", <|"EventCount" -> maxEventCount|>]];

postTagSystemFinalState[args___] := (
  Developer`CheckArgumentCount[PostTagSystemFinalState[args], 2, 2];
  Throw[Failure["invalidArgumentCount"]];
);
