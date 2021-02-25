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
                         <|"EventCount" -> 8, "MaxTapeLength" -> 9, "FinalState" -> {1, {0, 0, 0, 0, 0, 0, 0}}|>],
        VerificationTest[
          GenerateTagSystemHistory["Post", {2, {1, 1, 1, 1, 1, 1, 1, 1, 1}}, 8],
          <|"EventCount" -> 8, "MaxTapeLength" -> 12, "FinalState" -> {1, {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, {}}, 8],
                         <|"EventCount" -> 0, "MaxTapeLength" -> 0, "FinalState" -> {0, {}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, {0}}, 8],
                         <|"EventCount" -> 0, "MaxTapeLength" -> 1, "FinalState" -> {0, {0}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, nineZeros}, 0],
                         <|"EventCount" -> 0, "MaxTapeLength" -> 9, "FinalState" -> {0, nineZeros}|>],

        VerificationTest[GenerateTagSystemHistory["Post", {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858040],
                         <|"EventCount" -> 20858040,
                           "MaxTapeLength" -> 6783,
                           "FinalState" -> {2, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}|>],
        VerificationTest[GenerateTagSystemHistory["Post", {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, #],
                         <|"EventCount" -> 20858048,
                           "MaxTapeLength" -> 6783,
                           "FinalState" -> {0, {0, 0, 0, 0, 0, 0, 0}}|>] & /@
          {20858048, 20858048 + 8, 2^32 - 8, 2^63 - 8},

        VerificationTest[GenerateTagSystemHistory["Post",
                                                  {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                                  208570000,
                                                  {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}],
                         <|"EventCount" -> 20858000,
                           "MaxTapeLength" -> 6783,
                           "FinalState" -> {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}|>],
        VerificationTest[
          GenerateTagSystemHistory["Post",
                                   {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                   208570000,
                                   {{2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
                                    {1, {1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1,
                                         1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1}}}],
          <|"EventCount" -> 20857000,
            "MaxTapeLength" -> 6783,
            "FinalState" -> {1, {1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1,
                                 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1}}|>],

        VerificationTest[
          GenerateTagSystemHistory["002211",
                                   {0, {0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0}},
                                   600],
          <|"EventCount" -> 600,
            "MaxTapeLength" -> 40,
            "FinalState" -> {1, {1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1}}|>
        ],

        VerificationTest[
          GenerateTagSystemHistory["002211",
                                   {0, {0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0}},
                                   656],
          <|"EventCount" -> 656, "MaxTapeLength" -> 40, "FinalState" -> {1, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0}}|>
        ],

        VerificationTest[
          GenerateTagSystemHistory["000010111", {0, IntegerDigits[716, 2, 10]}, 10^9],
          <|"EventCount" -> 100280, "MaxTapeLength" -> 992, "FinalState" -> {0, {0, 0, 0, 0, 0, 0}}|>
        ],

        VerificationTest[
          GenerateTagSystemHistory["000010111", {0, IntegerDigits[345, 2, 9]}, 10^9],
          <|"EventCount" -> 26760, "MaxTapeLength" -> 557, "FinalState" -> {0, {0, 0, 0, 0, 0, 0, 0}}|>
        ],

        VerificationTest[
          GenerateTagSystemHistory["Post", {0, {1, 1, 1, 1, 1, 1, 1, 1, 1}}, 88]["MaxTapeLength"],
          20
        ],

        VerificationTest[
          GenerateTagSystemHistory["Post", {0, {1, 1, 1, 1, 1, 1, 1, 1, 1}}, 96]["MaxTapeLength"],
          20
        ],

        VerificationTest[
          GenerateTagSystemHistory["Post", {1, {1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1}}, 128, {"PowerOfTwoEventCounts"}][[
            {"EventCount", "FinalState"}]],
          <|"EventCount" -> 40, "FinalState" -> {1, {1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1}}|>
        ],

        VerificationTest[
          GenerateTagSystemHistory["Post", {1, {1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1}}, 128, {}]["EventCount"],
          128
        ],

        VerificationTest[
          GenerateTagSystemHistory["Post", {0, {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1}}, 128, {"PowerOfTwoEventCounts"}][
            "EventCount"],
          48
        ],

        VerificationTest[
          GenerateTagSystemHistory["Post", {2, {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0}}, 128, {"PowerOfTwoEventCounts"}][[
            {"EventCount", "FinalState"}]],
          <|"EventCount" -> 88,
            "FinalState" -> GenerateTagSystemHistory[
              "Post", {2, {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0}}, 64, {"PowerOfTwoEventCounts"}]["FinalState"]|>
        ],

        VerificationTest[
          Max @ BlockRandom[
            Table[
              GenerateTagSystemHistory[
                "Post", {RandomInteger[2], RandomInteger[1, 32]}, 10^12, {"PowerOfTwoEventCounts"}],
              1000]
          , RandomSeeding -> 0][[All, "EventCount"]] < 10^12
        ]
      }]
    }
  |>
|>
