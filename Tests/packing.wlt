<|
  "packing" -> <|
    "init" -> (
      Attributes[Global`testUnevaluated] = Attributes[Global`testSymbolLeak] = {HoldAll};
      Global`testUnevaluated[args___] := PostTagSystem`PackageScope`testUnevaluated[VerificationTest, args];
      Global`testSymbolLeak[args___] := PostTagSystem`PackageScope`testSymbolLeak[VerificationTest, args];
    ),
    "tests" -> {
      VerificationTest[ToPackedTagSystemState["Post"][{}], {0, {}}],
      VerificationTest[ToPackedTagSystemState["Post"][{0}], {1, {0}}],
      VerificationTest[ToPackedTagSystemState["Post"][{0, 0}], {2, {0}}],
      VerificationTest[ToPackedTagSystemState["Post"][{0, 0, 0}], {0, {0}}],
      VerificationTest[ToPackedTagSystemState["Post"][{1, 0, 1, 1, 0, 1, 1, 0}], {2, {1, 1, 1}}],

      VerificationTest[ToPackedTagSystemState["002211"][{0, 2, 1, 2, 1, 0}], {0, {0, 0, 0, 1, 0, 1}}],
      VerificationTest[ToPackedTagSystemState["002211"][{0, 0, 1, 2, 1, 2, 2}], {1, {0, 0, 0, 1, 0, 1, 1, 0}}],
      VerificationTest[ToPackedTagSystemState["002211"][{0, _, 1, _, 1, _, 2}], {1, {0, 0, 0, 1, 0, 1, 1, 0}}],
      VerificationTest[ToPackedTagSystemState["002211"][{}], {0, {}}],
      VerificationTest[ToPackedTagSystemState["002211"][{0}], {1, {0, 0}}],
      VerificationTest[ToPackedTagSystemState["002211"][{0, 0}], {0, {0, 0}}],
      VerificationTest[ToPackedTagSystemState["002211"][{2, 0}], {0, {1, 0}}],
      VerificationTest[ToPackedTagSystemState["002211"][{2, 1, 2}], {1, {1, 0, 1, 0}}],

      testUnevaluated[ToPackedTagSystemState["invalid"][{}], ToPackedTagSystemState::invalidSystem],
      testUnevaluated[ToPackedTagSystemState["Post"][{0, 2, 1}], ToPackedTagSystemState::invalidUnpackedTape],
      testUnevaluated[ToPackedTagSystemState["002211", 2][{}], ToPackedTagSystemState::invalidArgumentCountRange],
      testUnevaluated[ToPackedTagSystemState["002211"][{}, 3], ToPackedTagSystemState::invalidArgumentCountRange],
      testUnevaluated[ToPackedTagSystemState["Post"][{0, _, _, _}], ToPackedTagSystemState::invalidBlanks],

      VerificationTest[ToPackedTagSystemState["Post"] @ FromPackedTagSystemState["Post"] @ #, #] & /@ {
        {0, {}}, {1, {0}}, {2, {0}}, {0, {0}}, {2, {1, 1, 1}}, {0, {0, 0, 0, 1, 0, 1}}, {1, {0, 0, 0, 1, 0, 1, 1, 0}}
      },

      VerificationTest[ToPackedTagSystemState["002211"] @ FromPackedTagSystemState["002211"] @ #, #] & /@ {
        {0, {0, 0, 0, 1, 0, 1}}, {1, {0, 0, 0, 1, 0, 1, 1, 0}}, {0, {}}, {1, {0, 0}}, {0, {0, 0}}, {0, {1, 0}},
        {1, {1, 0, 1, 0}}
      },

      testUnevaluated[FromPackedTagSystemState["invalid"][{}], FromPackedTagSystemState::invalidSystem],
      testUnevaluated[FromPackedTagSystemState["Post"][{10, {}}], FromPackedTagSystemState::invalidStateFormat],
      testUnevaluated[
        FromPackedTagSystemState["002211", 2][{0, {1}}], FromPackedTagSystemState::invalidArgumentCountRange],
      testUnevaluated[
        FromPackedTagSystemState["002211"][{0, {1}}, 4], FromPackedTagSystemState::invalidArgumentCountRange],

      VerificationTest[ToPackedTagSystemState["000010111"][{0, 0, 1, 0, 1}], {0, {0, 0, 1, 0, 1}}],
      VerificationTest[FromPackedTagSystemState["000010111"][{0, {0, 0, 1, 0, 1}}], {0, 0, 1, 0, 1}]
    }
  |>
|>
