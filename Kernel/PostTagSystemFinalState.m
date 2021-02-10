Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystemFinalState"]

SetUsage @ "
PostTagSystemFinalState[{initPhase$, initTape$}, maxEventCount$] computes the evolution of the Post tag system \
starting from a state with head initPhase$ and tape initTape$ for at most maxEventCount$ events.
If the system reaches a state with <= 8 tape cells before maxEventCount$ is reached, that state is returned instead.
";

SyntaxInformation[PostTagSystemFinalState] = SyntaxInformation[GeneratePostTagSystemHistory];

expr : PostTagSystemFinalState[args___] := ModuleScope[
  result = Catch[
    postTagSystemFinalState[args], _ ? FailureQ, message[PostTagSystemFinalState, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

postTagSystemFinalState[args___] := generatePostTagSystemHistory[args]["FinalState"];
