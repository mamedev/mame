# ALG1 (Internal Sinus) Analysis - Program 1

## Problem Statement
Program 1 produces **no sound** when playing notes. WXY trace shows:
- `wf=0x000` - External mode (should be internal sinus 0x100)
- `phi=0x000` - Phase stuck at zero (not incrementing)
- `y=0` - Amplitude is ZERO

## Test Setup
```bash
# Create test MIDI: /tmp/test_prog1.mid - Note C4 (60) on program 1
cd /home/jeff/bastel/mame/mame
./mamemuse keyfox10 -rompath /home/jeff/bastel/roms/hohner -oslog -seconds_to_run 3 -midiin /tmp/test_prog1.mid 2>&1
```

## Key Finding: CPU Writes Only Zeros

### Program 2 (Piano - WORKS):
```
D-RAM[22] = 0x6DF80 (slot 2, word 2) - amplitude with bit 18 set
D-RAM[20] = 0x000C7 (slot 2, word 0) - pitch delta (DPHI)
D-RAM[2F] = 0x00202 (slot 2, word 15) - ALG=2, M=1
```

### Program 1 (Organ - NO SOUND):
```
D-RAM[02] = 0x00000 (slot 0, word 2) - amplitude = ZERO
D-RAM[00] = 0x00000 (slot 0, word 0) - DPHI = ZERO (not initialized)
D-RAM[0F] = 0x00101 (slot 0, word 15) - ALG=1, M=1
```

## Microcode Dump: ALG1 (addresses 0x20-0x3F in A-RAM)

```
ALG1[00]: 0x08FD  RM0, WXY,clearB,WWF,WACC
ALG1[01]: 0x006F  RM0,WA,WB,WM,WPHI
ALG1[02]: 0x18BF  RM1,WA,WM,WPHI,clearB
ALG1[03]: 0x02DF  RM0,WB,WM,WPHI,clearB
ALG1[04]: 0x093F  RM0 WSP,WA,WM,WPHI,clearB
ALG1[05]: 0x0ADB  RM0 WSP,WB,WM,WPHI,WXY
ALG1[06]: 0x13DF  RM1,WB,WM,WPHI,clearB
ALG1[07]: 0x11F7  RM1,WA,WPHI,WXY,clearB
ALG1[08]: 0x28FD  RM2,WXY,clearB,WWF,WACC
ALG1[09]: 0x206F  RM2,WA,WB,WM,WPHI
ALG1[10]: 0x38BF  RM3,WA,WM,WPHI,clearB
ALG1[11]: 0x22DE  RM2,WB,WM,WPHI,clearB,WACC
ALG1[12]: 0x293F  RM2 WSP,WA,WM,WPHI,clearB
ALG1[13]: 0x2ADB  RM2 WSP,WB,WM,WPHI,WXY
ALG1[14]: 0x33DF  RM3,WB,WM,WPHI,clearB
ALG1[15]: 0x31F7  RM3,WA,WPHI,WXY,clearB
ALG1[16]: 0x48FD  RM4,WXY,clearB,WWF,WACC
ALG1[17]: 0x406F  RM4,WA,WB,WM,WPHI
ALG1[18]: 0x58BF  RM5,WA,WM,WPHI,clearB
ALG1[19]: 0x42DE  RM4,WB,WM,WPHI,clearB,WACC
ALG1[20]: 0x493F  RM4 WSP,WA,WM,WPHI,clearB
ALG1[21]: 0x4ADB  RM4 WSP,WB,WM,WPHI,WXY
ALG1[22]: 0x63DF  RM6,WB,WM,WPHI,clearB
ALG1[23]: 0x6BDF  RM6 WSP,WB,WM,WPHI,clearB
ALG1[24]: 0x51F7  RM5,WA,WPHI,WXY,clearB
ALG1[25]: 0x60B7  RM6,WA,WM,WPHI,WXY,clearB
ALG1[26]: 0x687F  RM6 WSP,WA,WB,WM,WPHI
ALG1[27]: 0x63DF  RM6,WB,WM,WPHI,clearB
ALG1[28]: 0x7EFE  RM7,WB,clearB,WWF,WACC
ALG1[29]: 0x7FFF  RM7 - reserved
```

## Analysis TODO

- [ ] Trace firmware at PC=761C to understand why word 2/0 are zero
- [ ] Check if ALG=1 expects different D-RAM word assignments
- [ ] Decode ALG1 microcode to understand expected data flow
- [ ] Compare with program 2 firmware initialization at PC=744F

## D-RAM Word Usage (per programmer's guide)

| Word | Typical Use | Program 2 Value | Program 1 Value |
|------|-------------|-----------------|-----------------|
| 0 | DPHI (pitch delta) | 0x000C7 | 0x00000 |
| 1 | ? | ? | 0x00000 |
| 2 | Envelope/Amplitude | 0x6DF80 | 0x00000 |
| 3 | Envelope delta | 0x00000 | 0x00000 |
| 15 | ALG/I/M control | 0x00202 | 0x00101 |

## Microcode Analysis

### ALG1 Full Disassembly (32 instructions at A-RAM 0x20-0x3F)

```
[00] 0x08FD: MAD= 1 RM        -> WWF
[01] 0x006F: MAD= 0 RM        -> WA,WPHI
[02] 0x18BF: MAD= 3 RM        -> WB
[03] 0x02DF: MAD= 0 RADD      -> WM
[04] 0x093F: MAD= 1 RM   WSP  -> WA,WB
[05] 0x0ADB: MAD= 1 RADD      -> WM,clearB
[06] 0x13DF: MAD= 2 RADD WSP  -> WM            ← Envelope update to word 2
[07] 0x11F7: MAD= 2 RM   WSP  -> WXY           ← Uses word 2 for amplitude
[08] 0x28FD: MAD= 5 RM        -> WWF
[09] 0x206F: MAD= 4 RM        -> WA,WPHI
[10] 0x38BF: MAD= 7 RM        -> WB
[11] 0x22DE: MAD= 4 RADD      -> WM,WACC
[12] 0x293F: MAD= 5 RM   WSP  -> WA,WB
[13] 0x2ADB: MAD= 5 RADD      -> WM,clearB
[14] 0x33DF: MAD= 6 RADD WSP  -> WM            ← Envelope update to word 6
[15] 0x31F7: MAD= 6 RM   WSP  -> WXY           ← Uses word 6 for amplitude
[16] 0x48FD: MAD= 9 RM        -> WWF
[17] 0x406F: MAD= 8 RM        -> WA,WPHI
[18] 0x58BF: MAD=11 RM        -> WB
[19] 0x42DE: MAD= 8 RADD      -> WM,WACC
[20] 0x493F: MAD= 9 RM   WSP  -> WA,WB
[21] 0x4ADB: MAD= 9 RADD      -> WM,clearB
[22] 0x63DF: MAD=12 RADD WSP  -> WM
[23] 0x6BDF: MAD=13 RADD WSP  -> WM
[24] 0x51F7: MAD=10 RM   WSP  -> WXY           ← Uses word 10 for amplitude
[25] 0x60B7: MAD=12 RM        -> WB,WXY
[26] 0x687F: MAD=13 RM        -> WA
[27] 0x63DF: MAD=12 RADD WSP  -> WM
[28] 0x7EFE: MAD=15 RSP       -> WACC
[29] 0x7FFF: MAD=15 RSP  WSP  -> (reserved)
```

### ALG1 Data Flow Analysis

**Oscillator 1 (words 0-3):**
- Word 0: Phase accumulator (PHI)
- Word 1: Envelope state
- Word 2: Amplitude for WXY
- Word 3: Pitch delta (DPHI)

**Oscillator 2 (words 4-7):**
- Word 4: Phase accumulator
- Word 5: Envelope state
- Word 6: Amplitude for WXY
- Word 7: Pitch delta

**Oscillator 3 (words 8-11):**
- Word 8: Phase accumulator
- Word 9: Envelope state
- Word 10: Amplitude for WXY
- Word 11: Pitch delta

**KEY FINDING**: ALG1 is a **3-oscillator FM/additive synthesis** algorithm!
Each oscillator needs: phase (word N), envelope (word N+1), amplitude (word N+2), pitch delta (word N+3)

**KEY FINDING**: Both ALG1 and ALG2 use **word 2** for amplitude!

## Root Cause

The firmware writes **0** to word 2 for ALG=1 slots but writes non-zero values
(like 0x6DF80) for ALG=2 slots. Both algorithms expect amplitude in word 2.

Possible causes:
1. Different envelope state machine path for ALG=1 vs ALG=2
2. Program data specifies zero initial amplitude for ALG=1
3. Missing emulation feature that triggers envelope attack

## Key Difference: Envelope Update Calls

**Program 2 (piano) - envelope updates called:**
```
PC=71F6: Writes DPHI (word 0) = 0x000C7
PC=744F: Writes amplitude (word 2) = 0x6D300, 0x6D440, etc.
```

**Program 1 (organ) - envelope updates NOT called:**
```
Only PC=761C: Initialization with zeros
NO calls to PC=71F6 or PC=744F
```

## Program Data Differences

| Offset | Prog 1 | Prog 2 | Meaning |
|--------|--------|--------|---------|
| 15 | 0x49 | 0x61 | ? |
| 17 | 0x55 | 0x6D | ? |
| 33 | 0x00 | 0x0B | Envelope rate? |
| 34 | 0xF4 | 0x07 | Envelope level? |
| 36 | 0x00 | 0x10 | ? |
| 38 | 0x00 | 0x02 | ? |

The 0x00 at offset 33 for program 1 might tell the state machine to skip envelope updates.

## Theory

ALG=1 microcode IS designed to update amplitude internally through WM_WSP:
```
[06] 0x13DF: MAD=2 RADD WSP -> WM  ← Writes to word 2 with WSP
```

But this requires non-zero initial values to work. If word 2 and word 3 start at 0:
- word0 + word3 = 0 (phase never changes)
- word2 accumulation = 0 (amplitude stays 0)

## Next Steps

1. **Check if program data at offset 33-38 controls envelope initialization**
2. **Trace what happens in FUN_CODE_7d6f_copy_program for both programs**
3. **Look for where word 3 (pitch delta) is supposed to be initialized**
4. **Check if there's a key velocity → amplitude lookup table**
