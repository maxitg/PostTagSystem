Package["PostTagSystem`"]

PackageImport["GeneralUtilities`"]

(* Note: this is actually set in Kernel/init.m, but it needs to be exported from a new-style package so that
the new-style package loader understands that this is a global symbol *)
PackageExport["$PostTagSystemRootDirectory"]

SetUsage @ "
$PostTagSystemRootDirectory contains the path of the root of the PostTagSystem package that was loaded. \
This corresponds to the directory that containts the Kernel/ directory.
";
