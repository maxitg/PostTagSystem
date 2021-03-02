BeginPackage["PostTagFileFormats`CribFile`"]

Begin["`Private`"]

Needs["PostTagFileFormats`Utility`"]

$MagicNumber = "C"
$CurrentFormatVersion = 1

Export::"posttagcrib-invalid" = "The supplied crib data is invalid."
Export::"posttagcrib-unsupportedversion" = "Crib file version `1` is not supported."

(* ::Section:: *)
(* Write a crib file to disk by opening a stream *)

writeCribFile[path_String, data_List, ___] := Module[{stream, result},
    stream = OpenWrite[path, BinaryFormat -> True];
    result = writeCribFile[stream, data];
    Close[stream];
    Return[result]
]

(* ::Section:: *)
(* Write a crib file to a stream and delegate to the appropriate version handler *)

writeCribFile[
    stream_OutputStream,
    cribs:{Rule[{__Integer}, _Integer] ..},
    ___
] := Block[{$ByteOrdering = -1}, Module[{
    version = $CurrentFormatVersion
},
    BinaryWrite[stream, $MagicNumber, "Character8"];
    BinaryWrite[stream, version, "UnsignedInteger8"];

    Return@writeCribFileBody[version, stream, cribs];
]]

writeCribFile[
    stream_OutputStream,
    data_,
    ___
] := (
    Message[Export::"posttagcrib-invalid"];
    Return[$Failed]
)

(* ::Section:: *)
(* Unknown file version *)

writeCribFileBody[version_, ___] := (
    Message[Export::"posttagcrib-unsupportedversion", version];
    Return[$Failed]
)

(* ::Section:: *)
(* File version 1 *)

(* ::Subsection:: *)
(* Write a crib file (starting after magic and version) *)

writeCribFileBody[version: 1, stream_OutputStream, cribs_List] := (
    BinaryWrite[stream, Length[cribs], "UnsignedInteger64"];
    Scan[
        writeCribFileEntry[version, stream, #] &,
        cribs
    ]
)

(* ::Subsection:: *)
(* Write a single crib entry *)

writeCribFileEntry[
    version: 1,
    stream_OutputStream,
    Rule[tape_List, headState_Integer]
] := (
    BinaryWrite[stream, headState, "UnsignedInteger8"];
    BinaryWrite[stream, Length[tape], "UnsignedInteger64"];

    PostTagFileFormats`Utility`writePackedBits[stream, tape];
)

(* ::Section:: *)
(* Register export converter *)

ImportExport`RegisterExport[
    "PostTagCrib",
    writeCribFile,
    "FunctionChannels" -> {"Streams"},
    "BinaryFormat" -> True
]

End[]

EndPackage[]
