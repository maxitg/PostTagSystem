# File formats

- Diagrams generated with http://www.luismg.com/protocol/
- All integers stored in little-endian order

## Crib file (list of known sequence checkpoints)

### Version 1

- Magic number: `0x43` (`'C'`)
- Version: `0x01`
- Checkpoint count: `u64`
  - Number of checkpoint sequences stored in the file
- Checkpoint sequence (repeating)
  - Head state: `u8`
  - Bit count: `u64`
    - Number of bits to follow
  - Bits: packed bit array, padded at the end with zeroes to the nearest 8-bit boundary

Diagram shows a sequence of 19 bits followed by one of 16 bits.

```
 0                   1                   2                   3  
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Magic ['C']  | Version [0x01]|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
|                        Checkpoint count                       |
+                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               |   Head state  |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               +
|                           Bit count                           |
+                                               +-+-+-+-+-+-+-+-+
|                                               |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Bits        | Padding |   Head state  |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               +
|                           Bit count                           |
+                                               +-+-+-+-+-+-+-+-+
|                                               |      Bits     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               |
+-+-+-+-+-+-+-+-+
```

`Magic ['C']:8,Version [0x01]:8,Checkpoint count:64,Head state:8,Bit count:64,Bits:19,Padding:5,Head state:8,Bit count:64,Bits:16`
