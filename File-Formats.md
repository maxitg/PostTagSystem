# File formats

- Diagrams generated with http://www.luismg.com/protocol/
- All integers stored in little-endian order

## Crib file (list of known sequence checkpoints for "chase" evaluation)

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


## Init file (list of initial states for "pounce" evaluation)

### Version 1

- Magic number: `0x49` (`'I'`)
- Version: `0x01`
- State count: `u64`
  - Number of initial states stored in the file
- Initial state (repeating)
  - State header: `u8`
    - Bits 0-1: Head state as unsigned 2-bit integer (0 to 3)
    - Bits 2-7: Bit count (number of significant bits in following `u64`)
      - Unsigned 6-bit integer; zero-indexed (`0` means 1 bit; `63` means 64 bits)
  - Bits: packed bit array of up to 64 bits, padded at the end with zeroes to the nearest 8-bit boundary
    - Number of significant bits indicated by bits 2-7 of the state header

Diagram shows a single initial state of 25 bits.

```
 0                   1                   2                   3  
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Magic ['I']  | Version [0x01]|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
|                          State count                          |
+                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               | HS| Bit count |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               Bits              |   Padding   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

`Magic ['I']:8,Version [0x01]:8,State count:64,HS:2,Bit count:6,Bits:25,Padding:7`

