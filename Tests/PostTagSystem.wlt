<|
  "PostTagSystem" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];
    ),
    "tests" -> {
      VerificationTest[
        PostTagSystem[{0, {0, 0, 0}}]["StateCount"],
        10
      ],

      VerificationTest[
        NDTMEvaluate[{{1, 0} -> {1, 0, 1}}, 4, 100],
        Failure["MaxEventsExceeded", <|"MaxEventCount" -> 4, "StateCount" -> 5|>]
      ],

      VerificationTest[
        NDTMEvaluateSimple[{{1, 0} -> {1, 0, 1}}, 4, 100],
        None
      ]
    }
  |>
|>
