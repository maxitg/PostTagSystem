Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageExport["ToPackedTagSystemState"]
PackageExport["FromPackedTagSystemState"]

SetUsage @ "
ToPackedTagSystemState[system$][unpackedTape$] converts the unpackedTape$ to a packed state for system$.
";

SyntaxInformation[ToPackedTagSystemState] = {"ArgumentsPattern" -> {system_}};

declareMessage[ToPackedTagSystemState::invalidUnpackedTape,
               "Tape `tape` in `expr` should be a list of integers between 0 and `maxCellValue`."];

expr : ToPackedTagSystemState[args1___][args2___] := ModuleScope[
  result = Catch[toPackedTagSystemState[args1][args2],
                 _ ? FailureQ,
                 message[ToPackedTagSystemState, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

$systems = {"Post", "002211", "000010111"};

With[{systems = $systems},
  FE`Evaluate[FEPrivate`AddSpecialArgCompletion["ToPackedTagSystemState" -> {systems}]]
];

declareMessage[ToPackedTagSystemState::invalidBlanks,
               "Blanks found in state `state` in `expr` in positions which will need to be read."];

checkForBlanks[tape_, _] /; FreeQ[tape, Verbatim[_]] := tape;
checkForBlanks[_, unpackedTape_] := throw[Failure["invalidBlanks", <|"state" -> unpackedTape|>]];

toPackedState[phaseCount_, tapeAtomLength_][tape_] := ModuleScope[
  phase = Mod[Length[tape], phaseCount];
  atomicDigitsTape = checkForBlanks[tape[[1 ;; -1 ;; phaseCount]], tape];
  tape = Catenate[IntegerDigits[#, 2, tapeAtomLength] & /@ atomicDigitsTape];
  {phase, tape}
];

toPackedTagSystemState["Post"][unpackedTape : {(0 | 1 | Verbatim[_]) ...}] := toPackedState[3, 1][unpackedTape];
toPackedTagSystemState["Post"][unpackedTape_] :=
  throw[Failure["invalidUnpackedTape", <|"tape" -> unpackedTape, "maxCellValue" -> 1|>]];

toPackedTagSystemState["002211"][unpackedTape : {(0 | 1 | 2 | Verbatim[_]) ...}] := toPackedState[2, 2][unpackedTape];
toPackedTagSystemState["002211"][unpackedTape_] :=
  throw[Failure["invalidUnpackedTape", <|"tape" -> unpackedTape, "maxCellValue" -> 2|>]];

toPackedTagSystemState["000010111"][unpackedTape : {(0 | 1 | Verbatim[_]) ...}] := toPackedState[1, 1][unpackedTape];
toPackedTagSystemState["000010111"][unpackedTape_] :=
  throw[Failure["invalidUnpackedTape", <|"tape" -> unpackedTape, "maxCellValue" -> 1|>]];

toPackedTagSystemState[args1___][args2___] /; (Length[{args1}] =!= 1 || Length[{args2}] =!= 1) :=
  throw[Failure["invalidArgumentCountRange",
                <|"expectedMin" -> 1, "expectedMax" -> 1, "actual" -> {Length[{args1}], Length[{args2}]}|>]];

toPackedTagSystemState[system : Except[Alternatives @@ $systems]] :=
  throw[Failure["invalidSystem", <|"system" -> system, "systemList" -> systems|>]];

SetUsage @ "
FromPackedTagSystemState[system$][packedState$] converts the packedState$ to an unpacked tape for system$.
";

SyntaxInformation[FromPackedTagSystemState] = {"ArgumentsPattern" -> {system_}};

expr : FromPackedTagSystemState[args1___][args2___] := ModuleScope[
  result = Catch[fromPackedTagSystemState[args1][args2],
                 _ ? FailureQ,
                 message[FromPackedTagSystemState, #, <|"expr" -> HoldForm[expr]|>] &];
  result /; !FailureQ[result]
];

With[{systems = $systems},
  FE`Evaluate[FEPrivate`AddSpecialArgCompletion["FromPackedTagSystemState" -> {systems}]]
];

fromPackedState[phaseCount_, tapeAtomLength_][{phase_, tape_}] := ModuleScope[
  atomTape = FromDigits[#, 2] & /@ Partition[tape, tapeAtomLength];
  phase0Tape =
    Catenate @ Catenate @ Transpose @ {List /@ atomTape, ConstantArray[_, {Length[atomTape], phaseCount - 1}]};
  elementsToDrop = Mod[-phase + phaseCount, phaseCount];
  Drop[phase0Tape, -elementsToDrop]
]

fromPackedTagSystemState["Post"][packedState : {0 | 1 | 2, {(0 | 1) ...}}] := fromPackedState[3, 1][packedState];
fromPackedTagSystemState["Post"][packedState_] := throw[Failure["invalidStateFormat", <|"init" -> packedState|>]];

fromPackedTagSystemState["002211"][packedState : {0 | 1, {(0 | 1) ...}}] /; Mod[Length[packedState], 2] == 0 :=
  fromPackedState[2, 2][packedState];
fromPackedTagSystemState["002211"][packedState_] := throw[Failure["invalidStateFormat", <|"init" -> packedState|>]];

fromPackedTagSystemState["000010111"][packedState_ : {0, {(0 | 1) ...}}] := fromPackedState[1, 1][packedState];
fromPackedTagSystemState["000010111"][packedState : Except[{0, {(0 | 1) ...}}]] :=
  throw[Failure["invalidStateFormat", <|"init" -> packedState|>]];

fromPackedTagSystemState[args1___][args2___] /; (Length[{args1}] =!= 1 || Length[{args2}] =!= 1) :=
  throw[Failure["invalidArgumentCountRange",
                <|"expectedMin" -> 1, "expectedMax" -> 1, "actual" -> {Length[{args1}], Length[{args2}]}|>]];

fromPackedTagSystemState[system : Except[Alternatives @@ $systems]] :=
  throw[Failure["invalidSystem", <|"system" -> system, "systemList" -> systems|>]];
