<|
  "GenerateTagSystemHistory" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];
    ),
    "tests" -> {
      testSymbolLeak[GenerateTagSystemHistory["Post", {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858048]],

      With[{nineZeros = ConstantArray[0, 9]}, {
        testUnevaluated[GenerateTagSystemHistory[], {GenerateTagSystemHistory::invalidArgumentCountRange}],
        testUnevaluated[GenerateTagSystemHistory["invalid", 1, 2], GenerateTagSystemHistory::invalidSystem],
        testUnevaluated[GenerateTagSystemHistory["Post", 1, 2, 3],
                        {GenerateTagSystemHistory::invalidStateFormat}],
        testUnevaluated[GenerateTagSystemHistory["Post", #, 8],
                        {GenerateTagSystemHistory::invalidStateFormat}] & /@ {1, {1, 2, 3}, {1}},
        testUnevaluated[GenerateTagSystemHistory["Post", {#, nineZeros}, 8],
                        {GenerateTagSystemHistory::invalidInitPhase}] & /@ {-1, 3, "x"},
        testUnevaluated[GenerateTagSystemHistory["Post", {0, #}, 8],
                        {GenerateTagSystemHistory::invalidInitTape}] & /@ {0, "x", {"x"}, {-1}, {2}, {0, 1, 2}},
        testUnevaluated[GenerateTagSystemHistory["Post", {0, nineZeros}, #],
                        {GenerateTagSystemHistory::eventCountNotInteger}] & /@ {"x", 2.3},
        testUnevaluated[GenerateTagSystemHistory["Post", {0, nineZeros}, -1],
                        {GenerateTagSystemHistory::eventCountNegative}],
        testUnevaluated[GenerateTagSystemHistory["Post", {0, nineZeros}, 9223372036854775808 (* 2^63 *)],
                        {GenerateTagSystemHistory::eventCountTooLarge}],
        testUnevaluated[GenerateTagSystemHistory["Post", {0, nineZeros}, #],
                        {GenerateTagSystemHistory::eventCountUneven}] & /@ {1, 2, 3, 4, 5, 6, 7, 9, 100},
        testUnevaluated[GenerateTagSystemHistory["Post", {0, nineZeros}, 8, #],
                        {GenerateTagSystemHistory::invalidCheckpoints}] & /@ {0, {0, 0}, {{0, nineZeros}, 0}},

        VerificationTest[GenerateTagSystemHistory["Post", {0, {0, 0, 0, 0, 0, 0, 0, 0, 0}}, 8],
                         <|"EventCount" -> 8, "FinalState" -> {1, {0, 0, 0, 0, 0, 0, 0}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {2, {1, 1, 1, 1, 1, 1, 1, 1, 1}}, 8],
                         <|"EventCount" -> 8, "FinalState" -> {1, {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, {}}, 8], <|"EventCount" -> 0, "FinalState" -> {0, {}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, {0}}, 8],
                         <|"EventCount" -> 0, "FinalState" -> {0, {0}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, nineZeros}, 0],
                         <|"EventCount" -> 0, "FinalState" -> {0, nineZeros}|>],

        VerificationTest[GenerateTagSystemHistory["Post", {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858040],
                         <|"EventCount" -> 20858040, "FinalState" -> {2, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, #],
                         <|"EventCount" -> 20858048, "FinalState" -> {0, {0, 0, 0, 0, 0, 0, 0}}|>] & /@
          {20858048, 20858048 + 8, 2^32 - 8, 2^63 - 8},

        VerificationTest[GenerateTagSystemHistory["Post",
                                                  {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                                  208570000,
                                                  {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}],
                         <|"EventCount" -> 20858000,
                           "FinalState" -> {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}|>],
        VerificationTest[
          GenerateTagSystemHistory["Post",
                                   {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                   208570000,
                                   {{2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
                                    {1, {1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1,
                                         1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1}}}],
          <|"EventCount" -> 20857000,
            "FinalState" -> {1, {1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1,
                                 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1}}|>],

        VerificationTest[
          GenerateTagSystemHistory["002211",
                                   {0, {0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0}},
                                   600],
          <|"EventCount" -> 600,
            "FinalState" -> {1, {1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1}}|>
        ],

        VerificationTest[
          GenerateTagSystemHistory["002211",
                                   {0, {0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0}},
                                   656],
          <|"EventCount" -> 656, "FinalState" -> {1, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0}}|>
        ]
      }]
    }
  |>
|>
