<|
  "GeneratePostTagSystemHistory" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];
    ),
    "tests" -> {
      testSymbolLeak[GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858048]],

      With[{nineZeros = ConstantArray[0, 9]}, {
        testUnevaluated[GeneratePostTagSystemHistory[], {GeneratePostTagSystemHistory::invalidArgumentCountRange}],
        testUnevaluated[GeneratePostTagSystemHistory[1, 2, 3],
                        {GeneratePostTagSystemHistory::invalidArgumentCountRange}],
        testUnevaluated[GeneratePostTagSystemHistory[#, 8],
                        {GeneratePostTagSystemHistory::invalidStateFormat}] & /@ {1, {1, 2, 3}, {1}},
        testUnevaluated[GeneratePostTagSystemHistory[{#, nineZeros}, 8],
                        {GeneratePostTagSystemHistory::invalidInitPhase}] & /@ {-1, 3, "x"},
        testUnevaluated[GeneratePostTagSystemHistory[{0, #}, 8],
                        {GeneratePostTagSystemHistory::invalidInitTape}] & /@ {0, "x", {"x"}, {-1}, {2}, {0, 1, 2}},
        testUnevaluated[GeneratePostTagSystemHistory[{0, nineZeros}, #],
                        {GeneratePostTagSystemHistory::eventCountNotInteger}] & /@ {"x", 2.3},
        testUnevaluated[GeneratePostTagSystemHistory[{0, nineZeros}, -1],
                        {GeneratePostTagSystemHistory::eventCountNegative}],
        testUnevaluated[GeneratePostTagSystemHistory[{0, nineZeros}, 9223372036854775808 (* 2^63 *)],
                        {GeneratePostTagSystemHistory::eventCountTooLarge}],
        testUnevaluated[GeneratePostTagSystemHistory[{0, nineZeros}, #],
                        {GeneratePostTagSystemHistory::eventCountUneven}] & /@ {1, 2, 3, 4, 5, 6, 7, 9, 100},
        testUnevaluated[GeneratePostTagSystemHistory[{0, nineZeros}, 8, #],
                        {GeneratePostTagSystemHistory::invalidCheckpoints}] & /@ {0, {0, 0}, {{0, nineZeros}, 0}},

        VerificationTest[GeneratePostTagSystemHistory[{0, {0, 0, 0, 0, 0, 0, 0, 0, 0}}, 8],
                         <|"EventCount" -> 8, "FinalState" -> {1, {0, 0, 0, 0, 0, 0, 0}}|>],
        VerificationTest[GeneratePostTagSystemHistory[{2, {1, 1, 1, 1, 1, 1, 1, 1, 1}}, 8],
                         <|"EventCount" -> 8, "FinalState" -> {1, {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1}}|>],
        VerificationTest[GeneratePostTagSystemHistory[{0, {}}, 8], <|"EventCount" -> 0, "FinalState" -> {0, {}}|>],
        VerificationTest[GeneratePostTagSystemHistory[{0, {0}}, 8], <|"EventCount" -> 0, "FinalState" -> {0, {0}}|>],
        VerificationTest[GeneratePostTagSystemHistory[{0, nineZeros}, 0],
                         <|"EventCount" -> 0, "FinalState" -> {0, nineZeros}|>],

        VerificationTest[GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858040],
                         <|"EventCount" -> 20858040, "FinalState" -> {2, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}|>],
        VerificationTest[GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, #],
                         <|"EventCount" -> 20858048, "FinalState" -> {0, {0, 0, 0, 0, 0, 0, 0}}|>] & /@
          {20858048, 20858048 + 8, 2^32 - 8, 2^63 - 8},

        VerificationTest[GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                                      208570000,
                                                      {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}],
                         <|"EventCount" -> 20858000,
                           "FinalState" -> {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}|>],
        VerificationTest[
          GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                       208570000,
                                       {{2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
                                        {1, {1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1,
                                             1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1}}}],
          <|"EventCount" -> 20857000,
            "FinalState" -> {1, {1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1,
                                 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1}}|>],

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
              eventCount = 8 * (#2[[1]] - 1)}, {
            VerificationTest[GeneratePostTagSystemHistory[init, eventCount],
                             <|"EventCount" -> eventCount, "FinalState" -> #1|>],
            VerificationTest[
              GeneratePostTagSystemHistory[init, eventCount, PostTagSystemFinalState[init, Round[eventCount / 2, 8]]],
              <|"EventCount" -> Round[eventCount / 2, 8],
                "FinalState" -> PostTagSystemFinalState[init, Round[eventCount / 2, 8]]|>]
          }] &, stateList[init, 8, 8]]] /@ randomInits
        ]
      }]
    }
  |>
|>
