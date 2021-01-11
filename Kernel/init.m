Unprotect["PostTagSystem`*"];

(* this is a no-op the first time round, subsequent loads will unload the C++ library first *)
PostTagSystem`PackageScope`unloadLibrary[];

ClearAll @@ (# <> "*" & /@ Contexts["PostTagSystem`*"]);

(* Make sure that we don't affect $ContextPath by getting GU, and that we are isolated from
any user contexts *)
BeginPackage["PostTagSystem`"];

(* This is useful for various purposes, like loading libraries from the 'same place' as the
paclet, and also knowing *where* the loaded code even came from. *)
$PostTagSystemRootDirectory = FileNameDrop[$InputFileName, -2];

Needs["GeneralUtilities`"];

(* ensure private symbols we use below don't show up on Global, etc *)
Begin["PostTagSystem`Private`"];

Block[
  (* Temporarily overrule some of the more exotic features of the macro system.
  TODO: Fix this upstream when GU is open sourced. *)
  {GeneralUtilities`Control`PackagePrivate`$DesugaringRules = {
    HoldPattern[$Unreachable] :> Unreachable[$LHSHead],
    HoldPattern[ReturnFailed[msg_String, args___]] :> ReturnFailed[MessageName[$LHSHead, msg], args],
    HoldPattern[ReturnFailure[msg_String, args___]] :> ReturnFailure[MessageName[$LHSHead, msg], args]
  }},
  Get[FileNameJoin[{$PostTagSystemRootDirectory, "Kernel", "globals.m"}]];
];

End[];
EndPackage[];

SetAttributes[#, {Protected, ReadProtected}] & /@ Evaluate @ Names @ "PostTagSystem`*";
