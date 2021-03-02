BeginPackage["PostTagFileFormats`InitFile`"]

Begin["`Private`"]

Needs["PostTagFileFormats`Utility`"]

$MagicNumber = "I"
$CurrentFormatVersion = 1

Export::"posttaginit-invalid" = "The supplied init data is invalid."
Export::"posttaginit-unsupportedversion" = "Init file version `1` is not supported."

(* ::Section:: *)
(* Write an init file to disk by opening a stream *)

writeInitFile[path_String, data_List, ___] := Module[{stream, result},
    stream = OpenWrite[path, BinaryFormat -> True];
    result = writeInitFile[stream, data];
    Close[stream];
    Return[result]
]

(* ::Section:: *)
(* Write an init file to a stream and delegate to the appropriate version handler *)

writeInitFile[
    stream_OutputStream,
    inits:{Repeated@Rule[
            {__Integer}?(Length[#] <= 64 &),
            _Integer
        ]},
    ___
] := Block[{$ByteOrdering = -1}, Module[{
    version = $CurrentFormatVersion
},
    BinaryWrite[stream, $MagicNumber, "Character8"];
    BinaryWrite[stream, version, "UnsignedInteger8"];

    Return@writeInitFileBody[version, stream, inits];
]]

writeInitFile[
    stream_OutputStream,
    data_,
    ___
] := (
    Message[Export::"posttaginit-invalid"];
    Return[$Failed]
)

(* ::Section:: *)
(* Unknown file version *)

writeInitFileBody[version_, ___] := (
    Message[Export::"posttaginit-unsupportedversion", version];
    Return[$Failed]
)

(* ::Section:: *)
(* File version 1 *)

(* ::Subsection:: *)
(* Write an init file (starting after magic and version) *)

writeInitFileBody[version: 1, stream_OutputStream, inits_List] := (
    BinaryWrite[stream, Length[inits], "UnsignedInteger64"];
    Scan[
        writeInitFileEntry[version, stream, #] &,
        inits
    ]
)

(* ::Subsection:: *)
(* Write a single init entry *)

writeInitFileEntry[
    version: 1,
    stream_OutputStream,
    Rule[tape_List, headState_Integer]
] := (
    BinaryWrite[
        stream,
        BitOr[
            BitShiftLeft[BitAnd[headState, 2^^00000011], 6],
            BitAnd[Length[tape] - 1, 2^^00111111]
        ],
        "UnsignedInteger8"
    ];

    PostTagFileFormats`Utility`writePackedBits[stream, tape];
)

(* ::Section:: *)
(* Register export converter *)

ImportExport`RegisterExport[
    "PostTagInit",
    writeInitFile,
    "FunctionChannels" -> {"Streams"},
    "BinaryFormat" -> True
]

End[]

EndPackage[]
