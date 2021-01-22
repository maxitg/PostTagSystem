Package["PostTagSystemDevUtils`"]

PackageImport["GeneralUtilities`"]

PackageExport["PostTagSystemConsolePrintList"]

SetUsage @ "
PostTagSystemConsolePrintList[list$] will print a list of items in InputForm, one per line, with commas as appropriate.
";

PostTagSystemConsolePrintList[list_List] := (
  Print["{"];
  Scan[Print["  ", ToString[#, InputForm], ","]&, Most @ list];
  Print["  ", ToString[#, InputForm]]& @ Last @ list;
  Print["}"];
);

PackageExport["PostTagSystemConsoleTryEnvironment"]

SetUsage @ "
PostTagSystemConsoleTryEnvironment[var$, default$] will look up the value of the environment variable var$, but use \
default$ if it is not availabe.
";

SetAttributes[PostTagSystemConsoleTryEnvironment, HoldRest];
PostTagSystemConsoleTryEnvironment[var_, default_] := Replace[$Failed :> default] @ Environment[var];
