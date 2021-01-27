<|
  "PostTagSystemEvolution" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];
    ),
    "tests" -> {
      VerificationTest[
        PostTagSystem[{0, {0, 0, 0}}]["StateGraph"],
        PathGraph[Range[10], DirectedEdges -> True],
        SameTest -> IsomorphicGraphQ
      ]
    }
  |>
|>
