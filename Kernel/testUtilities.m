Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageScope["testUnevaluated"]
PackageScope["testSymbolLeak"]
PackageScope["checkGraphics"]
PackageScope["graphicsQ"]

(* VerificationTest should not directly appear here, as it is replaced by test.wls into other heads during evaluation.
    Use testHead argument instead. *)

(* It is necessary to compare strings because, i.e.,
    MatchQ[<|x -> 1|>, HoldPattern[<|x -> 1|>]] returns False. *)
Attributes[testUnevaluated] = {HoldAll};
testUnevaluated[testHead_, input_, messages_, opts___] :=
  testHead[
    ToString[FullForm[input]],
    StringReplace[ToString[FullForm[Hold[input]]], StartOfString ~~ "Hold[" ~~ expr___ ~~ "]" ~~ EndOfString :> expr],
    messages,
    opts];

Attributes[testSymbolLeak] = {HoldAll};
testSymbolLeak[testHead_, expr_, opts___] :=
  testHead[
    Module[{Global`before, Global`after},
      expr; (* symbols might get created at the first run due to initialization *)
      Global`before = Length[Names["*`*"]];
      expr;
      Global`after = Length[Names["*`*"]];
      Global`after - Global`before
    ],
    0,
    opts
  ];

(* UsingFrontEnd is necessary while running from wolframscript *)
(* Flashes a new frontend window momentarily, but that is ok, because this function is mostly for use in the CI *)
frontEndErrors[expr_] := UsingFrontEnd @ Module[{notebook, result},
  notebook = CreateDocument[ExpressionCell[expr]];
  SelectionMove[notebook, All, Notebook];
  result = MathLink`CallFrontEnd[FrontEnd`GetErrorsInSelectionPacket[notebook]];
  NotebookClose[notebook];
  result
];

checkGraphics::frontEndErrors := "``";

checkGraphics[graphics_] := (
  Message[checkGraphics::frontEndErrors, #] & /@ Flatten[frontEndErrors[graphics]];
  graphics
);

graphicsQ[graphics_] := Head[graphics] === Graphics && frontEndErrors[graphics] === {};
