# Phase 5/7 Function Analysis - Porting Strategy

Analysis of `voice_init_next_slot` (CODE:AB40) and `periodic_voice_update` (CODE:9BA7)
to identify smaller, independently portable packages.

## Overview

Both functions are complex orchestrators that call multiple sub-functions. The key insight
is that **the sub-functions are mostly independent leaf functions** that can be ported and
tested separately.

---

## 1. voice_init_next_slot (CODE:AB40)

### Call Graph

```
voice_init_next_slot (AB40)
├── voice_init_copy_and_envelope (AB73)  ← mutual recursion!
│   ├── velocity_curve_lookup
│   ├── signed_multiply_sat (AA6F)
│   ├── signed_multiply_chain (AA52)
│   └── voice_init_next_slot (AB40)  ← recurses back
└── dram_config_handler_XX (already ported)
    ├── dram_config_handler_00 (AD8F) ✓
    ├── dram_config_handler_08 (ADBD) ✓
    ├── dram_config_handler_10 (B030) ✓
    ├── dram_config_handler_18 (B222) ✓
    ├── dram_config_handler_20 (B278) ✓
    ├── dram_config_handler_28 (B2D2) ✓
    └── dram_config_handler_30 (B2CF) ✓
```

### Structure Analysis

`voice_init_next_slot` has two main paths:

1. **Voice Slot Advance Path** (when `voice_slot_base + 0x10` doesn't overflow):
   - Reads 2-byte pointer from ROM (voice_data_ptr)
   - If pointer != 0: advances to next 16-byte slot block, calls `voice_init_copy_and_envelope`
   - If pointer == 0: marks slot as inactive (0xFF)

2. **D-RAM Config Dispatch Path** (when voice_slot_base overflows to 0x80+):
   - Sets up D-RAM addressing: `voice_slot_base = (dram_address_counter << 4) >> 1 | 0x80`
   - Loops up to 16 times reading dispatch bytes from ROM
   - Switch on bits 5:3 (& 0x38) to call appropriate handler
   - Handler 0x38 is "skip" - writes 0x38, advances pointers
   - Loop terminates on bit 7 set (0x80+)

### Leaf Functions to Port First

| Priority | Function | Address | Complexity | Dependencies |
|----------|----------|---------|------------|--------------|
| 1 | `signed_multiply_sat` | AA6F | Low | None |
| 2 | `signed_multiply_chain` | AA52 | Low | signed_multiply_sat |
| 3 | `velocity_curve_lookup` | ~AA00 | Medium | ROM tables |

### Mutual Recursion Problem

`voice_init_next_slot` ↔ `voice_init_copy_and_envelope` are mutually recursive.
This is a **state machine** pattern:
- `voice_init_next_slot`: Dispatches to D-RAM handlers OR advances to next envelope block
- `voice_init_copy_and_envelope`: Copies 7 bytes, sets up envelope, then calls back

**Solution**: Port both together as a unit, or use a loop/state machine instead of recursion.

---

## 2. periodic_voice_update (CODE:9BA7)

### Call Graph

```
periodic_voice_update (9BA7)
├── voice_free (9946) ✓ already ported
│   └── voice_slots_clear (99BC)
│       └── sam_dram_write_word15 (A523)
├── global_mod_lfo_update (A314)  ← LEAF, simple!
├── signed_multiply_sat (AA6F)    ← LEAF
├── dram_slot_amplitude_update (A18F)
│   └── dram_slot_apply_mod_depth (A2E3)
│       └── signed_multiply_chain (AA52)
├── dram_slot_portamento_update (A33E)
└── modulation_write_dram (9FCD)  ← LEAF, medium complexity
```

### Structure Analysis

`periodic_voice_update` has two major parts:

#### PART 1: LFO/Envelope Block Processing (7 blocks × 16 bytes)

Iterates `voice_slot_base` from 0x00 to 0x60 (step 0x10):
- Each 16-byte block is an LFO/envelope generator
- Reads control byte at slot+2, processes based on type
- LFO waveform computation (switch on type & 7):
  - 0,5,6,7: Sine (table lookup at CODE:9833)
  - 1: Ramp (identity)
  - 2: Inverted ramp
  - 3: Square
  - 4: Noise (LFSR: x = x*3 + 0x43)
- Envelope segment processing (3-byte entries from ROM)

#### PART 2: D-RAM Slot Modulation (page+0x70..0xFF)

Iterates `dram_slot_index` from 0x70 to 0x4F (voice slot lookup table):
- Reads slot mapping from page+0x70 area
- For each active slot, reads mod type from page+0x80+slot*8
- Dispatches based on type:
  - 0x10: amplitude → `dram_slot_amplitude_update`
  - bit7 set: portamento → `dram_slot_portamento_update`
  - 0x38: skip
  - other: pitch modulation → `modulation_write_dram`

### Leaf Functions to Port First (Priority Order)

| Priority | Function | Address | Lines | Complexity | Notes |
|----------|----------|---------|-------|------------|-------|
| **1** | `global_mod_lfo_update` | A314 | ~15 | **Very Low** | Pure leaf, no calls |
| **2** | `signed_multiply_sat` | AA6F | ~20 | Low | Pure math, no calls |
| **3** | `signed_multiply_chain` | AA52 | ~15 | Low | Calls signed_multiply_sat |
| **4** | `modulation_write_dram` | 9FCD | ~50 | Medium | Writes to SAM D-RAM |
| **5** | `dram_slot_apply_mod_depth` | A2E3 | ~30 | Medium | Calls signed_multiply_chain |
| **6** | `dram_slot_portamento_update` | A33E | ~60 | Medium | Reads/writes voice slot |
| **7** | `dram_slot_amplitude_update` | A18F | ~100 | **High** | Complex, multiple paths |

---

## 3. Recommended Porting Packages

### Package A: Math Utilities (Prerequisite)

**Files**: `sam_math.h`, `sam_math.c`
**Functions**:
- `signed_multiply_sat` (AA6F) - saturating signed 8×8→8 multiply
- `signed_multiply_chain` (AA52) - chained multiply with accumulate
- `velocity_curve_lookup` - velocity/curve table lookup

**Why first**: These are pure math functions with no external dependencies.

### Package B: Global LFO

**Files**: `sam_lfo.h`, `sam_lfo.c`
**Functions**:
- `global_mod_lfo_update` (A314)

**Why second**: Simplest periodic update function. Only needs:
- EXTMEM access (0x1180-0x1183)
- Sine table (CODE:9833)

**Implementation**:
```c
void global_mod_lfo_update(void)
{
    if (g_extmem.mod_lfo_rate == 0) return;

    // Phase increment: rate * 32 (5-bit left shift)
    uint16_t phase = ((uint16_t)g_extmem.mod_lfo_phase_hi << 8) | g_extmem.mod_lfo_phase_lo;
    uint16_t increment = (uint16_t)g_extmem.mod_lfo_rate << 5;
    phase += increment;

    g_extmem.mod_lfo_phase_lo = phase & 0xFF;
    g_extmem.mod_lfo_phase_hi = phase >> 8;

    // Sine lookup: phase_hi >> 1, masked to 6 bits
    uint8_t table_idx = (g_extmem.mod_lfo_phase_hi >> 1) & 0x3F;
    g_extmem.mod_lfo_output = g_sine_table[table_idx];
}
```

### Package C: Pitch Modulation Write

**Files**: Add to `sam_dram_config.c`
**Functions**:
- `modulation_write_dram` (9FCD)

**Why third**: Needed by periodic update for pitch modulation.
Medium complexity but isolated - just computes and writes D-RAM.

### Package D: Portamento Update

**Files**: Add to `sam_voice.c` or new `sam_modulation.c`
**Functions**:
- `dram_slot_portamento_update` (A33E)

**Why fourth**: Self-contained pitch glide calculation.
Reads target/current pitch from voice slot, writes to D-RAM.

### Package E: Amplitude/Envelope Update

**Files**: `sam_envelope.c`
**Functions**:
- `dram_slot_apply_mod_depth` (A2E3)
- `dram_slot_amplitude_update` (A18F)

**Why fifth**: Most complex single function. Multiple code paths for:
- Direct amplitude write
- Velocity-scaled amplitude
- Envelope-gated amplitude
- Read-modify-write D-RAM operations

### Package F: Voice Init Envelope

**Files**: Add to `sam_voice.c`
**Functions**:
- `voice_init_copy_and_envelope` (AB73)
- `voice_init_next_slot` (AB40) - refactored as loop

**Why sixth**: Depends on Package A (math) and existing D-RAM handlers.

### Package G: Periodic Voice Update (Final Assembly)

**Files**: `sam_periodic.c`
**Functions**:
- `periodic_voice_update` (9BA7) - main orchestrator

**Why last**: This is the glue that calls everything else.
Can be stubbed to call packages B-F progressively.

---

## 4. Voice Page Memory Layout (for reference)

```
Voice Page (256 bytes per voice, at EXTMEM pages 0x00-0x0F)

LFO/Envelope Blocks (7 × 16 bytes = 112 bytes at 0x00-0x6F):
  Each block:
    [0-1]: Envelope segment pointer (LE)
    [2]:   Control flags (bit7=enable, bits2:0=waveform type)
    [3]:   Reserved
    [4]:   Sustain/attack param
    [5]:   Depth/modulation param
    [6]:   Output amplitude
    [7]:   Output value (LFO result)
    [8-9]: Phase accumulator (16-bit)
    [10-11]: Rate/limit (16-bit)
    [12]:  Envelope control
    [13-15]: Envelope segment data

Slot Mapping Table (16 bytes at 0x70-0x7F):
  Maps D-RAM slot indices to mod state blocks

Mod State Blocks (8 × 8 bytes = 64 bytes at 0x80-0xBF):
  Per D-RAM slot modulation state

Voice Status (at 0xFB-0xFF):
  [0xFB]: Status flags (bit6=ROM access, bit5=release pending)
  [0xFC]: Channel << 4 | note info
  [0xFD]: Note number
  [0xFE]: Next voice in linked list (0xFF=end)
  [0xFF]: Slot count / active marker
```

---

## 5. Sine Table (CODE:9833)

64-entry quarter-wave sine table (0x00-0x7F range):

```c
// Extract from ROM or compute
static const uint8_t g_sine_table[64] = {
    0x00, 0x03, 0x06, 0x09, 0x0C, 0x0F, 0x12, 0x15,
    0x18, 0x1B, 0x1E, 0x21, 0x24, 0x27, 0x2A, 0x2D,
    0x30, 0x33, 0x36, 0x38, 0x3B, 0x3E, 0x40, 0x43,
    0x45, 0x48, 0x4A, 0x4D, 0x4F, 0x51, 0x53, 0x55,
    0x58, 0x5A, 0x5C, 0x5E, 0x5F, 0x61, 0x63, 0x65,
    0x66, 0x68, 0x69, 0x6B, 0x6C, 0x6D, 0x6E, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x74, 0x75, 0x76, 0x76,
    0x77, 0x78, 0x78, 0x79, 0x79, 0x79, 0x7A, 0x7A
};
```

---

## 6. Implementation Order Summary

```
Week 1: Foundation
  [A] sam_math.c: signed_multiply_sat, signed_multiply_chain
  [B] sam_lfo.c: global_mod_lfo_update + sine table

Week 2: D-RAM Modulation
  [C] modulation_write_dram (pitch mod)
  [D] dram_slot_portamento_update

Week 3: Envelope System
  [E] dram_slot_apply_mod_depth
  [E] dram_slot_amplitude_update

Week 4: Voice Init Completion
  [F] voice_init_copy_and_envelope
  [F] voice_init_next_slot (as state machine)

Week 5: Integration
  [G] periodic_voice_update (orchestrator)
  Integration testing with real programs
```

---

## 7. Testing Strategy

### Unit Tests for Each Package

**Package A (Math)**:
- Test signed_multiply_sat with edge cases: 0×0, 127×127, -128×127, overflow saturation
- Test signed_multiply_chain accumulation

**Package B (LFO)**:
- Test phase increment at various rates
- Test sine table lookup produces expected values
- Test rate=0 early exit

**Package C (Pitch Mod)**:
- Test with known mod values, verify D-RAM write
- Test early exit when value unchanged

**Package D (Portamento)**:
- Test glide from low→high pitch
- Test glide completion (flag clear)

**Package E (Amplitude)**:
- Test direct amplitude write
- Test velocity scaling

**Package F/G (Integration)**:
- Send note-on, verify envelope starts
- Send note-off, verify release phase
- Verify periodic updates modify D-RAM

---

## 8. Simplification Options

If full fidelity is not required, these can be simplified:

1. **LFO waveforms**: Only implement sine (most common), stub others
2. **Envelope segments**: Implement simple ADSR, skip complex segment chaining
3. **Portamento**: Skip entirely (just set pitch directly)
4. **Velocity scaling**: Use linear instead of curve table lookup
5. **Amplitude update**: Skip read-modify-write, just overwrite

The emulator currently works with basic note-on/off. Adding Package B alone
would give working mod wheel support. Package C+D would add pitch bend and
portamento. Package E would add proper volume envelopes.

---

## 9. Progress Tracking

| Package | Status | Tests | Notes |
|---------|--------|-------|-------|
| A: Math | ✅ COMPLETE | test_signed_multiply_sat, test_signed_multiply_chain | In sam_math.h |
| B: LFO | ✅ COMPLETE | test_sine_table, test_global_mod_lfo_update, test_noise_lfsr | In sam_lfo.c/h |
| C: Pitch Mod | ✅ COMPLETE | test_multiply_16x24, test_modulation_write_dram | multiply_16x24 in sam_math.h, modulation_write_dram in sam_dram_config.c |
| D: Portamento | ✅ COMPLETE | test_portamento_update | dram_slot_portamento_update in sam_dram_config.c |
| E: Amplitude | ✅ COMPLETE | test_apply_mod_depth, test_amplitude_update | SIMPLIFIED - see shortcuts below |
| F: Voice Init | ✅ COMPLETE | test_voice_init_copy_and_envelope | SIMPLIFIED - see shortcuts below |
| G: Orchestrator | ⏳ PENDING | | periodic_voice_update final assembly |

### Package E Shortcuts (requires follow-up for full fidelity)

**dram_slot_apply_mod_depth (A2E3):**
- Uses linear velocity instead of curve table lookup
- Missing bank register preservation (not needed in C)

**dram_slot_amplitude_update (A18F):**
- No read-modify-write D-RAM (affects smooth envelope release)
- No envelope block cross-reference (affects dynamic envelope)
- No negative amplitude handling (affects inverted envelopes)
- Simplified done flag propagation
- Linear velocity instead of curve table

### Package F Shortcuts (requires follow-up for full fidelity)

**voice_init_copy_and_envelope (AB73):**
- Velocity modulation of envelope params not implemented (bytes 9-11 in ROM)
- velocity_curve_lookup replaced with linear approximation
- Envelope table scanning is simplified (may miss edge cases)
- signed_multiply_sat for envelope level scaling not fully matched
- Bank register preservation not tracked (not needed in C)

**voice_init_next_slot (AB40):**
- Uses existing dram_config_advance_and_dispatch (already ported)
- Mutual recursion eliminated by calling voice_init_next_slot at end
