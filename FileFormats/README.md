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


## Result file (list of initial states and corresponding evaluation results)

### Version 1

- Magic number: `0x52` (`'R'`)
- Version: `0x01`
- Result count: `u64`
  - Number of results stored in the file
- Result (repeating)
  - Initial state header: `u8`
    - Bits 0-1: Head state as unsigned 2-bit integer (0 to 3)
    - Bits 2-7: Bit count (number of significant bits in following `u64`)
      - Unsigned 6-bit integer; zero-indexed (`0` means 1 bit; `63` means 64 bits)
  - Initial state bits: packed bit array of up to 64 bits, padded at the end with zeroes to the nearest 8-bit boundary
    - Number of significant bits indicated by bits 2-7 of the initial state header
  - Result header: `u8`
    - Bits 0-4: Conclusion reason as unsigned 5-bit integer (0 to 31)
      - `0`: Shouldn't happen (uninitialized enum?)
      - `1`: Invalid input
      - `2`: Evaluation terminated
      - `3`: Evaluation reached cycle
      - `4`: Evaluation reached known checkpoint
      - `5`: Maximum tape length exceeded
      - `6`: Maximum event count exceeded
      - `7`: Time constraint exceeded
      - `8`: Not evaluated (previous evaluation hit time constraint)
      - `9`: Evaluation merged with that of another initial state
    - Bits 5-6: Final head state as unsigned 2-bit integer (0 to 3)
    - Bit 7: Whether final state is present
  - Event count: `u64`
  - Max. intermediate tape size reached: `u64`
  - Final tape size: `u64`
  - Final state bits: packed bit array, padded at the end with zeroes to the nearest 8-bit boundary
    - Present only if indicated by bit 7 of the result header
    - Number of significant bits indicated by the final tape length field

Diagram shows a single result with a 23-bit initial state and a 32-bit final state

```
 0                   1                   2                   3  
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Magic ['R']  | Version [0x01]|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
|                          Result count                         |
+                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               | HS| Bit count |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      Initial state bits     |     Padding     |  Reason | HS|F|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Event count                          +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                     Max. intermediate size                    +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                           Final size                          +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Final state bits                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

`Magic ['R']:8,Version [0x01]:8,Result count:64,HS:2,Bit count:6,Initial state bits:23,Padding:9,Reason:5,HS:2,F:1,Event count:64,Max. intermediate size:64,Final size:64,Final state bits:32`
