Package["PostTagSystemDevUtils`"]

(* PostTagSystemDevUtils is *not* included in paclet builds, so is not visible to users,
but is available for developer workflow purposes, and is used by the build scripts *)

PackageImport["GeneralUtilities`"]

PackageExport["$PostTagSystemRoot"]
PackageExport["$PostTagSystemDevUtilsRoot"]
PackageExport["$PostTagSystemDevUtilsTemporaryDirectory"]

$PostTagSystemRoot = FileNameDrop[$InputFileName, -2];
$PostTagSystemDevUtilsRoot = FileNameDrop[$InputFileName, -1];
$PostTagSystemDevUtilsTemporaryDirectory := EnsureDirectory @ FileNameJoin[{$TemporaryDirectory, "PostTagSystem"}];
