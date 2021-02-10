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
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, -1], {PostTagSystemFinalState::eventCountNegative}],
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, 9223372036854775808 (* 2^63 *)],
                        {PostTagSystemFinalState::eventCountTooLarge}],
        testUnevaluated[PostTagSystemFinalState[{0, nineZeros}, #],
                        {PostTagSystemFinalState::eventCountUneven}] & /@ {1, 2, 3, 4, 5, 6, 7, 9, 100},

        VerificationTest[PostTagSystemFinalState[{0, {0, 0, 0, 0, 0, 0, 0, 0, 0}}, 8], {1, {0, 0, 0, 0, 0, 0, 0}}],
        VerificationTest[PostTagSystemFinalState[{2, {1, 1, 1, 1, 1, 1, 1, 1, 1}}, 8],
                         {1, {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1}}],
        VerificationTest[PostTagSystemFinalState[{0, {}}, 8], {0, {}}],
        VerificationTest[PostTagSystemFinalState[{0, {0}}, 8], {0, {0}}],
        VerificationTest[PostTagSystemFinalState[{0, nineZeros}, 0], {0, nineZeros}],

        VerificationTest[PostTagSystemFinalState[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858040],
                         {2, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}],
        VerificationTest[PostTagSystemFinalState[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, #],
                         {0, {0, 0, 0, 0, 0, 0, 0}}] & /@ {20858048, 20858048 + 8, 2^32 - 8, 2^63 - 8},

        randomInit[seed_] := ModuleScope[BlockRandom[
          phase = RandomInteger[{0, 2}];
          length = RandomInteger[{9, 14}];
          tape = RandomInteger[1, length];
          {phase, tape}
        , RandomSeeding -> seed]];

        stateList[init_, stride_, minSize_] := ModuleScope[
          system = PostTagSystem[init];
          statesOfAnySize = system["State", #] & /@ Range[1, system["StateCount"], stride];
          firstSmallState =
            FirstPosition[statesOfAnySize, {_, _ ? (Length[#] <= minSize &)}, {Length[statesOfAnySize]}, {1}][[1]];
          statesOfAnySize[[1 ;; firstSmallState]]
        ];

        With[{randomInits = randomInit /@ Range[1000]},
          Function[{init}, MapIndexed[With[{
              eventCount = 8 * (#2[[1]] - 1)},
            VerificationTest[PostTagSystemFinalState[init, eventCount], #1]
          ] &, stateList[init, 8, 8]]] /@ randomInits
        ]
      }]
    }
  |>
|>
