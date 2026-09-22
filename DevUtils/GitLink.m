Package["PostTagSystemDevUtils`"]

PackageImport["GeneralUtilities`"]

PackageImport["PacletManager`"] (* for PacletFind, PacletInstall in versions prior to 12.1 *)

PackageExport["$PostTagSystemGitLinkAvailableQ"]
PackageExport["$PostTagSystemGitAvailableQ"]

(* unfortunately, owing to a bug in GitLink, GitLink *needs* to be on the $ContextPath or GitRepo objects
end up in the wrong context, since they are generated in a loopback link unqualified *)
(* Needs can succeed after a previous library initialization failure, so also check the library itself. *)
$PostTagSystemGitLinkAvailableQ := AssociationQ[Quiet @ Check[
  Needs["GitLink`"];
  GitLink`$GitLibraryInformation,
  $Failed
]];

$PostTagSystemGitAvailableQ := $PostTagSystemGitLinkAvailableQ ||
  TrueQ[Quiet @ Check[RunProcess[{"git", "--version"}, "ExitCode"] === 0, False]];

runGit::failed = "Git command `` failed in ``: ``";

(* An argument list avoids shell quoting and supports repository paths containing spaces. *)
runGit[repoDir_String, args__String] := Module[{result},
  result = Quiet @ Check[RunProcess[Join[{"git", "-C", repoDir}, {args}]], $Failed];
  If[!AssociationQ[result],
    Message[runGit::failed, {args}, repoDir, "Could not run git; check PATH."];
    Return[$Failed];
  ];
  If[result["ExitCode"] =!= 0,
    Message[runGit::failed, {args}, repoDir, StringTrim[result["StandardError"]]];
    Return[$Failed];
  ];
  StringTrim[result["StandardOutput"]]
];

PackageExport["PostTagSystemGitSHAWithDirtyStar"]

Clear[PostTagSystemGitSHAWithDirtyStar];

SetUsage @ "
PostTagSystemGitSHAWithDirtyStar['path$'] returns the SHA hash of the commit that is currently checked on \
for the Git repository at 'path$'. Unlike the GitSHA function, this will include a '*' character \
if the current working tree is dirty.
";

PostTagSystemGitSHAWithDirtyStar[repoDir_] /; TrueQ[$PostTagSystemGitLinkAvailableQ] := ModuleScope[
  repo = GitLink`GitOpen[repoDir];
  sha = GitLink`GitSHA[repo, repo["HEAD"]];
  cleanQ = AllTrue[# === {} &] @ GitLink`GitStatus[repo];
  If[cleanQ, sha, sha <> "*"]
];

PostTagSystemGitSHAWithDirtyStar[repoDir_] /; FalseQ[$PostTagSystemGitLinkAvailableQ] := ModuleScope[
  sha = runGit[repoDir, "rev-parse", "--verify", "HEAD"];
  If[!StringQ[sha], Return[$Failed]];
  status = runGit[repoDir, "status", "--porcelain=v1", "--untracked-files=normal", "--ignore-submodules=all"];
  If[!StringQ[status], Return[$Failed]];
  If[status === "", sha, sha <> "*"]
];

PackageExport["PostTagSystemInstallGitLink"]

SetUsage @ "
PostTagSystemInstallGitLink[] will attempt to install GitLink if neither GitLink nor command-line git is available.
";

PostTagSystemInstallGitLink[] := If[!$PostTagSystemGitAvailableQ && PacletFind["GitLink", "Internal" -> All] === {},
  PacletInstall["https://www.wolframcloud.com/obj/maxp1/GitLink-2019.11.26.01.paclet"];
];

PackageExport["PostTagSystemCalculateMinorVersionNumber"]

SetUsage @ "
PostTagSystemCalculateMinorVersionNumber[repositoryDirectory$, masterBranch$] will calculate a minor version \
derived from the number of commits between the last checkpoint and the 'master' branch, \
which can be overriden with the 'MasterBranch' option. The checkpoint is defined in scripts/version.wl.
";

PostTagSystemCalculateMinorVersionNumber[repoDir_, masterBranch_] := ModuleScope[
  versionInformation = Import[FileNameJoin[{repoDir, "scripts", "version.wl"}]];
  If[!$PostTagSystemGitLinkAvailableQ,
    If[TrueQ[$internalBuildQ] && runGit[repoDir, "fetch", "origin"] === $Failed, Return[$Failed]];
    mergeBase = runGit[repoDir, "merge-base", "HEAD", masterBranch];
    If[!StringQ[mergeBase], Return[$Failed]];
    count = runGit[repoDir, "rev-list", "--count", versionInformation["Checkpoint"] <> ".." <> mergeBase];
    If[!StringQ[count] || !StringMatchQ[count, DigitCharacter ..], Return[$Failed]];
    Return[Max[0, FromDigits[count] - 1]];
  ];
  gitRepo = GitLink`GitOpen[repoDir];
  If[$internalBuildQ, GitLink`GitFetch[gitRepo, "origin"]];
  minorVersionNumber = Max[0, Length[GitLink`GitRange[
    gitRepo,
    Except[versionInformation["Checkpoint"]],
    GitLink`GitMergeBase[gitRepo, "HEAD", masterBranch]]] - 1];
  minorVersionNumber
];
