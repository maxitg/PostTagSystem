BeginPackage["PostTagFileFormats`Utility`"]

PostTagFileFormats`Utility`readPackedBits
PostTagFileFormats`Utility`writePackedBits

Begin["`Private`"]

readPackedBits[
    stream_InputStream,
    bitCount_Integer
] := Take[
    Flatten@IntegerDigits[
        BinaryReadList[stream, "UnsignedInteger8", Ceiling[bitCount / 8]],
        2,
        8
    ],
    bitCount
]

writePackedBits[
    stream_OutputStream,
    bits_List
] := BinaryWrite[
    stream,
    FromDigits[#, 2] & /@ Partition[bits, 8, 8, 1, 0],
    "UnsignedInteger8"
]

End[]

EndPackage[]
