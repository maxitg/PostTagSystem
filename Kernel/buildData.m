Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

PackageScope["$packageRoot"]

$packageRoot = FileNameDrop[$InputFileName, -2];

PostTagSystem::jitbuildfail =
  "Failed to (re)build libPostTagSystem. The existing library, if any, will be used instead.";

(* before loading build data, we check if we are running on a developer's machine, indicated by
the presence of the DevUtils sub-package, if so, we load it and do a rebuild, so that we can
get up-to-date versions of the various build properties *)
$devUtilsPath = FileNameJoin[{$packageRoot, "DevUtils", "init.m"}];
If[FileExistsQ[$devUtilsPath],
  Block[{$ContextPath = {"System`"}}, Get[$devUtilsPath]];

  (* forwarders for the functions we want from DevUtils. This is done so
  we don't create the PostTagSystemDevUtils context for ordinary users (when DevUtils *isn't* available) *)
  buildLibPostTagSystem = Symbol["PostTagSystemDevUtils`BuildLibPostTagSystem"];
  gitSHAWithDirtyStar = Symbol["PostTagSystemDevUtils`GitSHAWithDirtyStar"];

  (* try build the C++ code immediately (which will most likely retrieve a cached library) *)
  (* if there is a frontend, then give a temporary progress panel, otherwise just Print *)
  If[TrueQ @ $Notebooks,
    (* WithLocalSettings will run the final 'cleanup' argument even if the evaluation of the second
    argument aborts (due to a Throw, user abort, etc.) *)
    Internal`WithLocalSettings[
      $progCell = None;
    ,
      $buildResult = buildLibPostTagSystem["PreBuildCallback" -> Function[
        $progCell = PrintTemporary @ Panel[
          "Building libPostTagSystem from sources in " <> #LibrarySourceDirectory,
          Background -> LightOrange]]];
    ,
      NotebookDelete[$progCell];
      $progCell = None;
    ];
  ,
    $buildResult = buildLibPostTagSystem["PreBuildCallback" -> "Print"];
  ];

  If[!AssociationQ[$buildResult],
    Message[PostTagSystem::jitbuildfail];
  ];
];

readJSONFile[file_] := Quiet @ Check[Developer`ReadRawJSONFile[file], $Failed];

PackageExport["$PostTagSystemLibraryBuildTime"]
PackageExport["$PostTagSystemLibraryPath"]

SetUsage @ "
$PostTagSystemLibraryBuildTime gives the date object at which this C++ libPostTagSystem library was built.
";

SetUsage @ "
$PostTagSystemLibraryPath stores the path of the C++ libPostTagSystem library.
";

$libraryDirectory = FileNameJoin[{$packageRoot, "LibraryResources", $SystemID}];
$libraryBuildDataPath = FileNameJoin[{$libraryDirectory, "libPostTagSystemBuildInfo.json"}];

$buildData = readJSONFile[$libraryBuildDataPath];
If[$buildData === $Failed,
  $PostTagSystemLibraryBuildTime = $PostTagSystemLibraryPath = Missing["LibraryBuildDataNotFound"];
,
  $PostTagSystemLibraryBuildTime = DateObject[$buildData["LibraryBuildTime"], TimeZone -> "UTC"];
  $PostTagSystemLibraryPath = FileNameJoin[{$libraryDirectory, $buildData["LibraryFileName"]}];
];

PackageExport["$PostTagSystemBuildTime"]
PackageExport["$PostTagSystemGitSHA"]

SetUsage @ "
$PostTagSystemBuildTime gives the time at which this PostTagSystem paclet was built.
* When evaluated for an in-place build, this time is the time at which PostTagSystem was loaded.
";

SetUsage @ "
$PostTagSystemGitSHA gives the Git SHA of the repository from which this SetRepace paclet was built.
* When evaluated for an in-place build, this is simply the current HEAD of the git repository.
";

$pacletBuildInfoPath = FileNameJoin[{$packageRoot, "PacletBuildInfo.json"}];

If[FileExistsQ[$pacletBuildInfoPath] && AssociationQ[$pacletBuildInfo = readJSONFile[$pacletBuildInfoPath]],
  $PostTagSystemBuildTime = DateObject[$pacletBuildInfo["BuildTime"], TimeZone -> "UTC"];
  $PostTagSystemGitSHA = $pacletBuildInfo["GitSHA"];
,
  $PostTagSystemGitSHA = gitSHAWithDirtyStar[$packageRoot];
  If[!StringQ[$PostTagSystemGitSHA], Missing["GitLinkNotAvailable"]];
  $PostTagSystemBuildTime = DateObject[TimeZone -> "UTC"];
];
