# Keyfox10 FX Chip - Reverb Algorithm Analysis

## Status: IN PROGRESS

Started: 2026-01-13
Updated: 2026-01-15

## Overview

The Keyfox10 uses a dedicated SAM8905 chip ("SAM[FX]") for reverb/effects processing. This document analyzes the algorithms and D-RAM configuration programmed by the firmware.

## BUG FOUND AND FIXED (2026-01-14)

### Problem: FX produces no audio output

**Root cause:** Emulator bug in 22kHz mode A-RAM address calculation.

### SAM8905 A-RAM Address Format (Table 6 from datasheet)

| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 | Mode |
|-------|-------|-------|-------|-------|-------|-------|-------|------|
| AL2   | AL1   | AL0   | PC4   | PC3   | PC2   | PC1   | PC0   | SSR=0 (44.1kHz) |
| AL2   | AL1   | PC5   | PC4   | PC3   | PC2   | PC1   | PC0   | SSR=1 (22.05kHz) |

**Key insight:** In 22kHz mode, only **AL2 and AL1** (2 bits) select the algorithm.
AL0 becomes PC5 (part of the instruction counter).

### The Bug

The D-RAM word 15 ALG field is at bits 10-8: `AL2=bit10, AL1=bit9, AL0=bit8`

**Wrong code (was using bits 9-8):**
```cpp
uint8_t alg = (param15 >> 8) & 0x3;  // WRONG: extracts AL1,AL0 (bits 9-8)
```

**Fixed code (now using bits 10-9):**
```cpp
uint8_t alg = (param15 >> 9) & 0x3;  // CORRECT: extracts AL2,AL1 (bits 10-9)
```

### Verification

For slots 7-11 with param15 = 0x3C480 (ALG=4 in D-RAM field):
- Bits 10-8 = 100 binary (AL2=1, AL1=0, AL0=0)
- **Wrong:** (0x3C480 >> 8) & 0x3 = 0 → ran ALG 0 code at A-RAM 0x00
- **Fixed:** (0x3C480 >> 9) & 0x3 = 2 → runs ALG 2 code at A-RAM 0x80

### Result After Fix

| param15 ALG | 22kHz ALG (AL2,AL1) | A-RAM Base |
|-------------|---------------------|------------|
| 0 (000)     | 0 (00)              | 0x00       |
| 2 (010)     | 1 (01)              | 0x40       |
| 4 (100)     | 2 (10)              | 0x80       |
| 6 (110)     | 3 (11)              | 0xC0       |

Now slots 7-11 (ALG=4) correctly run A-RAM 0x80-0xBF which has WXY WSP at PC07!

### A-RAM Content Analysis (Correct Mapping)

| 22kHz ALG | A-RAM Range | WXY WSP Instructions | WACC Instructions |
|-----------|-------------|---------------------|-------------------|
| 0         | 0x00-0x3F   | 0                   | 0                 |
| 1         | 0x40-0x7F   | 0                   | 13                |
| 2         | 0x80-0xBF   | 1 (PC07: 0x29B7)    | 3 (PC10,11,29)    |
| 3         | 0xC0-0xFF   | 1 (PC05: 0x39F7)    | 2 (PC11,33)       |

- ALG 0: Input conditioning, no direct output
- ALG 1: Heavy diffusion output (13 WACC spread throughout)
- ALG 2: Main reverb output with stereo panning (WXY+WSP at PC07)
- ALG 3: All-pass filter, MUTED (mix_l=mix_r=0) - processes SRAM for feedback network

### Remaining Investigation

- [x] Fixed ALG bit selection for 22kHz mode
- [ ] Verify FX produces actual audio with input signal from SND chip
- [ ] Check if FX input routing is working correctly

## Critical: External SRAM Write Mechanism (from Programmer's Guide Section 9)

The SAM8905 programmer's guide documents exactly how to write to external SRAM:

```
PHI=0                      ;lower part of memory address (upper 9 bits)
WF=1                       ;0(2)|WF(8)|X(9) upper part of memory address
DATA=2                     ;data to be written in memory

RM      WF,     <WWF>              ;activate memory chip select (WCS/)
RM      PHI,    <WPHI>             ;PHIreg=PHI
RM      DATA,   <WXY>              ;prepare to output data on WD pins
RSP             <ClearB,WSP>       ;WOE/=1 (disable outputs from external mem)
RSP             <ClearB,WSP>       ;WOE/=1, apply data to WD pins
RSP             <ClearB>           ;WOE/=0 (write pulse)
RSP             <ClearB,WSP>       ;WOE/=1
RSP             <ClearB,WSP>       ;WOE/=1 data still applied
                                   ;data removed
                                   ;WOE/=0 (back to normal operation)
```

**Key insight:** The WXY instruction loads Y from bus bits [18:7]. The bus can be driven by:
- **RM** - D-RAM value (as shown in the example)
- **RP** - Product register (mul_result)
- **RADD** - A+B sum
- **RSP** - Zero

The programmer's guide example uses D-RAM, but any bus source works. Common patterns:
1. `RM D[x], <WXY>` → Y = D-RAM[x][18:7]
2. `RP x, <WXY>` → Y = product[18:7]
3. `RADD x, <WXY>` → Y = (A+B)[18:7]

**Therefore:** WWE writing Y is CORRECT. Y is loaded from whatever is on the bus
when WXY executes - this can be D-RAM, product, or A+B directly.

### Keyfox10 ALG 0 SRAM Write Analysis

ALG 0 has TWO separate WWE sequences with different data sources:

**WWE #1 (PC16-19) - From D-RAM feedback:**
```
PC16: 68F7  RM   13, <WXY>              ; Y = D[13][18:7] - from D-RAM
PC17: 38FD  RM    7, <WWF>              ; WWF = D[7] (SRAM bank select)
PC18: 7FFB  RSP  15, <clrB> [WSP]       ; WWE - writes Y (D[13] value)
PC19: 7FFB  RSP  15, <clrB> [WSP]       ; WWE - second pulse
```
D[13] is written at PC61 (end of frame), read at PC16 (next frame) = **one-frame delay feedback**

**WWE #2 (PC34-36) - From A+B computation:**
```
PC34: 7AF7  RADD 15, <WXY>              ; Y = (A+B)[18:7] - DIRECT from adder!
PC35: 7FFB  RSP  15, <clrB> [WSP]       ; WWE - writes Y (A+B result)
PC36: 7FFB  RSP  15, <clrB> [WSP]       ; WWE - second pulse
```
This writes the **current computation result** directly to SRAM, no D-RAM involved.

### Current Issue Analysis

The problem is that BOTH WWE sequences write zero/near-zero values:

1. **WWE #1:** D[13] = 0 because:
   - D[13] initialized to 0
   - PC61 writes D[13] = A+B, but A+B is small/zero at that point

2. **WWE #2:** (A+B)[18:7] = 0 because:
   - A and B accumulate values but result doesn't have significant bits in [18:7]
   - Or the computation path doesn't reach meaningful audio levels

Need to trace what values A and B actually contain at PC34 to understand why
the RADD result has zero in bits [18:7].

## Hardware Notes: FX SRAM Connection

From the schematic (IC21 FX SAM → IC19 SRAM256):

**SAM8905 has 12-bit waveform data (WD0-WD11), but FX SRAM is only 8-bit:**

| SRAM256 Pin | SAM8905 FX Pin | Notes |
|-------------|----------------|-------|
| D0          | WD3            | SRAM bit 0 → SAM bit 3 |
| D1          | WD4            | |
| D2          | WD5            | |
| D3          | WD6            | |
| D4          | WD7            | |
| D5          | WD8            | |
| D6          | WD9            | |
| D7          | WD10           | SRAM bit 7 → SAM bit 10 |
| -           | WD0-WD2        | Pulled to GND via 1k resistors |
| -           | WD11           | Generated by logic (see below) |

**WD11 Logic (IC9 74HC00):**
```
WDH11 = ~NAND(~WAH0, WDH10) = (~WAH0) AND WDH10
```
- **WAH0=0:** WDH11 = WDH10 (sign extension from bit 10)
- **WAH0=1:** WDH11 = 0

This provides **conditional sign extension** - when WAH0=0, the 8-bit SRAM data
is sign-extended from 11 bits (bit 10 copied to bit 11) to maintain proper
signed audio values.

**Effective data format when SAM reads from SRAM (WAH0=0, sign-extended):**
```
Bit:   11  10   9   8   7   6   5   4   3   2   1   0
      D7  D7  D6  D5  D4  D3  D2  D1  D0   0   0   0
       ^--- sign extension from bit 10
```

**Hardware behavior:**
- **Write:** SAM outputs 12-bit, SRAM stores only bits 10:3 → `sram = (wd >> 3) & 0xFF`
- **Read (WAH0=0):** Sign-extended 8-bit → 12-bit: `wd = (int8_t)sram << 3`
- **Read (WAH0=1):** Zero-extended: `wd = (uint8_t)sram << 3`
- Lower 3 bits always zero (truncated on write, pulled low on read)
- Effective resolution: 8 bits signed

**Emulation note:** Current emulator stores full 12-bit values for better accuracy.
For hardware-accurate emulation, use signed 8-bit with sign extension on read.

## Active Configuration

**FX SAM Mode:** 22kHz (SSR=1)
**Active slots:** 4, 5, 6, 7, 8, 9, 10, 11 (8 slots)
**param15 ALG values:** 0, 2, 4, 6 (but wrapped to 0, 2 in 22kHz mode)

| Slot | IDLE | ALG | Description |
|------|------|-----|-------------|
| 0-3  | 1    | -   | Idle (NOP - no processing) |
| 4    | 0    | 0   | Active - Algorithm 0 |
| 5    | 0    | 2   | Active - Algorithm 2 |
| 6    | 0    | 6   | Active - Algorithm 6 |
| 7-11 | 0    | 4   | Active - Algorithm 4 (5 instances!) |
| 12-15| 1    | -   | Idle (NOP - no processing) |

## Audio Output Configuration

### WACC (Accumulator to DAC) Usage

| Slot | Algorithm | WACC Instructions | Notes |
|------|-----------|-------------------|-------|
| 4    | ALG 0     | PC00 only         | Init instruction, minimal output |
| 5    | ALG 2     | PC04,07,09,13,14,19,20,30,31 | Heavy output (9 instructions) |
| 6    | ALG 6     | PC11 only         | Single output but **MUTED** |
| 7-11 | ALG 4     | PC10,11,29        | 3 outputs each |

### MIXL/MIXR Settings (Stereo Panning)

MIX is updated by `WXY WSP` instruction. Format: `| AMP (12 bits) | X | MIXL (3) | MIXR (3) | X |`

| Slot | D-RAM Word | AMP    | MIXL   | MIXR  | Notes |
|------|------------|--------|--------|-------|-------|
| 6    | word[7]    | 0xA58  | **mute** | **mute** | All-pass processes but doesn't output! |
| 7    | word[5]    | 0xCCC  | 0dB    | 0dB   | Full stereo |
| 8    | word[5]    | 0xCD4  | 0dB    | 0dB   | Full stereo |
| 9    | word[5]    | 0xCDD  | -24dB  | 0dB   | Right-biased |
| 10   | word[5]    | 0xCE9  | -24dB  | 0dB   | Right-biased |
| 11   | word[5]    | 0xCEF  | 0dB    | 0dB   | Full stereo |

**Key insight:** Slot 6 (all-pass filter) is **muted** - it processes audio internally (SRAM read/write) but contributes nothing to the output. The other slots read from its delay buffers.

### IDLE Slots Analysis

IDLE slots (0-3, 12-15) execute **NOP** regardless of their ALG setting. They:
- Do NOT write to SRAM
- Do NOT contribute to audio output
- Have minimal D-RAM initialization (internal wave configs only)
- Are truly inactive placeholders

## A-RAM Algorithms (22kHz Mode - 64 Instructions Each)

**Important:** In 22kHz mode (SSR=1), D-RAM ALG field maps to A-RAM base address:
- D-RAM ALG 0,1 → A-RAM 0x00-0x3F (22kHz ALG 0)
- D-RAM ALG 2,3 → A-RAM 0x40-0x7F (22kHz ALG 1)
- D-RAM ALG 4,5 → A-RAM 0x80-0xBF (22kHz ALG 2)
- D-RAM ALG 6,7 → A-RAM 0xC0-0xFF (22kHz ALG 3)

PC62-PC63 are reserved system instructions (not executed by algorithm).

---

### A-RAM 0x00-0x3F: 22kHz ALG 0 (D-RAM ALG=0, Used by Slot 4)

**Purpose:** Initial signal conditioning and phase accumulation for input processing.

```python
dram_slot4 = [
    0x00000,  # word 0: 
    0x50080,  # word 1: DPHI/config
    0x00400,  # word 2: WWF config
    0x40000,  # word 3: Config
    0x00080,  # word 4: Config
    0x00000,  # word 5: Accumulator
    0x7FFFF,  # word 6: (Max amplitude - says claude ???) [val=-1]
    0x40402,  # word 7: WWF/address
    0x00100,  # word 8: PHI increment
    0x00080,  # word 9: Config
    0x00180,  # word 10: Config
    0x0007C,  # word 11: Feedback coef [val=124]
    0x00000,  # word 12: accumulator state???
    0x00000,  # word 13
    0x00000,  # word 14
    0x34080,  # word 15: IDLE=0, ALG=0
]
```

```
; === INITIALIZATION PHASE (PC00-PC09) ===
PC00: 00F7  RM    0, <WXY>                    ; Y = D[0] = 0 ?, X = last rom value ???
PC01: 607F  RM   12, <WA>                     ; A = D[12] (accumulator state)
PC02: 58BF  RM   11, <WB>                     ; B = D[11] (feedback coefficient)
PC03: 5A5F  RADD 11, <WA, WM>                 ; D[11] = A+B, A = result (accumulate feedback), WA without WSP sets CLEARRQST
PC04: 30BF  RM    6, <WB>                     ; B = D[6] -> add -1
PC05: 5DDF  RP   11, <WM> [WSP]               ; D[11] = X*Y IF CLEARRQST AND NOT CARRY
PC06: 082D  RM    1, <WA, WB, WPHI, WWF>      ; Load PHI,WF from D[1] (SRAM address config)
PC07: 593F  RM   11, <WA, WB> [WSP]           ; A=B=D[11] (scaled sample)
PC08: 5ADF  RADD 11, <WM>                     ; D[11] = A+B (double for mixing)
PC09: 58F7  RM   11, <WXY>                    ; Load X,Y from D[11] for multiplication

; === DELAY TAP 1 (PC10-PC16) ===
PC10: 406F  RM    8, <WA, WPHI>               ; A = PHI = D[8] (delay offset 1)
PC11: 2CDF  RP    5, <WM>                     ; D[5] = product (store delayed sample)
PC12: 48BF  RM    9, <WB>                     ; B = D[9] (delay increment)
PC13: 58F7  RM   11, <WXY>                    ; Reload XY from D[11]
PC14: 42DF  RADD  8, <WM>                     ; D[8] = A+B (advance delay pointer)
PC15: 749F  RP   14, <WB, WM>                 ; D[14] = product, B = product
PC16: 68F7  RM   13, <WXY>                    ; Load XY from D[13]

; === SRAM WRITE SEQUENCE (PC17-PC22) ===
PC17: 38FD  RM    7, <WWF>                    ; WWF = D[7] (SRAM bank select)
PC18: 7FFB  RSP  15, <clrB> [WSP]             ; Clear B, trigger SRAM write (RSP+clrB+WSP=WWE)
PC19: 7FFB  RSP  15, <clrB> [WSP]             ; WWE trigger (timing/settling)
PC20: 7EFB  RSP  15, <clrB>                   ; Clear B (settling time)
PC21: 7EFB  RSP  15, <clrB>                   ; Clear B (settling time)
PC22: 7FFF  RSP  15, <-> [WSP]                ; NOP with WSP (sync)

; === DELAY TAP 2 (PC23-PC31) ===
PC23: 406F  RM    8, <WA, WPHI>               ; A = PHI = D[8]
PC24: 50BF  RM   10, <WB>                     ; B = D[10] (second delay increment)
PC25: 42DF  RADD  8, <WM>                     ; D[8] = A+B
PC26: 683F  RM   13, <WA, WB>                 ; A = B = D[13]
PC27: 7A3F  RADD 15, <WA, WB>                 ; A = B = A+B (double)
PC28: 7A3F  RADD 15, <WA, WB>                 ; A = B = A+B (4x)
PC29: 7A3F  RADD 15, <WA, WB>                 ; A = B = A+B (8x)
PC30: 7A3F  RADD 15, <WA, WB>                 ; A = B = A+B (16x)
PC31: 7A3F  RADD 15, <WA, WB>                 ; A = B = A+B (32x) - amplitude ramp

; === SRAM READ AND OUTPUT (PC32-PC47) ===
PC32: 7A3F  RADD 15, <WA, WB>                 ; Continue amplitude ramp (64x)
PC33: 7A3F  RADD 15, <WA, WB>                 ; (128x)
PC34: 7AF7  RADD 15, <WXY>                    ; XY = A+B result - SRAM READ via WXY
PC35: 7FFB  RSP  15, <clrB> [WSP]             ; WWE - write back to SRAM
PC36: 7FFB  RSP  15, <clrB> [WSP]             ; Second write
PC37: 7EFB  RSP  15, <clrB>                   ; Settling
PC38: 7EFB  RSP  15, <clrB>                   ; Settling
PC39: 7FFF  RSP  15, <-> [WSP]                ; Sync
PC40: 78FD  RM   15, <WWF>                    ; WWF = D[15] (another SRAM bank)            
PC41: 18EF  RM    3, <WPHI>                   ; PHI = D[3]
PC42: 58F7  RM   11, <WXY>                    ; XY = D[11] - SRAM READ
PC43: 7FFF  RSP  15, <-> [WSP]                ; Sync
PC44: 50EF  RM   10, <WPHI>                   ; PHI = D[10]
PC45: 08FD  RM    1, <WWF>                    ; WWF = D[1]
PC46: 24DF  RSP   4, <WM>                     ; D[4] = 0 (RSP emits 0)
PC47: 7FFF  RSP  15, <-> [WSP]                ; Sync

; === FINAL PROCESSING (PC48-PC61) ===
PC48: 20F7  RM    4, <WXY>                    ; XY = D[4] - READ for output mix
PC49: 287F  RM    5, <WA>                     ; A = D[5]
PC50: 00EF  RM    0, <WPHI>                   ; PHI = D[0]
PC51: 7CBF  RP   15, <WB>                     ; B = product
PC52: 2ADF  RADD  5, <WM>                     ; D[5] = A+B
PC53: 20F7  RM    4, <WXY>                    ; XY = D[4]
PC54: 707F  RM   14, <WA>                     ; A = D[14]
PC55: 7CBF  RP   15, <WB>                     ; B = product
PC56: 28EF  RM    5, <WPHI>                   ; PHI = D[5]
PC57: 78FD  RM   15, <WWF>                    ; WWF = D[15]
PC58: 10F7  RM    2, <WXY>                    ; XY = D[2] - FINAL READ
PC59: 7A7F  RADD 15, <WA>                     ; A = A+B
PC60: 7CBF  RP   15, <WB>                     ; B = product
PC61: 6A5B  RADD 13, <WA, WM, clrB>           ; D[13] = A+B, clear B (output to buffer)
; PC62-63: Reserved
```

**Analysis:** This algorithm reads input samples, scales them, and writes to SRAM delay buffers.
No WACC instructions = no direct DAC output. Output goes to SRAM for other slots to read.

**D-RAM usage:** Reads D[0-15], Writes D[4,5,8,11,13,14]

---

### A-RAM 0x40-0x7F: 22kHz ALG 1 (D-RAM ALG=2, Used by Slot 5 - Diffusion)

**Purpose:** Diffusion/scatter processing. Heavy DAC output (13 WACC instructions).
No WXY+WSP (MIX register update) - uses MIX values set by previous slot.

```
; === INITIAL SETUP (PC00-PC04) ===
PC00: 30EF  RM    6, <WPHI>                     ; PHI = D[6] (phase setup)
PC01: 48FD  RM    9, <WWF>                      ; WWF = D[9] (waveform config)
PC02: 6ADF  RADD 13, <WM>                       ; D[13] = A+B (accumulate)
PC03: 703F  RM   14, <WA, WB>                   ; A=B=D[14]
PC04: 0000  RM    0, <WA,WB,WM,WPHI,WXY,clrB,WWF,WACC>  ; WACC - DAC output

; === FIRST PROCESSING BLOCK (PC05-PC15) ===
PC05: 6BDF  RADD 13, <WM> [WSP]                 ; D[13] = A+B, WSP active
PC06: 38EF  RM    7, <WPHI>                     ; PHI = D[7]
PC07: 50FC  RM   10, <WWF,WACC>                 ; WWF=D[10], WACC - DAC output
PC08: 687F  RM   13, <WA>                       ; A = D[13]
PC09: 7CBE  RP   15, <WB,WACC>                  ; B = product, WACC - DAC output
PC10: 18F7  RM    3, <WXY>                      ; WXY - waveform read
PC11: 7A7F  RADD 15, <WA>                       ; A = A+B
PC12: 40EF  RM    8, <WPHI>                     ; PHI = D[8]
PC13: 58FC  RM   11, <WWF,WACC>                 ; WWF=D[11], WACC - DAC output
PC14: 7CBE  RP   15, <WB,WACC>                  ; B = product, WACC - DAC output
PC15: 6ADF  RADD 13, <WM>                       ; D[13] = A+B

; === SECOND PROCESSING BLOCK (PC16-PC25) ===
PC16: 18F7  RM    3, <WXY>                      ; WXY - waveform read
PC17: 00BF  RM    0, <WB>                       ; B = D[0]
PC18: 307F  RM    6, <WA>                       ; A = D[6]
PC19: 32CE  RADD  6, <WM,WPHI,WACC>             ; D[6]=A+B, PHI=bus, WACC - DAC output
PC20: 48FC  RM    9, <WWF,WACC>                 ; WWF=D[9], WACC - DAC output
PC21: 387F  RM    7, <WA>                       ; A = D[7]
PC22: 3ADF  RADD  7, <WM>                       ; D[7] = A+B
PC23: 407F  RM    8, <WA>                       ; A = D[8]
PC24: 42DF  RADD  8, <WM>                       ; D[8] = A+B
PC25: 7CBF  RP   15, <WB>                       ; B = product

; === THIRD PROCESSING BLOCK (PC26-PC37) ===
PC26: 20F7  RM    4, <WXY>                      ; WXY - waveform read
PC27: 687F  RM   13, <WA>                       ; A = D[13]
PC28: 38EF  RM    7, <WPHI>                     ; PHI = D[7]
PC29: 50FD  RM   10, <WWF>                      ; WWF = D[10]
PC30: 7A7E  RADD 15, <WA,WACC>                  ; A=A+B, WACC - DAC output
PC31: 7CBE  RP   15, <WB,WACC>                  ; B = product, WACC - DAC output
PC32: 28F7  RM    5, <WXY>                      ; WXY - waveform read
PC33: 7A7F  RADD 15, <WA>                       ; A = A+B
PC34: 40EF  RM    8, <WPHI>                     ; PHI = D[8]
PC35: 58FD  RM   11, <WWF>                      ; WWF = D[11]
PC36: 7CBE  RP   15, <WB,WACC>                  ; B = product, WACC - DAC output
PC37: 6ADE  RADD 13, <WM,WACC>                  ; D[13]=A+B, WACC - DAC output

; === ACCUMULATION CHAIN (PC38-PC59) ===
PC38: 28F7  RM    5, <WXY>                      ; WXY - waveform read
PC39: 08BF  RM    1, <WB>                       ; B = D[1]
PC40: 307F  RM    6, <WA>                       ; A = D[6]
PC41: 32DF  RADD  6, <WM>                       ; D[6] = A+B
PC42: 493F  RM    9, <WA,WB> [WSP]              ; A=B=D[9], WSP active
PC43: 4ADF  RADD  9, <WM>                       ; D[9] = A+B
PC44: 60BF  RM   12, <WB>                       ; B = D[12]
PC45: 4BDF  RADD  9, <WM> [WSP]                 ; D[9]=A+B, WSP active
PC46: 08BF  RM    1, <WB>                       ; B = D[1]
PC47: 387F  RM    7, <WA>                       ; A = D[7]
PC48: 3ADF  RADD  7, <WM>                       ; D[7] = A+B
PC49: 513F  RM   10, <WA,WB> [WSP]              ; A=B=D[10], WSP active
PC50: 52DF  RADD 10, <WM>                       ; D[10] = A+B
PC51: 60BF  RM   12, <WB>                       ; B = D[12]
PC52: 53DF  RADD 10, <WM> [WSP]                 ; D[10]=A+B, WSP active
PC53: 08BF  RM    1, <WB>                       ; B = D[1]
PC54: 407F  RM    8, <WA>                       ; A = D[8]
PC55: 42DF  RADD  8, <WM>                       ; D[8] = A+B
PC56: 593F  RM   11, <WA,WB> [WSP]              ; A=B=D[11], WSP active
PC57: 5ADF  RADD 11, <WM>                       ; D[11] = A+B
PC58: 60BF  RM   12, <WB>                       ; B = D[12]
PC59: 5BDF  RADD 11, <WM> [WSP]                 ; D[11]=A+B, WSP active

; === FINAL OUTPUT (PC60-PC63) ===
PC60: 7CBE  RP   15, <WB,WACC>                  ; B = product, WACC - DAC output
PC61: 687E  RM   13, <WA,WACC>                  ; A = D[13], WACC - DAC output
PC62: 7FFF  RSP  15, <WSP>                      ; NOP with WSP
PC63: 7FFF  RSP  15, <WSP>                      ; NOP with WSP
```

**Analysis:** This algorithm has heavy DAC output (13 WACC instructions) spread throughout.
It reads from D-RAM and does extensive multiply-accumulate operations.
Multiple WSP markers (PC05,42,45,49,52,56,59,62,63) but no WXY+WSP combo - these WSP flags
don't update MIX registers (no simultaneous WXY).

**D-RAM usage:** Reads D[0-13], Writes D[6,7,8,9,10,11,13]

---

### A-RAM 0x80-0xBF: 22kHz ALG 2 (D-RAM ALG=4, Used by Slots 7-11)

**Purpose:** Delay tap processing with stereo output. This is the PRIMARY REVERB OUTPUT.

```
; === FEEDBACK ACCUMULATION (PC00-PC06) ===
PC00: 5ADF  RADD 11, <WM>                     ; D[11] = A+B (from PREVIOUS SLOT's final state)
PC01: 68BF  RM   13, <WB>                     ; B = D[13] (feedback state)
PC02: 72DF  RADD 14, <WM>                     ; D[14] = A+B (accumulate output)
PC03: 10FD  RM    2, <WWF>                    ; WWF = D[2] (SRAM config for delay read)
PC04: 006F  RM    0, <WA, WPHI>               ; A = PHI = D[0] (delay line base address)
PC05: 18BF  RM    3, <WB>                     ; B = D[3] (delay increment)
PC06: 02DF  RADD  0, <WM>                     ; D[0] = A+B (advance delay pointer)

; === WAVEFORM READ WITH MIX UPDATE (PC07) - CRITICAL INSTRUCTION ===
PC07: 29B7  RM    5, <WB, WXY> [WSP]          ; B=D[5], READ SRAM waveform, UPDATE MIX_L/MIX_R
;     ^^ This instruction: 1) reads delay sample via WXY
;                          2) WSP updates mix_l/mix_r from D[5] bits 5:3 and 2:0
;                          3) Sets up multiplication X*Y

; === FIRST OUTPUT TO DAC (PC08-PC11) ===
PC08: 707F  RM   14, <WA>                     ; A = D[14] (accumulated output)
PC09: 7CBF  RP   15, <WB>                     ; B = X*Y product (delayed sample × amplitude)
PC10: 7A7E  RADD 15, <WA> [WACC]              ; A = A+B, OUTPUT TO DAC (first accumulation)
PC11: 72DE  RADD 14, <WM> [WACC]              ; D[14] = A+B, OUTPUT TO DAC (second accumulation)

; === COMB FILTER TAP (PC12-PC19) ===
PC12: 30F7  RM    6, <WXY>                    ; XY = D[6] (comb delay tap)
PC13: 20BF  RM    4, <WB>                     ; B = D[4] (comb coefficient)
PC14: 6CDF  RP   13, <WM>                     ; D[13] = product (filtered tap)
PC15: 006F  RM    0, <WA, WPHI>               ; A = PHI = D[0]
PC16: 02DF  RADD  0, <WM>                     ; D[0] = A+B (advance pointer)
PC17: 087F  RM    1, <WA>                     ; A = D[1]
PC18: 28F7  RM    5, <WXY>                    ; XY = D[5] - second delay read
PC19: 0ADF  RADD  1, <WM>                     ; D[1] = A+B

; === ALL-PASS SECTION (PC20-PC31) ===
PC20: 7CEF  RP   15, <WPHI>                   ; PHI = product (computed address)
PC21: 78FD  RM   15, <WWF>                    ; WWF = D[15] (all-pass SRAM bank)
PC22: 40F7  RM    8, <WXY>                    ; XY = D[8] - all-pass delay read
PC23: 687F  RM   13, <WA>                     ; A = D[13]
PC24: 7CBF  RP   15, <WB>                     ; B = product
PC25: 6ADF  RADD 13, <WM>                     ; D[13] = A+B (all-pass accumulation)
PC26: 38F7  RM    7, <WXY>                    ; XY = D[7] - second all-pass tap
PC27: 707F  RM   14, <WA>                     ; A = D[14]
PC28: 7CBF  RP   15, <WB>                     ; B = product
PC29: 725E  RADD 14, <WA, WM> [WACC]          ; D[14]=A+B, OUTPUT TO DAC (third accumulation)
PC30: 50BF  RM   10, <WB>                     ; B = D[10]
PC31: 797B  RM   15, <WA, clrB> [WSP]         ; A=D[15], clear B, special operation

; === SRAM WRITE-BACK (PC32-PC39) ===
PC32: 7AEF  RADD 15, <WPHI>                   ; PHI = A+B (compute write address)
PC33: 78FD  RM   15, <WWF>                    ; WWF = D[15] (SRAM bank)
PC34: 60F7  RM   12, <WXY>                    ; XY = D[12] - READ for write-back
PC35: 70BF  RM   14, <WB>                     ; B = D[14]
PC36: 7C7F  RP   15, <WA>                     ; A = product
PC37: 72D7  RADD 14, <WM, WXY>                ; D[14] = A+B, trigger WXY (write-back read)
PC38: 10FD  RM    2, <WWF>                    ; WWF = D[2]
PC39: 086F  RM    1, <WA, WPHI>               ; A = PHI = D[1]

; === SRAM WRITE SEQUENCE (PC40-PC47) ===
PC40: 7FFB  RSP  15, <clrB> [WSP]             ; WWE - write to SRAM
PC41: 7FFB  RSP  15, <clrB> [WSP]             ; Second write
PC42: 7EFB  RSP  15, <clrB>                   ; Settling
PC43: 7EFB  RSP  15, <clrB>                   ; Settling
PC44: 7FFF  RSP  15, <-> [WSP]                ; Sync
PC45: 18BF  RM    3, <WB>                     ; B = D[3]
PC46: 0ACF  RADD  1, <WM, WPHI>               ; D[1] = A+B, PHI = A+B
PC47: 703F  RM   14, <WA, WB>                 ; A = B = D[14]

; === DELAY LINE ADVANCE (PC48-PC55) ===
PC48: 7A3F  RADD 15, <WA, WB>                 ; A = B = A+B
PC49: 7A3F  RADD 15, <WA, WB>                 ; Doubling chain
PC50: 7A3F  RADD 15, <WA, WB>                 ;
PC51: 7A3F  RADD 15, <WA, WB>                 ;
PC52: 7A3F  RADD 15, <WA, WB>                 ;
PC53: 7A3F  RADD 15, <WA, WB>                 ;
PC54: 7A3F  RADD 15, <WA, WB>                 ;
PC55: 7AF7  RADD 15, <WXY>                    ; Final SRAM read via WXY

; === FINAL WRITE-BACK (PC56-PC61) ===
PC56: 7FFB  RSP  15, <clrB> [WSP]             ; WWE
PC57: 7FFB  RSP  15, <clrB> [WSP]             ; Second write
PC58: 7EFB  RSP  15, <clrB>                   ; Settling
PC59: 7EFB  RSP  15, <clrB>                   ; Settling
PC60: 7FFF  RSP  15, <-> [WSP]                ; Sync
PC61: 587B  RM   11, <WA, clrB>               ; A = D[11], clear B (prepare next frame)
; PC62-63: Reserved
```

**Analysis:** This is the main reverb output algorithm with:
- 3 WACC instructions (PC10, PC11, PC29) = stereo DAC output
- 1 WXY+WSP at PC07 = critical MIX update from D[5] for stereo panning
- Multiple SRAM reads for delay taps
- Write-back to SRAM for feedback loop

**D-RAM usage:** Reads D[0-3,5-8,10-15], Writes D[0,1,11,13,14]

---

### A-RAM 0xC0-0xFF: 22kHz ALG 3 (D-RAM ALG=6, Used by Slot 6 - All-pass, MUTED)

**Purpose:** All-pass filter processing. This slot is MUTED (mix_l=0, mix_r=0) so it
contributes no direct audio output. However, it still processes audio data through SRAM
which other slots can read.

```
; === INITIAL SETUP (PC00-PC04) ===
PC00: 2ADF  RADD  5, <WM>                       ; D[5] = A+B (from previous slot)
PC01: 10FD  RM    2, <WWF>                      ; WWF = D[2] (waveform config)
PC02: 006F  RM    0, <WA, WPHI>                 ; A = PHI = D[0]
PC03: 18BF  RM    3, <WB>                       ; B = D[3]
PC04: 02DF  RADD  0, <WM>                       ; D[0] = A+B

; === MIX UPDATE + PROCESSING (PC05-PC11) ===
PC05: 39F7  RM    7, <WXY> [WSP]                ; ** WXY+WSP - MIX UPDATE from D[7] **
PC06: 287F  RM    5, <WA>                       ; A = D[5]
PC07: 7CBF  RP   15, <WB>                       ; B = product
PC08: 40F7  RM    8, <WXY>                      ; WXY - waveform read
PC09: 32DF  RADD  6, <WM>                       ; D[6] = A+B
PC10: 7C3F  RP   15, <WA, WB>                   ; A = B = product
PC11: 6ADE  RADD 13, <WM,WACC>                  ; D[13]=A+B, WACC - DAC output (but MUTED!)

; === SECOND PROCESSING BLOCK (PC12-PC24) ===
PC12: 006F  RM    0, <WA, WPHI>                 ; A = PHI = D[0]
PC13: 20BF  RM    4, <WB>                       ; B = D[4]
PC14: 02DF  RADD  0, <WM>                       ; D[0] = A+B
PC15: 087F  RM    1, <WA>                       ; A = D[1]
PC16: 40F7  RM    8, <WXY>                      ; WXY - waveform read
PC17: 0ADF  RADD  1, <WM>                       ; D[1] = A+B
PC18: 74DF  RP   14, <WM>                       ; D[14] = product
PC19: 38F7  RM    7, <WXY>                      ; WXY - waveform read
PC20: 307F  RM    6, <WA>                       ; A = D[6]
PC21: 7CEF  RP   15, <WPHI>                     ; PHI = product
PC22: 60FD  RM   12, <WWF>                      ; WWF = D[12]
PC23: 48F7  RM    9, <WXY>                      ; WXY - waveform read
PC24: 7FFF  RSP  15, <-> [WSP]                  ; Sync with WSP

; === THIRD PROCESSING BLOCK (PC25-PC41) ===
PC25: 7CBF  RP   15, <WB>                       ; B = product
PC26: 70EF  RM   14, <WPHI>                     ; PHI = D[14]
PC27: 48F7  RM    9, <WXY>                      ; WXY - waveform read
PC28: 325F  RADD  6, <WA, WM>                   ; D[6]=A+B, A=bus
PC29: 68BF  RM   13, <WB>                       ; B = D[13]
PC30: 7A7F  RADD 15, <WA>                       ; A = A+B
PC31: 7CBF  RP   15, <WB>                       ; B = product
PC32: 7A7F  RADD 15, <WA>                       ; A = A+B
PC33: 6ADE  RADD 13, <WM,WACC>                  ; D[13]=A+B, WACC - DAC output (but MUTED!)
PC34: 30F7  RM    6, <WXY>                      ; WXY - waveform read
PC35: 10FD  RM    2, <WWF>                      ; WWF = D[2]
PC36: 086F  RM    1, <WA, WPHI>                 ; A = PHI = D[1]

; === SETTLING SEQUENCE (PC37-PC41) ===
PC37: 7FFB  RSP  15, <clrB> [WSP]               ; Clear B, sync
PC38: 7FFB  RSP  15, <clrB> [WSP]               ; Clear B, sync
PC39: 7EFB  RSP  15, <clrB>                     ; Clear B
PC40: 7EFB  RSP  15, <clrB>                     ; Clear B
PC41: 7FFF  RSP  15, <-> [WSP]                  ; Sync

; === ACCUMULATION CHAIN (PC42-PC53) ===
PC42: 18BF  RM    3, <WB>                       ; B = D[3]
PC43: 0ADF  RADD  1, <WM>                       ; D[1] = A+B
PC44: 303F  RM    6, <WA, WB>                   ; A = B = D[6]
PC45: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B (multiply chain)
PC46: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B
PC47: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B
PC48: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B
PC49: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B
PC50: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B
PC51: 7A3F  RADD 15, <WA, WB>                   ; A = B = A+B
PC52: 08EF  RM    1, <WPHI>                     ; PHI = D[1]
PC53: 7AF7  RADD 15, <WXY>                      ; WXY - waveform read

; === FINAL SETTLING (PC54-PC63) ===
PC54: 7FFB  RSP  15, <clrB> [WSP]               ; Clear B, sync
PC55: 7FFB  RSP  15, <clrB> [WSP]               ; Clear B, sync
PC56: 7EFB  RSP  15, <clrB>                     ; Clear B
PC57: 7EFB  RSP  15, <clrB>                     ; Clear B
PC58: 7FFF  RSP  15, <-> [WSP]                  ; Sync
PC59: 687B  RM   13, <WA, clrB>                 ; A = D[13], clear B (prepare next)
PC60: 7FFF  RSP  15, <-> [WSP]                  ; Sync
PC61: 7FFF  RSP  15, <-> [WSP]                  ; Sync
PC62: 7FFF  RSP  15, <-> [WSP]                  ; Sync
PC63: 7FFF  RSP  15, <-> [WSP]                  ; Sync (extra padding)
```

**Analysis:** This algorithm is MUTED (slot 6 has mix_l=0, mix_r=0 in D-RAM word 5).
Despite having 2 WACC instructions (PC11, PC33), these produce zero output because
the MIX attenuation is maximum. The algorithm still processes audio through SRAM
for other slots to read as part of the reverb feedback network.

The WXY+WSP at PC05 updates MIX from D[7], but since slot 6 is configured as muted,
this sets mix_l=mix_r=0.

**D-RAM usage:** Reads D[0-9,12-14], Writes D[0,1,5,6,13,14]

---

### Algorithm Mappings (Summary)

| Slot | D-RAM ALG | 22kHz ALG | A-RAM Base | WACC Count | WXY+WSP | Purpose |
|------|-----------|-----------|------------|------------|---------|---------|
| 4    | 0         | 0         | 0x00       | 0          | 0       | Input conditioning |
| 5    | 2         | 1         | 0x40       | 13         | 0       | Diffusion |
| 6    | 6         | 3         | 0xC0       | 2          | 1       | All-pass (muted) |
| 7-11 | 4         | 2         | 0x80       | 3          | 1       | Delay taps + output |

## D-RAM Slot Configuration

### Slot 5 (ALG 2) - Diffusion

```python
dram_slot5 = [
    0x00080,  # word 0: PHI offset
    0x00180,  # word 1
    0x1003F,  # word 2: WWF config
    0x10000,  # word 3: Amplitude
    0x00100,  # word 4
    0x00100,  # word 5
    0x3FF00,  # word 6: Large delay address
    0x6DF00,  # word 7: Delay address
    0x5A100,  # word 8: Delay address
    0x40402,  # word 9: WWF/SRAM address
    0x40402,  # word 10: WWF/SRAM address
    0x40402,  # word 11: WWF/SRAM address
    0x40402,  # word 12: WWF/SRAM address
    0x00000,  # word 13
    0x40000,  # word 14
    0x3C280,  # word 15: IDLE=0, ALG=2
]
```

### Slot 6 (ALG 6) - All-pass Filter

```python
dram_slot6 = [
    0x5F000,  # word 0: Large SRAM address
    0x7FC80,  # word 1: Near-max address
    0x00600,  # word 2
    0x00080,  # word 3
    0x00380,  # word 4
    0x00000,  # word 5
    0x00000,  # word 6
    0x52C00,  # word 7: SRAM address
    0x2D430,  # word 8: SRAM address (comb delay)
    0x00400,  # word 9
    0x7FC00,  # word 10
    0x00000,  # word 11
    0x34000,  # word 12: WWF config
    0x00000,  # word 13
    0x00000,  # word 14
    0x00680,  # word 15: IDLE=0, ALG=6
]
```

### Slots 7-11 (ALG 4) - Delay Taps

Five instances of Algorithm 4 with different delay addresses:

| Slot | word 0 (SRAM addr) | word 5 | word 6 | Description |
|------|-------------------|--------|--------|-------------|
| 7    | 0x2B800           | 0x6667F| 0x79999| Early tap 1 |
| 8    | 0x23500           | 0x66A7F| 0x79A9F| Early tap 2 |
| 9    | 0x1AE00           | 0x66EBF| 0x79BA5| Early tap 3 |
| 10   | 0x0F700           | 0x674BF| 0x79D2F| Early tap 4 |
| 11   | 0x08C00           | 0x677FF| 0x79DF3| Early tap 5 |

## Reverb Architecture Analysis

The FX configuration implements a **multi-tap reverb** with:

1. **Input processing** (Slot 4, ALG 0): Initial signal conditioning
2. **Diffusion network** (Slot 5, ALG 2): Spreads the early reflections
3. **All-pass filter** (Slot 6, ALG 6): Creates diffuse late reverberation
4. **5 delay taps** (Slots 7-11, ALG 4): Early reflections at different times

### SRAM Usage

The external SRAM appears to be used for delay line storage:
- Large addresses (0x5F000, 0x6DF00, etc.) suggest substantial delay buffer
- Multiple tap points reading from different SRAM locations
- WWF fields contain SRAM bank/address configurations

### Signal Flow (Revised)

```
Input -> Slot 4 (ALG 0, conditioning) -> SRAM write
                |
                v
         Slot 5 (ALG 1, diffusion) -----> DAC output (heavy WACC)
                |
                v
         Slot 6 (ALG 3, all-pass) -> SRAM buffers only (MUTED)
                |
                +---> Slots 7-11 (ALG 2, delay taps) -> DAC output
                      - 5 parallel taps at different delays
                      - Stereo mix: 7,8,11 center; 9,10 right-biased
```

**Audio output sources:**
1. Slot 5 (diffusion) - primary reverb output
2. Slots 7-11 (delay taps) - early reflections

**Internal processing only:**
- Slot 4 - input conditioning
- Slot 6 - all-pass diffusion (muted, feeds other slots via SRAM)

## Fixed Issues (2026-01-14)

### FX Input Sign Extension Bug

**Problem:** FX reads input samples with 8-bit resolution (via 74HC595 shift registers).
The sign extension logic `WDH11 = (~WAH0) AND WDH10` suggested asymmetric handling
where RIGHT channel (WAH0=1) would have bit 11 forced to 0.

**Impact:** This would turn negative right-channel samples positive, breaking audio.

**Solution:** Apply symmetric sign extension for both channels when reading input samples.
The hardware quirk may exist but the algorithm uses both even and odd PHI addresses,
so proper sign extension is required for correct audio.

**Code change:** In `sam_fx_waveform_r()`, always sign-extend input samples:
```cpp
// Sign extend bit 10 to bit 11 (for proper signed audio)
if (result & 0x400)
    result |= 0x800;
```

**Result:** FX now produces oscillating audio output instead of accumulating DC offset.

## Open Questions

- [ ] Exact SRAM address mapping (bits 18-10 of WWF?)
- [ ] Feedback path routing between slots
- [x] ~~How do idle slots (0-3, 12-15) contribute to SRAM buffer management?~~ They don't - truly inactive
- [ ] Reverb time/decay control mechanism
- [ ] Actual L/R channel handling - SND outputs L=0, R=audio; FX reads both via PHI[0]

## Testing

### Test MIDI File with Reverb

To test FX reverb with a MIDI file that enables reverb mode:

```bash
./mamemuse keyfox10 -rompath /home/jeff/bastel/roms/hohner \
  -midiin sam8905/mid/test_piano.mid \
  -seconds_to_run 4 \
  -wavwrite /tmp/fx_test_piano.wav
```

The `test_piano.mid` file contains:
- CC80 = 1 (reverb mode on)
- CC91 = 64 (reverb level)
- Note C4 velocity 100

Check output with:
```bash
sox /tmp/fx_test_piano.wav -n stat
```

**HACK applied:** FX output scaled down by 16 (>> 4) to prevent clipping.

The issue is that FX algorithms use multiple WACC instructions per slot:
- Slot 5 (ALG 1): 12 WACC instructions
- Slots 7-11 (ALG 2): 3 WACC each = 15 total
- Total: 27 WACC per frame, all at mix=7 (0dB)

This causes accumulated output to exceed 16-bit DAC range (±32K). The >> 4 scaling
brings output within range. Real hardware may have implicit output attenuation.

## Test Harness Analysis (2026-01-14)

### SRAM Write/Read Patterns

Ran `fx_test_harness` with 440 Hz sine input for 22050 frames (1 second):

- **SRAM usage:** 235 locations in range 0x0200-0x02FF (256-byte buffer)
- **Total writes:** 529,200 (24 per frame = 4 WWE × 6 active slots)
- **Total reads:** 1,830,152 (83 per frame from waveform lookups)

### Gain Structure Issue

All 5 delay tap slots (7-11) have MIX=7 (0dB = full volume):
- Each slot contributes full multiplication result to output
- Sum of 5 slots = +14dB gain before any signal processing
- Combined with multiple WACCs per slot = severe clipping

**D[5] values for slots 7-11:**
| Slot | D[5] Value | Y coefficient | mix_l | mix_r |
|------|------------|---------------|-------|-------|
| 7    | 0x6667F    | 0xCCC (3276)  | 7     | 7     |
| 8    | 0x66A7F    | 0xCD4 (3284)  | 7     | 7     |
| 9    | 0x66EBF    | 0xCDD (3293)  | 7     | 7     |
| 10   | 0x674BF    | 0xCE9 (3305)  | 7     | 7     |
| 11   | 0x677FF    | 0xCEF (3311)  | 7     | 7     |

All at 0dB causes constant clipping without the >> 4 output hack.

### WAV Output Analysis

With 440 Hz sine input (amplitude ±16000):
- **Input:** Mean norm 0.31, RMS 0.35
- **Output (saturated):** Constant ±32767 clip, unusable
- **Output (with >> 4 hack):** Would be reasonable reverb

### Algorithm Dynamics

ALG 0 modifies D[11] dynamically:
- Initial D[11] = 0x0007C
- PC03 writes D[11] ← 0x0007C (A+B from RADD)
- PC08 writes D[11] ← 0x0027C (A+B with A=0x200)
- Result: Y = (0x0027C >> 7) & 0xFFF = 4 (tiny coefficient)

The algorithm computes feedback coefficients at runtime, not statically loaded.

### Files Generated

- `fx_input.wav` - Input sine wave
- `fx_output_l.wav`, `fx_output_r.wav` - FX output (clipped)
- `fx_sram_dump.wav` - SRAM buffer contents
- `fx_sram_writes.wav` - All data written to SRAM
- `fx_sram_reads.wav` - All data read from SRAM

## Files

- `sam8905/sam8905_aram_decoder.py` - Instruction decoder
- `sam8905/WIP_fx_reverb_analysis.md` - This file
- `sam8905/sam8905_programmers_guide.md` - SAM8905 documentation
- `sam8905/mid/test_piano.mid` - Test MIDI with reverb enabled
- `sam8905/fx_test_harness.cpp` - Standalone FX test harness

## Tasks

- [x] Extract A-RAM and D-RAM data from emulator
- [x] Decode all used algorithms (0, 2, 4, 6)
- [x] Document slot configurations
- [x] Analyze WACC output usage per slot
- [x] Analyze MIXL/MIXR stereo panning settings
- [x] Understand idle slot behavior (they do nothing)
- [x] Analyze SRAM addressing scheme (0x0200-0x02FF, 256-byte buffer)
- [x] Trace signal flow through test harness
- [x] Identify feedback coefficients (dynamic computation via RADD)
- [ ] Investigate why real hardware doesn't clip (different MIX values? output stage?)
