#!/usr/bin/env wolframscript

Check[
  Needs["GeneralUtilities`"];
  Needs["GitLink`"];

  $postTagSystemRoot = FileNameDrop[$InputFileName, -1];
  Get[FileNameJoin[{$postTagSystemRoot, "DevUtils", "init.m"}]];
  Get[FileNameJoin[{$postTagSystemRoot, "Kernel", "init.m"}]];
,
  Print["Could not get one of the dependencies. Exiting."];
  Exit[1];
];

Check[
  tests = Hold @ <|
    "LongEvolution" -> GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}}, 20858048],
    "SingleCheckpoint" -> GeneratePostTagSystemHistory[{0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                                                       208570000,
                                                       {2, {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}],
    "PowerOfTwoCheckpoints" -> BlockRandom[
      Table[
        GenerateTagSystemHistory[
          "Post", {RandomInteger[2], RandomInteger[1, 32]}, 10^12, {"PowerOfTwoEventCounts"}],
        1000]
    , RandomSeeding -> 0],
    "ThousandCheckpoints" -> BlockRandom[
      GenerateTagSystemHistory["Post",
                               {0, {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0}},
                               20858048,
                               Table[{RandomInteger[2], RandomInteger[1, 1000]}, 1000]]
    , RandomSeeding -> 0]
  |>;

  $defaultMeasurementsCount = 5;

  $measurementsCount = If[Length @ $ScriptCommandLine >= 4,
    ToExpression[$ScriptCommandLine[[4]]],
    $defaultMeasurementsCount];
  If[!IntegerQ[$measurementsCount] || $measurementsCount < 2,
    Print["The third argument should be an integer measurements count of at least 2."];
    Exit[1];
  ];

  Attributes[meanAroundTiming] = {HoldAll};
  meanAroundTiming[expr_] := MeanAround @ Table[First @ AbsoluteTiming[expr], $measurementsCount];

  runTests[repo_, sha_, tests_] := ModuleScope[
    Print["Testing ", sha];
    GitCheckoutReference[repo, sha];
    CloseKernels[];
    {kernel} = LaunchKernels[1];
    result = ParallelEvaluate[
      Get[FileNameJoin[{$postTagSystemRoot, "Kernel", "init.m"}]];
      Check[
        meanAroundTiming @@@ KeyMap[ReleaseHold, ReleaseHold @ Map[Hold, tests, {3}]]
      ,
        $Failed
      ]
    ,
      kernel
    ];
    Print[""];
    result
  ];

  speedupDelta[old_, new_] := (old - new) / old;

  $gitRepo = GitOpen[$PostTagSystemRoot];
  $currentSHA = GitSHA[$gitRepo, $gitRepo["HEAD"]];
  $cleanQ = AllTrue[# === {} &] @ GitStatus[$gitRepo];

  If[!$cleanQ,
    Print["Current git tree must be clean."];
    Exit[1];
  ];

  $oldSHA = If[Length @ $ScriptCommandLine >= 2, $ScriptCommandLine[[2]], "master"];
  $newSHA = If[Length @ $ScriptCommandLine >= 3, $ScriptCommandLine[[3]], $currentSHA];

  $symbolicRef = RunProcess[{"git", "symbolic-ref", "--short", "HEAD"}];
  $originalRef = If[$symbolicRef["ExitCode"] === 0, StringExtract[$symbolicRef["StandardOutput"], 1], $currentSHA];

  {$oldResults, $newResults} = runTests[$gitRepo, #, tests] & /@ {$oldSHA, $newSHA};
  RunProcess[{"git", "checkout", "-q", $originalRef}];

  (* We need to return to the original branch before exiting. *)
  If[$oldResults === $Failed || $newResults === $Failed,
    Print["Messages occured while running the tests. Exiting."];
    Exit[1];
  ];

  $speedup = speedupDelta[$oldResults, $newResults];

  $redColor = "\033[0;31m";
  $greenColor = "\033[0;32m";
  $whiteColor = "\033[0;37m";
  $endColor = "\033[0m";

  differenceString[meanAround_] := With[{
      magnitude = QuantityMagnitude[meanAround, "Percent"]},
    If[5 * magnitude[[2]] < Abs[magnitude[[1]]], If[magnitude[[1]] > 0, $greenColor, $redColor], $whiteColor] <>
    FirstCase[
      MakeBoxes[magnitude, StandardForm],
      TemplateBox[{value_, error_}, "Around", ___] :> value <> " \[PlusMinus] " <> error <> " %"] <>
    $endColor
  ];

  KeyValueMap[
    Print[
      #1,
      StringJoin[ConstantArray[" ", Max[40 - StringLength[#1], 1]]],
      differenceString[#2]] &,
    $speedup];
,
  Print["Message occurred while running the script. Exiting."];
  Exit[1];
]
