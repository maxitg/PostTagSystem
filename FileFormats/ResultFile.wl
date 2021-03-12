BeginPackage["PostTagFileFormats`ResultFile`"]

Begin["`Private`"]

Needs["PostTagFileFormats`Utility`"]

$MagicNumber = "R"

$ConclusionReasons = <|
    0 -> "FileFormatError",
    1 -> "InvalidInput",
    2 -> "Terminated",
    3 -> "ReachedCycle",
    4 -> "ReachedKnownCheckpoint",
    5 -> "MaxTapeLengthExceeded",
    6 -> "MaxEventCountExceeded",
    7 -> "TimeConstraintExceeded",
    8 -> "NotEvaluated",
    9 -> "MergedWithAnotherInit"
|>

(* ::Section:: *)
(* Read a result file from disk by opening a stream *)

readResultFile[path_String, ___] := Module[{stream, result},
    stream = OpenRead[path, BinaryFormat -> True];
    result = readResultFile[stream];
    Close[stream];
    Return[result]
]

(* ::Section:: *)
(* Read a result file from a stream and delegate to the appropriate version handler *)

readResultFile[stream_InputStream, ___] := Block[{$ByteOrdering = -1}, Module[{
    magic, version
},
    magic = BinaryRead[stream, "Character8"];
    If[magic =!= $MagicNumber,
        Return@Failure["BadMagic", <|
            "MessageTemplate" -> "Invalid magic number encountered",
            "MagicNumber" -> magic
        |>]
    ];

    version = BinaryRead[stream, "UnsignedInteger8"];

    Return@readResultFileBody[version, stream];
]]

(* ::Section:: *)
(* Unknown file version *)

readResultFileBody[version_, ___] := Failure["UnsupportedVersion", <|
    "MessageTemplate" -> "File version `1` is not supported",
    "MessageParameters" -> {version}
|>]

(* ::Section:: *)
(* File version 1 *)

(* ::Subsection:: *)
(* Read a result file (starting after magic and version) *)

readResultFileBody[version: 1, stream_InputStream] := Module[{resultCount},
    resultCount = BinaryRead[stream, "UnsignedInteger64"];
    Return@Table[
        readResultFileResult[version, stream],
        resultCount
    ]
]

(* ::Subsection:: *)
(* Read a single result entry *)

readResultFileResult[version: 1, stream_InputStream] := Module[{
    result = <||>,
    initialState = <||>,
    finalState = <||>,

    initialStateHeaderByte,

    resultHeaderByte,
    conclusionReasonNumber,
    finalStateTapePresent
},
    initialStateHeaderByte = BinaryRead[stream, "UnsignedInteger8"];

    initialState["HeadState"] = BitShiftRight[BitAnd[initialStateHeaderByte, 2^^11000000], 6];
    initialState["TapeSize"] = 1 + BitAnd[initialStateHeaderByte, 2^^00111111];
    initialState["Tape"] = PostTagFileFormats`Utility`readPackedBitsAsInteger[
        stream,
        initialState["TapeSize"]
    ];
    result["InitialState"] = initialState;

    {
        resultHeaderByte,
        result["EventCount"],
        result["MaxTapeSize"],
        finalState["TapeSize"]
    } = BinaryRead[stream, {
        "UnsignedInteger8",
        "UnsignedInteger64",
        "UnsignedInteger64",
        "UnsignedInteger64"
    }];

    conclusionReasonNumber = BitShiftRight[BitAnd[resultHeaderByte, 2^^11111000], 3];
    result["Reason"] = Lookup[
        $ConclusionReasons,
        conclusionReasonNumber,
        Missing["Unknown", conclusionReasonNumber]
    ];

    finalState["HeadState"] = BitShiftRight[BitAnd[resultHeaderByte, 2^^00000110], 1];
    finalStateTapePresent = BitAnd[resultHeaderByte, 2^^00000001] === 1;

    If[finalStateTapePresent,
        finalState["Tape"] = PostTagFileFormats`Utility`readPackedBitsAsInteger[
            stream,
            finalState["TapeSize"]
        ]
    ];

    result["FinalState"] = finalState;

    Return[result]
]

(* ::Section:: *)
(* Register import converter *)

ImportExport`RegisterImport[
    "PostTagResult",
    readResultFile,
    "FunctionChannels" -> {"Streams"},
    "BinaryFormat" -> True
]

End[]

EndPackage[]
