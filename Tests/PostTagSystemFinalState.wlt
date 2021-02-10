<|
  "PostTagSystemFinalState" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];
    ),
    "tests" -> {
      testSymbolLeak[PostTagSystemFinalState[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858048]],

      With[{nineZeros = ConstantArray[0, 9]}, {
        testUnevaluated[PostTagSystemFinalState[], {PostTagSystemFinalState::invalidArgumentCount}],
        testUnevaluated[PostTagSystemFinalState[1, 2, 3], {PostTagSystemFinalState::invalidArgumentCount}],
        testUnevaluated[PostTagSystemFinalState[#, 8],
                        {PostTagSystemFinalState::invalidStateFormat}] & /@ {1, {1, 2, 3}, {1}},
        testUnevaluated[PostTagSystemFinalState[{#, nineZeros}, 8],
                        {PostTagSystemFinalState::invalidInitPhase}] & /@ {-1, 3, "x"},
        testUnevaluated[PostTagSystemFinalState[{0, #}, 8],
                        {PostTagSystemFinalState::invalidInitTape}] & /@ {0, "x", {"x"}, {-1}, {2}, {0, 1, 2}},
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, #],
                        {PostTagSystemFinalState::eventCountNotInteger}] & /@ {"x", 2.3},
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, -1],
                        {PostTagSystemFinalState::eventCountNegative}],
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, 9223372036854775808 (* 2^63 *)],
                        {PostTagSystemFinalState::eventCountTooLarge}],
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, #],
                        {PostTagSystemFinalState::eventCountUneven}] & /@ {1, 2, 3, 4, 5, 6, 7, 9, 100}
      }],

      BlockRandom[Table[
        With[{
            initTape = RandomInteger[1, RandomInteger[100]],
            initPhase = RandomInteger[2],
            eventCount = 8 * RandomInteger[100]},
          VerificationTest[PostTagSystemFinalState[{initPhase, initTape}, eventCount],
                           GeneratePostTagSystemHistory[{initPhase, initTape}, eventCount]["FinalState"]]
        ]
      , 1000], RandomSeeding -> 0]
    }
  |>
|>
