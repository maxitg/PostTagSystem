Package["PostTagSystemDevUtils`"]

PackageImport["GeneralUtilities`"]

PackageImport["PacletManager`"] (* for PacletFind, PacletInstall in versions prior to 12.1 *)

PackageExport["$PostTagSystemGitLinkAvailableQ"]

(* unfortunately, owing to a bug in GitLink, GitLink *needs* to be on the $ContextPath or GitRepo objects
end up in the wrong context, since they are generated in a loopback link unqualified *)
$PostTagSystemGitLinkAvailableQ := !FailureQ[Quiet @ Check[Needs["GitLink`"], $Failed]];

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

PostTagSystemGitSHAWithDirtyStar[_] /; FalseQ[$PostTagSystemGitLinkAvailableQ] := Missing["NotAvailable"];

PackageExport["PostTagSystemInstallGitLink"]

SetUsage @ "
PostTagSystemInstallGitLink[] will attempt to install GitLink on the current system (if necessary).
";

PostTagSystemInstallGitLink[] := If[PacletFind["GitLink", "Internal" -> All] === {},
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
  gitRepo = GitLink`GitOpen[repoDir];
  If[$internalBuildQ, GitLink`GitFetch[gitRepo, "origin"]];
  minorVersionNumber = Max[0, Length[GitLink`GitRange[
    gitRepo,
    Except[versionInformation["Checkpoint"]],
    GitLink`GitMergeBase[gitRepo, "HEAD", masterBranch]]] - 1];
  minorVersionNumber
];
