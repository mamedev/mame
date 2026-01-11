# SAM8905 Algorithm 2 Analysis (Piano)

## Overview

This document analyzes Algorithm 2 as used by the Keyfox10 electric piano program (program 2).
The goal is to understand why note-off doesn't cause amplitude decay for the piano sound.

## ⚠️ IMPORTANT: SAM IRQ IS NOT CONNECTED ⚠️

**DO NOT investigate SAM interrupt/IRQ mechanisms as a potential fix.**

The SAM8905 emulation has NO IRQ output callback connected to the CPU. The `m_interrupt_latch`
is set internally but there is no `irq_handler` callback or `set_input_line()` to actually
interrupt the CPU. The firmware reads the latch via polling but discards the value.

This is a KNOWN state - do not re-investigate IRQ connection as a solution.

## ⚠️ IMPORTANT: MIDI NOTE-OFF WORKS FOR OTHER ALGORITHMS ⚠️

**DO NOT investigate MIDI note-off handling as a potential issue.**

MIDI note-off works correctly for other algorithms (e.g., organ/ALG=1 sets idle bit on note-off).
The issue is specific to Algorithm 2 (piano) envelope/decay handling, NOT the MIDI layer.

This is a KNOWN fact - do not re-investigate MIDI note-off as the root cause.

## Key Findings

1. **Slot 2** produces the piano sound using **Algorithm 2**
2. **Interrupts are masked** (M=1 in param15=0x3429D)
3. CPU manages envelope entirely through **direct D-RAM writes**
4. The algorithm's WM WSP mechanism is working correctly
5. **Root cause**: CPU doesn't write decay values after note-off
6. **SAM IRQ line is unconnected** - latch is set but no CPU interrupt fires
7. **External waveform playback FIXED** - see WIP_waveform_memory_analysis.md
   - Correct mapping: `(WAVE[6:0] << 10) | PHI[11:2]` = 1024 samples/wave
   - Piano sound now plays with correct pitch and loop behavior
8. **Carry persistence FIXED** - slot.carry now persists between cycles
   - WAVE now properly increments when PHI overflows
   - IRQ latch is set when WAVE reaches finalWAVE with E=1
   - But piano algorithm doesn't zero amplitude - expects CPU to handle decay

## Algorithm 2 Decoded Microcode

```
PC00: RM 15, <WXY, WSP>      ; Load mix from word15, update output attenuators
PC01: RM 4, <WA>             ; A = word4 (wave format)
PC02: RM 5, <WB>             ; B = word5
PC03: RADD, <WWF>            ; WF = A+B (compute waveform address)
PC04: RM 1, <WA, WB, WPHI>   ; A = B = PHI = word1 (phase accumulator)
PC05: RADD, <WA, WB>         ; A = B = 2*PHI (doubled phase)
PC06: RADD 9, <WM>           ; word9 = A+B (intermediate phase)
PC07: RM 7, <WA, clearB>     ; A = word7 (delta), B = 0
PC08: RADD 8, <WM>           ; word8 = A+B = word7 (copy delta)
PC09: RSP, <WSP>             ; NOP
PC10: RM 2, <WXY>            ; X = waveform sample, Y = word2 (amplitude)
PC11: RM 9, <WPHI>           ; PHI = word9
PC12: RSP, <clearB>          ; B = 0
PC13: RM 9, <WA>             ; A = word9
PC14: RP, <WXY>              ; X = waveform, Y = mul_result (chained multiply)
PC15: RM 4, <WWF>            ; WF = word4 (wave select)
PC16: RM 1, <WA, WPHI>       ; A = PHI = word1
PC17: RM 0, <WB, WACC>       ; B = word0 (DPHI), accumulate output
PC18: RADD 1, <WM>           ; word1 = word1 + word0 (update phase)
PC19: RM 4, <WA, WB, WSP>    ; WA WSP: check wave cycle end
PC20: RADD 4, <WM, clearB>   ; word4 = A+B (update wave counter), B = 0
PC21: RADD 8, <WM, WSP>      ; Conditional write to word8
PC22: RM 2, <WB, WXY>        ; B = word2 (amplitude), prepare multiply
PC23: RM 3, <WA>             ; A = word3 (delta), sets CLEARRQST=1
PC24: RADD 2, <WM, WSP>      ; Conditional write word2 = word2 + word3
PC25: RM 6, <WA, WACC>       ; A = word6, accumulate
PC26: RM 8, <WB>             ; B = word8
PC27: RADD, <WB>             ; B = A+B
PC28: RM 4, <WA>             ; A = word4
PC29: RADD 4, <WM>           ; word4 = A+B (update wave)
PC30: RSP, <WSP>             ; Reserved
PC31: RSP, <WSP>             ; Reserved
```

## D-RAM Word Assignments for Slot 2

| Word | Purpose | Initial Value | Notes |
|------|---------|---------------|-------|
| 0    | DPHI (pitch increment) | 0x000C7 | Small positive value |
| 1    | PHI (phase accumulator) | varies | Updated by algorithm |
| 2    | Amplitude | 0x65800 → 0x6E040 | CPU writes directly for attack |
| 3    | Amplitude delta | 0 | Never set to negative for decay |
| 4    | Wave format (E\|WAVE\|finalWAVE) | 0x4E072 | E=1, WAVE=112, final=114 |
| 5    | Wave offset | 0x00600 | |
| 6    | Extra amplitude | 0x00000 | Read-only input |
| 7    | Delta copy source | 0x00000 | |
| 8    | Intermediate storage | varies | |
| 9    | Secondary phase | varies | |
| 15   | Config (I\|ALG\|M) | 0x3429D | I=0, ALG=2, M=1 (masked) |

## Envelope Mechanism

### Attack Phase (Working)
1. Note-on triggers CPU at PC=0x84EF to write word2 = 0x65800
2. CPU at PC=0x744F periodically increases word2 (0x65800 → 0x6E040)
3. Algorithm uses word2 as amplitude multiplier at PC10 and PC22

### Sustain Phase (Working)
1. CPU stops updating word2 when it reaches target (0x6E040)
2. Sound plays at constant amplitude

### Release Phase (NOT WORKING)
1. Note-off should trigger decay
2. **Problem**: CPU writes word3 = 0 (no delta)
3. Algorithm at PC24 writes word2 = word2 + word3 = word2 (unchanged)
4. Without negative delta, amplitude never decreases

## WM WSP Verification

The algorithm's conditional write mechanism is working correctly:

```
WM WSP slot2 word2: clrq=1 carry=0 int_mod=0 A=00000 B=65800 -> write=1 bus=65800
```

- CLEARRQST=1 (set by PC23's plain WA)
- CARRY=0 (no overflow because word3=0 is small)
- write=1 (write enabled per truth table)
- A=0 (word3 delta is zero)
- Result: word2 = 0 + 0x65800 = unchanged

## Interrupt Path (Not Used for Slot 2)

Slot 2 has M=1 (interrupt mask set), so even when INTMOD=1, no interrupts fire.
The firmware chose to manage piano envelope entirely via CPU polling/timing.

## Comparison with Organ (Algorithm 5)

The organ (program 0) works correctly for note-off because:
- Organ uses algorithm 5 with different envelope handling
- The organ's note-off mechanism likely uses direct CPU writes to set idle bit
- Or uses a simpler envelope that just stops (no decay needed)

## Root Cause Analysis

The issue is **NOT in the SAM emulation**. The SAM is working correctly:
- WA WSP correctly computes CLEARRQST and INTMOD
- WM WSP correctly enables/disables writes based on truth table
- Carry calculation is correct

The issue is in the **CPU-SAM communication**:
1. After note-off, CPU should either:
   - Write a negative delta to word3 for gradual decay
   - Or directly write decreasing values to word2
   - Or set the I (idle) bit in word15
2. Currently, CPU just writes 0 to word3, leaving amplitude unchanged

## Possible Causes

1. **CPU interrupt handling**: The CPU might rely on some signal that's not being generated
2. **Timer/polling mechanism**: The CPU might use a timer that's not properly emulated
3. **Note-off message processing**: The MIDI note-off handler might not properly trigger release

## 2025-01-07 Investigation Update

### Interrupt Mechanism Analysis

Fixed interrupt generation per WM WSP truth table:
- Old code: `if (slot.int_mod && !int_masked)`
- Fixed: `bool request_irq = slot.clear_rqst && (slot.int_mod || carry);`

Result: IRQ count dropped to 0. This is correct behavior because:
1. When int_mod=1, clrq=0 (from WA WSP truth table)
2. carry=0 for envelope updates (word3=0, no overflow)
3. The only way to get clrq=1 AND int_mod=1 is when carry=1 AND wf_match=true

### Algorithm 2 WM WSP Sequences

Algorithm 2 has TWO separate WA/WM WSP sequences:

**Sequence 1 (wave cycle check):**
- PC19: `RM 4, <WA, WB, WSP>` - WA WSP sets INTMOD=1
- PC21: `RADD 8, <WM, WSP>` - WM WSP uses INTMOD=1

**Sequence 2 (amplitude update):**
- PC23: `RM 3, <WA>` - Plain WA sets CLEARRQST=1, **clears INTMOD=0**
- PC24: `RADD 2, <WM, WSP>` - WM WSP uses INTMOD=0

Per Section 8-3: "WA (without WSP) always sets CLEARRQST and clears INTMOD"

### WM WSP IRQ Behavior for Amplitude Update

With plain WA → WM WSP (INTMOD=0, CLEARRQST=1):
| CLEARRQST | CARRY | INTMOD | write | IRQ |
|-----------|-------|--------|-------|-----|
| yes       | 0     | 0      | yes   | no  |
| yes       | 1     | X      | no    | yes |

IRQ fires only when **CARRY=1**. With word3=0:
- B positive → carry = bit 19 overflow = 0
- No IRQ, write happens (amplitude unchanged)

For decay IRQ to fire, **word3 must be NEGATIVE**:
- B negative → carry = complement of result sign bit
- While amplitude positive → CARRY=1 → IRQ fires
- When amplitude crosses zero → CARRY=0 → final write

### Key Difference: Organ vs Piano

| Parameter | Organ (slot 0,1) | Piano (slot 2) |
|-----------|------------------|----------------|
| param15   | 0x00101          | 0x3429D        |
| ALG       | 1                | 2              |
| M (mask)  | **0 (not masked)**| **1 (masked)** |
| I (idle)  | 0                | 0              |

**Organ note-off**: CPU sets idle bit (param15 → 0x00800)
**Piano note-off**: CPU should write negative delta to word3, but writes 0

### Why Piano Doesn't Decay

The piano has M=1 (interrupts masked), so even if the algorithm generated IRQs, the CPU wouldn't be notified. The firmware is supposed to manage piano envelope entirely via direct D-RAM writes:
1. At note-on: CPU writes initial amplitude to word2
2. During attack: CPU increases word2 periodically
3. At note-off: CPU should write negative delta to word3
4. **BUG**: CPU writes 0 to word3 instead of negative value

### Hypothesis

The firmware might have a bug where piano release phase isn't properly implemented. Or there might be a timer/polling mechanism that triggers the release that we haven't found yet.

## Conclusion

**SAM emulation is correct.** The implementation matches the manual:
- Plain WA sets CLEARRQST=1 and clears INTMOD=0
- WM WSP generates IRQ only when CLEARRQST=1 AND (INTMOD=1 OR CARRY=1)
- With word3=0, CARRY=0, so no IRQ fires for amplitude updates

**The issue is firmware/CPU-side:**
- Piano has M=1 (IRQ masked), so even if IRQs fire, CPU won't be notified
- Firmware must intend CPU-managed envelope (polling/timers)
- CPU isn't writing negative delta to word3 for release

## 2026-01-08 Update: Full Algorithm 2 Microcode Dump

### Actual A-RAM Values (from trace)

```
ALG2[00]: 0x79F7    ALG2[10]: 0x10F7    ALG2[20]: 0x22DB
ALG2[01]: 0x207F    ALG2[11]: 0x48EF    ALG2[21]: 0x43DF
ALG2[02]: 0x28BF    ALG2[12]: 0x7EFB    ALG2[22]: 0x10B7
ALG2[03]: 0x7AFD    ALG2[13]: 0x487F    ALG2[23]: 0x187F
ALG2[04]: 0x082F    ALG2[14]: 0x7CF7    ALG2[24]: 0x13DF
ALG2[05]: 0x7A3F    ALG2[15]: 0x20FD    ALG2[25]: 0x307E
ALG2[06]: 0x4ADF    ALG2[16]: 0x086F    ALG2[26]: 0x40BF
ALG2[07]: 0x387B    ALG2[17]: 0x00BE    ALG2[27]: 0x7ABF
ALG2[08]: 0x42DF    ALG2[18]: 0x0ADF    ALG2[28]: 0x207F
ALG2[09]: 0x7FFF    ALG2[19]: 0x213F    ALG2[29]: 0x22DF
```

### Key Instruction Decode

**ALG2[22]: 0x10B7** - Load amplitude to B
- MAD=2 (word 2), Emit=RM (read memory)
- WB active (bit 6=0) → B = word2 (amplitude)

**ALG2[23]: 0x187F** - Load delta to A (normal WA)
- MAD=3 (word 3), Emit=RM (read memory)
- WA active (bit 7=0), WSP=0 (bit 8=0)
- This is **normal WA**, NOT WA WSP
- Sets CLEARRQST=1, clears INTMOD=0

**ALG2[24]: 0x13DF** - WM WSP amplitude update
- MAD=2 (word 2), Emit=RADD (A+B)
- WSP=1 (bit 8=1), WM active (bit 5=0)
- Writes word2 = A + B (delta + amplitude)

### Critical Finding: Normal WA Clobbers Flags

The envelope update sequence is:
1. PC22: WB loads amplitude from word 2 → B = 0x65800
2. PC23: **Normal WA** loads delta from word 3 → A = 0x00000
   - Sets CLEARRQST=1, INTMOD=0 (per Section 8-3)
3. PC24: WM WSP writes A+B to word 2
   - With CLEARRQST=1, INTMOD=0, CARRY=0
   - Write enabled, no interrupt generated

The algorithm uses **plain WA** (not WA WSP) to load delta, which always sets:
- CLEARRQST = 1 (enables writes)
- INTMOD = 0 (disables interrupt unless CARRY=1)

### Firmware Behavior Trace

**Initial state (note-on):**
```
CPU D-RAM[2F] = 0x00000 (reset)
CPU D-RAM[2F] = 0x00800 (idle bit set)
CPU D-RAM[2F] = 0x3429D (ALG=2, M=1, active)
CPU D-RAM[22] = 0x65800 (initial amplitude)
```

**Attack phase (delta ramp-down):**
```
CPU D-RAM[23] = 0x0000A (delta)
CPU D-RAM[23] = 0x0000B (delta)
CPU D-RAM[23] = 0x00009 (delta)
... (decreases to 0)
CPU D-RAM[23] = 0x00000 (attack complete)
```

**Sustain phase (firmware writes amplitude directly):**
```
CPU D-RAM[22] = 0x6D300
CPU D-RAM[22] = 0x6D440
CPU D-RAM[22] = 0x6D540
... (increases)
CPU D-RAM[22] = 0x6E040
```

**Note-off (no change!):**
- param15 stays at 0x3429D (idle bit never set)
- word3 stays at 0x00000 (no negative delta)
- word2 continues increasing

### Root Cause Confirmed

**The firmware NEVER processes note-off for piano.** Evidence:
1. param15 stays at 0x3429D (slot never idled)
2. Delta (word 3) is NEVER set to negative
3. Amplitude keeps increasing even after note-off should occur

### Possible Firmware Bugs

1. **Note-off handler missing**: Piano release code path may not exist
2. **Slot state not updated**: FUN_CODE_6d25_SAM_UPDATE checks state bits 3-5 for 0x10 to trigger decay, but this state is never set
3. **MIDI routing issue**: Note-off may not reach the piano slot handler

## Next Steps

1. ~~Compare organ note-off handling to understand what works~~ DONE - organ uses idle bit
2. ~~Trace CPU code at note-off time~~ DONE - param15 never changes
3. Investigate why slot state bits 3-5 never reach 0x10 (decay state)
4. Check MIDI note-off routing in firmware (is it reaching the right handler?)
5. Search firmware for piano-specific release code path
6. Consider if this is a known limitation of the original keyboard

## 2026-01-08 Update: Decay State Mechanism Discovery

### How State 0x10 (Decay) Works

The decay state 0x10 is **not** set directly by note-off handling. Instead:

1. **Program Data Stream**: Slot state comes from program data (external ROM/RAM at `SAM_INTMEM_37_prg_ptr`)
2. **Dispatcher**: `SAM_CODE_7d48_COPY_PRG_SWITCH_LOOP` reads bytes from program stream
3. **State 0x10 Handler**: When bits 3-5 = 0x10, calls `SAM_CODE_8336`
4. **Negative Delta Write**: `SAM_CODE_8336` writes **negative delta** to SAM D-RAM:
   ```c
   DAT_EXTMEM_8002 = ~(value) + 1;  // Negative value
   DAT_EXTMEM_8003 = ~(value >> 5); // Negative MSB
   ```

### Two Processing Paths

1. **FUN_CODE_6d25_SAM_UPDATE**: Main loop that checks internal memory slot state
   - If state & 0x38 == 0x10: calls `SAM_CODE_7301` with `_6_0=0`

2. **SAM_CODE_7d48_COPY_PRG_SWITCH_LOOP**: Program data dispatcher
   - Reads state bytes from program stream
   - **Writes state to internal memory**: `*(byte*)bVar8 = bVar5;`
   - Dispatches to handler based on bits 3-5

### Key Finding

For piano decay to work, the **program data stream** must contain a command with state 0x10.
This command would be queued by the note-off handler specific to piano algorithm.

**Hypothesis**: Piano note-off handler doesn't queue the state 0x10 command, or there's
a condition preventing it from being processed.

### Program Data Flow

```
ROM/External Memory (program data)
        ↓
DAT_INTMEM_3b/3c (source pointer)
        ↓
SAM_CODE_7d6f_copy_program (copies 7 bytes to internal memory)
        ↓
SAM_CODE_7d48_COPY_PRG_SWITCH_LOOP (dispatcher)
        ↓
state & 0x38 == 0x10 → SAM_CODE_8336 (decay handler)
        ↓
Writes negative delta to SAM D-RAM (DAT_EXTMEM_8002/8003)
```

### Key Flags

- `_5_4`: When set, skips much of the program processing
- `_5_5`: Related to external memory access
- `_5_1`, `_5_2`: Memory access protection (T1/EA toggle)

### Possible Root Causes

1. **Piano program data doesn't contain state 0x10**: The piano envelope ROM data might not include a decay/release phase with state 0x10

2. **Note-off doesn't advance program pointer**: For organ, note-off directly sets idle bit. For piano, note-off should advance the program pointer to the decay phase, but this might not be happening

3. **Flag `_5_4` blocking**: This flag skips program processing when set - might be incorrectly set for piano

4. **Program structure difference**: Piano might use a different envelope mechanism than the state-based dispatch system

## 2026-01-08 Update: IRQ Latch Analysis

### IRQ Latch IS Set for Word 8

The IRQ latch IS being set for slot 2 word 8:
```
SAM IRQ latch SET: slot 2 word 8 (latch 00 -> 28) intmod=1 carry=0
```

This happens because:
- ALG2 PC19: `WA WSP` for word 4 sets `intmod=1`
- ALG2 PC21: `WM WSP` for word 8 uses `intmod=1`
- IRQ condition: `clrq && (intmod || carry)` = true → latch set to 0x28

### IRQ Latch NOT Set for Word 2 (Amplitude)

For amplitude updates (word 2), IRQ is blocked:
```
WM_WSP S2W2: IRQ blocked - clrq=1 intmod=0 carry=0 (needs intmod=1 or carry=1)
```

This happens because:
- ALG2 PC23: `plain WA` for word 3 sets `clrq=1, intmod=0`
- ALG2 PC24: `WM WSP` for word 2 needs `intmod=1 OR carry=1`
- With `intmod=0` and `carry=0` (since delta=0), no IRQ

### Firmware Reads Latch But Discards Value

The firmware reads the IRQ latch at PC=0x7686 inside `SAM_CODE_7671_WRAP_WRITE`:
```asm
CODE:7683: MOV R0,#0x0
CODE:7685: MOVX A,@R0    ; Read IRQ latch
CODE:7686: MOV 0xa0,0x30 ; Restore P2
CODE:7689: RET
```

All callers (SAM_CODE_6afe, SAM_CODE_8900) **discard the return value**.

### Key Difference: Organ vs Piano

| Aspect | Organ (ALG=1) | Piano (ALG=2) |
|--------|---------------|---------------|
| WM WSP word 2 | `clrq=0, intmod=1` | `clrq=1, intmod=0` |
| Write enabled | No (clrq=0) | Yes (clrq=1) |
| IRQ condition | No (clrq=0) | No (intmod=0, carry=0) |
| Note-off | Firmware sets idle bit | Should set state 0x10 |

### Hypothesis: Missing State Transition Trigger

The firmware should transition slot state to 0x10 (decay) when:
1. Attack amplitude reaches maximum (WM WSP carry=1)
2. OR MIDI note-off is received

For piano:
- Carry never becomes 1 (delta=0, no overflow)
- No code path sets state to 0x10 on note-off
- Without state=0x10, `_6_0` never set to decay mode
- Without decay mode, SAM_CODE_7301 never writes negative delta

### Current Status

**The issue is likely in firmware, not SAM emulation:**
- SAM correctly sets IRQ latch for word 8 (intmod=1)
- SAM correctly blocks IRQ for word 2 (intmod=0, carry=0)
- Firmware reads latch but doesn't use the value

**Possible explanations:**
1. Firmware expects a different trigger mechanism we haven't found
2. Piano release is intentionally not implemented in this firmware version
3. Real hardware has additional circuitry not emulated

## 2026-01-08 Update: Piano Program Data Analysis

### Piano Has No Decay State Command

Examined piano program data at 0x5117 ("pia43"):

```
Offset  Hex                              Meaning
0-7:    70 69 61 34 33 20 20 20          "pia43   " (name)
...
19-21:  00 00 08                         NULL ptr + state 0x08 (ATTACK setup)
                                         NOT state 0x10 (decay)!
```

**Key finding**: Piano program data contains `00 00 08` (state 0x08 = attack setup) but
**NO `00 00 10` sequence** (state 0x10 = decay). The program never triggers decay dispatch!

### _6_0 Flag Mismatch

Even if piano had state 0x10, there's another problem:

**In FUN_CODE_6d25_SAM_UPDATE:**
```c
if (bVar5 == 0x10) {
    _6_0 = 0;           // Sets _6_0 = 0 (NOT decay mode!)
    SAM_CODE_7301();
}
```

**In SAM_CODE_7301:**
```c
cVar6 = _6_0;
if (cVar6 == '\x01') {
    // DECAY path - writes NEGATIVE delta!
    DAT_EXTMEM_8001 = ~bVar5 + 1;      // Negative!
    DAT_EXTMEM_8002 = ~bVar7 + bVar8;  // Negative!
    ...
}
```

So when state == 0x10 is processed:
1. `_6_0` is set to **0** (not 1)
2. SAM_CODE_7301 is called
3. But decay path requires `_6_0 == 1`
4. Decay path is NOT taken!

### State Handler Functions

| State bits 3-5 | Handler Function | Purpose |
|----------------|------------------|---------|
| 0x00 | SAM_CODE_8009_SWITCH_CASE_0 | Unknown |
| 0x08 | SAM_CODE_8045 | **ATTACK** - pitch/waveform setup |
| 0x10 | SAM_CODE_8336 | **DECAY** - writes negative delta |
| 0x18 | SAM_CODE_8580_SWITCH_CASE_0x18 | Unknown |
| 0x20 | SAM_CODE_860e_SWITCH_CASE_0x20 | Unknown |
| 0x28 | SAM_CODE_8692_SWITCH_CASE_0x28 | Unknown |
| 0x30 | SAM_CODE_868f_SWITCH_CASE_0x30 | Unknown |
| 0x38 | (skip) | **IDLE** |

### Decay Mechanism Summary

**For decay to work, TWO things must happen:**

1. Program data must contain `00 00 10` (null pointer + state 0x10)
   - Piano has `00 00 08` (attack) only
   - No decay section in piano program ROM

2. FUN_CODE_6d25 must set `_6_0 = 1` before calling SAM_CODE_7301
   - Currently sets `_6_0 = 0`
   - SAM_CODE_7301 decay path never executes

### Root Cause Confirmed

Piano decay doesn't work because:
1. **Piano ROM program data has no decay commands** (no state 0x10)
2. **Note-off doesn't advance program pointer to decay** (unlike organ which sets idle bit)
3. **Even if triggered, _6_0=0 blocks decay path**

This appears to be a **firmware design choice** - piano sounds may not decay on real hardware either.

### Possible Fixes

1. **Firmware workaround in emulation**: Detect note-off for piano slots and directly write negative delta to SAM D-RAM (mimicking what decay handler would do)

2. **Check real hardware**: Verify if Keyfox10 piano actually decays on note-off

3. **Implement piano-specific decay**: In SAM emulation, track note-on/off state and apply software envelope when `_6_0` mechanism isn't used

## Test Commands

```bash
# Run with program 2 (piano) and capture logs
timeout 25 ./mamemuse keyfox10 -rompath /path/to/roms -seconds_to_run 14 -oslog -midiin /tmp/test_prog2.mid 2>&1 | tee /tmp/sam_prog2.log

# Filter for slot 2 WM WSP
grep "WM WSP slot2 word2" /tmp/sam_prog2.log

# Check D-RAM writes to word2
grep "D-RAM\[22\].*slot 2" /tmp/sam_prog2.log

# Check IRQ latch setting
grep "IRQ latch SET" /tmp/sam_prog2.log

# Check param15 (word 15) writes
grep "D-RAM\[2F\]" /tmp/sam_prog2.log
```
