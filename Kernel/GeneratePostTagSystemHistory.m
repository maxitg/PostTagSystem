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

expr : GeneratePostTagSystemHistory[args___] := ModuleScope[
  result = Catch[generatePostTagSystemHistory[args],
                 _ ? FailureQ,
                 message[GeneratePostTagSystemHistory, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

generatePostTagSystemHistory[args___] := generateTagSystemHistory["Post", args];
