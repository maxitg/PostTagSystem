<|
  "PostTagSystem" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];

      $interestingRule = {{0, 1}, {0, 2}, {0, 3}} ->
        {{4, 5}, {5, 4}, {4, 6}, {6, 4}, {5, 6},
          {6, 5}, {4, 1}, {5, 2}, {6, 3}, {1, 6}, {3, 4}};
      $interestingInit = {{0, 0}, {0, 0}, {0, 0}};

      $namingTestModel = {$interestingRule, $interestingInit, 5, "FinalState"};

      $timeConstraintRule = {{1, 2}} -> {{1, 3}, {3, 2}};
      $timeConstraintInit = {{0, 0}};

      maxVertexDegree[set_] := Max[Counts[Catenate[Union /@ set]]];
    ),
    "tests" -> {}
  |>
|>
