# Keyfox10 FX Chip - Reverb Algorithm Analysis

## Status: IN PROGRESS

Started: 2026-01-13

## Overview

The Keyfox10 uses a dedicated SAM8905 chip ("SAM[FX]") for reverb/effects processing. This document analyzes the algorithms and D-RAM configuration programmed by the firmware.

## Active Configuration

**Active slots:** 4, 5, 6, 7, 8, 9, 10, 11 (8 slots)
**Active algorithms:** 0, 2, 4, 6 (4 algorithms)

| Slot | IDLE | ALG | Description |
|------|------|-----|-------------|
| 0-3  | 1    | -   | Idle (likely SRAM tap buffer writes) |
| 4    | 0    | 0   | Active - Algorithm 0 |
| 5    | 0    | 2   | Active - Algorithm 2 |
| 6    | 0    | 6   | Active - Algorithm 6 |
| 7-11 | 0    | 4   | Active - Algorithm 4 (5 instances!) |
| 12-15| 1    | -   | Idle (likely SRAM tap buffer writes) |

## A-RAM Algorithms

### Algorithm 0 (Used by Slot 4)

```
PC00: 0000  RM 0, <WA, WB, WM, WPHI, WXY, clearB, WWF, WACC>  ; Full init from D[0]
PC01: 607F  RM 12, <WA>                                       ; A = D[12]
PC02: 58BF  RM 11, <WB>                                       ; B = D[11]
PC03: 5A5F  RADD 11, <WA, WM>                                 ; D[11] = A+B, A = A+B
PC04: 30BF  RM 6, <WB>                                        ; B = D[6]
PC05: 5DDF  RP 11, <WM, WSP> ***                              ; D[11] = product (special)
PC06: 082D  RM 1, <WA, WB, WPHI, WWF>                         ; PHI/WF from D[1]
PC07: 593F  RM 11, <WA, WB, WSP> ***                          ; A=B=D[11] (special)
PC08: 5ADF  RADD 11, <WM>                                     ; D[11] = A+B
PC09: 58F7  RM 11, <WXY>                                      ; XY = D[11]
PC10: 406F  RM 8, <WA, WPHI>                                  ; A=PHI=D[8]
PC11: 2CDF  RP 5, <WM>                                        ; D[5] = product
PC12: 48BF  RM 9, <WB>                                        ; B = D[9]
PC13: 58F7  RM 11, <WXY>                                      ; XY = D[11]
PC14: 42DF  RADD 8, <WM>                                      ; D[8] = A+B
PC15: 749F  RP 14, <WB, WM>                                   ; D[14] = product, B = product
PC16: 68F7  RM 13, <WXY>                                      ; XY = D[13]
PC17: 38FD  RM 7, <WWF>                                       ; WWF = D[7]
PC18: 7FFB  RSP, <clearB, WSP> ***                            ; Clear B
PC19: 7FFB  RSP, <clearB, WSP> ***                            ; Clear B
PC20: 7EFB  RSP, <clearB>                                     ; Clear B
PC21: 7EFB  RSP, <clearB>                                     ; Clear B
PC22: 7FFF  RSP, <WSP> ***                                    ; NOP with WSP
PC23: 406F  RM 8, <WA, WPHI>                                  ; A=PHI=D[8]
PC24: 50BF  RM 10, <WB>                                       ; B = D[10]
PC25: 42DF  RADD 8, <WM>                                      ; D[8] = A+B
PC26: 683F  RM 13, <WA, WB>                                   ; A=B=D[13]
PC27-31: 7A3F  RADD, <WA, WB>                                 ; A=B=A+B (repeated)
```

**D-RAM usage:** Reads D[0,1,6,7,8,9,10,11,12,13], Writes D[0,5,8,11,14]

### Algorithm 2 (Used by Slot 5)

```
PC00: 30EF  RM 6, <WPHI>                                      ; PHI = D[6]
PC01: 48FD  RM 9, <WWF>                                       ; WWF = D[9]
PC02: 6ADF  RADD 13, <WM>                                     ; D[13] = A+B
PC03: 703F  RM 14, <WA, WB>                                   ; A=B=D[14]
PC04: 0000  RM 0, <WA, WB, WM, WPHI, WXY, clearB, WWF, WACC>  ; Full init
PC05: 6BDF  RADD 13, <WM, WSP> ***                            ; D[13] = A+B (special)
PC06: 38EF  RM 7, <WPHI>                                      ; PHI = D[7]
PC07: 50FC  RM 10, <WWF, WACC>                                ; WWF=D[10], accumulate
PC08: 687F  RM 13, <WA>                                       ; A = D[13]
PC09: 7CBE  RP, <WB, WACC>                                    ; B = product, accumulate
PC10: 18F7  RM 3, <WXY>                                       ; XY = D[3]
PC11: 7A7F  RADD, <WA>                                        ; A = A+B
PC12: 40EF  RM 8, <WPHI>                                      ; PHI = D[8]
PC13: 58FC  RM 11, <WWF, WACC>                                ; WWF=D[11], accumulate
PC14: 7CBE  RP, <WB, WACC>                                    ; B = product, accumulate
PC15: 6ADF  RADD 13, <WM>                                     ; D[13] = A+B
PC16: 18F7  RM 3, <WXY>                                       ; XY = D[3]
PC17: 00BF  RM 0, <WB>                                        ; B = D[0]
PC18: 307F  RM 6, <WA>                                        ; A = D[6]
PC19: 32CE  RADD 6, <WM, WPHI, WACC>                          ; D[6] = A+B, PHI=A+B, acc
PC20: 48FC  RM 9, <WWF, WACC>                                 ; WWF=D[9], accumulate
PC21: 387F  RM 7, <WA>                                        ; A = D[7]
PC22: 3ADF  RADD 7, <WM>                                      ; D[7] = A+B
PC23: 407F  RM 8, <WA>                                        ; A = D[8]
PC24: 42DF  RADD 8, <WM>                                      ; D[8] = A+B
PC25: 7CBF  RP, <WB>                                          ; B = product
PC26: 20F7  RM 4, <WXY>                                       ; XY = D[4]
PC27: 687F  RM 13, <WA>                                       ; A = D[13]
PC28: 38EF  RM 7, <WPHI>                                      ; PHI = D[7]
PC29: 50FD  RM 10, <WWF>                                      ; WWF = D[10]
PC30: 7A7E  RADD, <WA, WACC>                                  ; A = A+B, accumulate
PC31: 7CBE  RP, <WB, WACC>                                    ; B = product, accumulate
```

**D-RAM usage:** Reads D[0,3,4,6,7,8,9,10,11,13,14], Writes D[0,6,7,8,13]

### Algorithm 4 (Used by Slots 7-11)

```
PC00: 5ADF  RADD 11, <WM>                                     ; D[11] = A+B
PC01: 68BF  RM 13, <WB>                                       ; B = D[13]
PC02: 72DF  RADD 14, <WM>                                     ; D[14] = A+B
PC03: 10FD  RM 2, <WWF>                                       ; WWF = D[2]
PC04: 006F  RM 0, <WA, WPHI>                                  ; A=PHI=D[0]
PC05: 18BF  RM 3, <WB>                                        ; B = D[3]
PC06: 02DF  RADD 0, <WM>                                      ; D[0] = A+B
PC07: 29B7  RM 5, <WB, WXY, WSP> ***                          ; B=XY=D[5], mix update
PC08: 707F  RM 14, <WA>                                       ; A = D[14]
PC09: 7CBF  RP, <WB>                                          ; B = product
PC10: 7A7E  RADD, <WA, WACC>                                  ; A = A+B, accumulate
PC11: 72DE  RADD 14, <WM, WACC>                               ; D[14] = A+B, accumulate
PC12: 30F7  RM 6, <WXY>                                       ; XY = D[6]
PC13: 20BF  RM 4, <WB>                                        ; B = D[4]
PC14: 6CDF  RP 13, <WM>                                       ; D[13] = product
PC15: 006F  RM 0, <WA, WPHI>                                  ; A=PHI=D[0]
PC16: 02DF  RADD 0, <WM>                                      ; D[0] = A+B
PC17: 087F  RM 1, <WA>                                        ; A = D[1]
PC18: 28F7  RM 5, <WXY>                                       ; XY = D[5]
PC19: 0ADF  RADD 1, <WM>                                      ; D[1] = A+B
PC20: 7CEF  RP, <WPHI>                                        ; PHI = product
PC21: 78FD  RM 15, <WWF>                                      ; WWF = D[15]
PC22: 40F7  RM 8, <WXY>                                       ; XY = D[8]
PC23: 687F  RM 13, <WA>                                       ; A = D[13]
PC24: 7CBF  RP, <WB>                                          ; B = product
PC25: 6ADF  RADD 13, <WM>                                     ; D[13] = A+B
PC26: 38F7  RM 7, <WXY>                                       ; XY = D[7]
PC27: 707F  RM 14, <WA>                                       ; A = D[14]
PC28: 7CBF  RP, <WB>                                          ; B = product
PC29: 725E  RADD 14, <WA, WM, WACC>                           ; D[14] = A+B, A=A+B, acc
PC30: 50BF  RM 10, <WB>                                       ; B = D[10]
PC31: 797B  RM 15, <WA, clearB, WSP> ***                      ; A=D[15], clearB, special
```

**D-RAM usage:** Reads D[0,1,2,3,4,5,6,7,8,10,13,14,15], Writes D[0,1,11,13,14]

### Algorithm 6 (Used by Slot 6)

```
PC00: 2ADF  RADD 5, <WM>                                      ; D[5] = A+B
PC01: 10FD  RM 2, <WWF>                                       ; WWF = D[2]
PC02: 006F  RM 0, <WA, WPHI>                                  ; A=PHI=D[0]
PC03: 18BF  RM 3, <WB>                                        ; B = D[3]
PC04: 02DF  RADD 0, <WM>                                      ; D[0] = A+B
PC05: 39F7  RM 7, <WXY, WSP> ***                              ; XY=D[7], mix update
PC06: 287F  RM 5, <WA>                                        ; A = D[5]
PC07: 7CBF  RP, <WB>                                          ; B = product
PC08: 40F7  RM 8, <WXY>                                       ; XY = D[8]
PC09: 32DF  RADD 6, <WM>                                      ; D[6] = A+B
PC10: 7C3F  RP, <WA, WB>                                      ; A=B=product
PC11: 6ADE  RADD 13, <WM, WACC>                               ; D[13] = A+B, accumulate
PC12: 006F  RM 0, <WA, WPHI>                                  ; A=PHI=D[0]
PC13: 20BF  RM 4, <WB>                                        ; B = D[4]
PC14: 02DF  RADD 0, <WM>                                      ; D[0] = A+B
PC15: 087F  RM 1, <WA>                                        ; A = D[1]
PC16: 40F7  RM 8, <WXY>                                       ; XY = D[8]
PC17: 0ADF  RADD 1, <WM>                                      ; D[1] = A+B
PC18: 74DF  RP 14, <WM>                                       ; D[14] = product
PC19: 38F7  RM 7, <WXY>                                       ; XY = D[7]
PC20: 307F  RM 6, <WA>                                        ; A = D[6]
PC21: 7CEF  RP, <WPHI>                                        ; PHI = product
PC22: 60FD  RM 12, <WWF>                                      ; WWF = D[12]
PC23: 48F7  RM 9, <WXY>                                       ; XY = D[9]
PC24: 7FFF  RSP, <WSP> ***                                    ; NOP
PC25: 7CBF  RP, <WB>                                          ; B = product
PC26: 70EF  RM 14, <WPHI>                                     ; PHI = D[14]
PC27: 48F7  RM 9, <WXY>                                       ; XY = D[9]
PC28: 325F  RADD 6, <WA, WM>                                  ; D[6] = A+B, A=A+B
PC29: 68BF  RM 13, <WB>                                       ; B = D[13]
PC30: 7A7F  RADD, <WA>                                        ; A = A+B
PC31: 7CBF  RP, <WB>                                          ; B = product
```

**D-RAM usage:** Reads D[0,1,2,3,4,5,7,8,9,12,13,14], Writes D[0,1,5,6,13,14]

## D-RAM Slot Configuration

### Slot 4 (ALG 0) - Initial Processing

```python
dram_slot4 = [
    0x00000,  # word 0: PHI
    0x50080,  # word 1: DPHI/config
    0x00400,  # word 2: WWF config
    0x40000,  # word 3: Config
    0x00080,  # word 4: Config
    0x00000,  # word 5: Accumulator
    0x7FFFF,  # word 6: Max amplitude
    0x40402,  # word 7: WWF/address
    0x00100,  # word 8: PHI increment
    0x00080,  # word 9: Config
    0x00180,  # word 10: Config
    0x0007C,  # word 11: Feedback coef
    0x00000,  # word 12
    0x00000,  # word 13
    0x00000,  # word 14
    0x34080,  # word 15: IDLE=0, ALG=0
]
```

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

### Signal Flow (Hypothesis)

```
Input -> Slot 4 (conditioning) -> SRAM write
                                     |
         +---------------------------+
         |
         v
    Slot 5 (diffusion) <- SRAM read
         |
         v
    Slot 6 (all-pass) <- SRAM read/write
         |
         +-> Slots 7-11 (parallel delay taps) -> Mix to output
```

## Open Questions

- [ ] Exact SRAM address mapping (bits 18-10 of WWF?)
- [ ] Feedback path routing between slots
- [ ] How do idle slots (0-3, 12-15) contribute to SRAM buffer management?
- [ ] Reverb time/decay control mechanism

## Files

- `sam8905/sam8905_aram_decoder.py` - Instruction decoder
- `sam8905/WIP_fx_reverb_analysis.md` - This file
- `sam8905/sam8905_programmers_guide.md` - SAM8905 documentation

## Tasks

- [x] Extract A-RAM and D-RAM data from emulator
- [x] Decode all used algorithms (0, 2, 4, 6)
- [x] Document slot configurations
- [ ] Analyze SRAM addressing scheme
- [ ] Trace signal flow through all slots
- [ ] Identify feedback coefficients
- [ ] Understand idle slot buffer write patterns
