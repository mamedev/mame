# SAM8905 Standalone Test Harness Implementation

## Goal
Standalone test harness for SAM8905 DSP emulation that loads A-RAM/D-RAM captures from Keyfox10 ROM and generates audio output for verification.

## Directory Structure
```
mame/sam8905/
├── sam8905_core.h        # Standalone SAM8905 core
├── sam8905_core.cpp      # Core implementation without MAME deps
├── test_harness.cpp      # Main test program
├── extract_test_data.py  # Extract A-RAM/D-RAM from ROM
├── test_data/            # Generated test data files
└── Makefile
```

---

## Task List

### Phase 1: Extract SAM8905 Core

- [x] **Task 1.1**: Create sam8905_core.h
  - Copy slot_t structure from sam8905.h
  - Define standalone class without MAME device inheritance
  - Add SSR mode flag for 22.05kHz support

- [x] **Task 1.2**: Create sam8905_core.cpp
  - Port execute_cycle() from sam8905.cpp
  - Port get_waveform() and get_constant()
  - Implement generate_samples()
  - Add proper dB attenuation lookup for MIXL/MIXR
  - Fix 22.05kHz mode (4 algos × 64 instructions)
  - Fix signed 19-bit accumulation

### Phase 2: Test Data Extraction

- [x] **Task 2.1**: Create extract_test_data.py
  - Extract algorithm data (128 bytes at DATA:E000, etc.)
  - Build D-RAM configuration for a specific sound
  - Output binary files for test harness

- [x] **Task 2.2**: Generate test data files
  - algo_e000.bin through algo_e2c1.bin: FX algorithms from ROM
  - dram_sine_440hz.bin: 440Hz internal sine configuration
  - dram_chord.bin: Two-note chord configuration

### Phase 3: Test Harness

- [x] **Task 3.1**: Create test_harness.cpp
  - Load A-RAM from algorithm file
  - Load D-RAM configuration
  - Generate samples and write WAV
  - Added make_inst() helper for instruction encoding

- [x] **Task 3.2**: Create Makefile
  - Targets: all, data, test, test-algo, test-rom, play, clean

### Phase 4: Test Cases

- [x] Test 1: Internal Sine Wave (440Hz verified with sox)
- [ ] Test 2: MIXL/MIXR Attenuation
- [x] Test 3: 22.05kHz Mode (verified working - 64 instructions per algorithm)
- [ ] Test 4: Full Sound (with ROM algorithms)
- [x] Test 5: Manual Algorithm (fixed WPHI WSP priority rule, now works)

---

## Key Implementation Details

### D-RAM Word 15 Format (from German docs)
```
| 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
```
- I: Idle flag (1=slot produces no sound)
- ALG: Algorithm number (0-7 for 44.1kHz, 0-3 for 22.05kHz)
- M: Interrupt mask

### 22.05kHz Mode (Keyfox10)
```cpp
// 4 algorithms × 64 instructions
// Per Table 6: Use AL2,AL1 (bits 10-9), not AL1,AL0 (bits 9-8)
uint8_t alg = (param15 >> 9) & 0x3;  // Use upper 2 bits of ALG field
uint16_t pc_start = alg << 6;        // 64 instructions
for (int pc = 0; pc < 64; ++pc) { ... }
```

### dB Attenuation Lookup
```cpp
// MIX code -> attenuation
// 000: mute, 001: -36dB, 010: -30dB, 011: -24dB
// 100: -18dB, 101: -12dB, 110: -6dB, 111: 0dB
static const uint16_t mix_atten[8] = {
    0, 16, 32, 64, 128, 256, 512, 1024
};
```

### Algorithm Addresses (FX chip)
- DATA:E000 (ROM 0x1E000) - FX Algorithm 0
- DATA:E090 (ROM 0x1E090) - FX Algorithm 1
- DATA:E114 (ROM 0x1E114) - FX Algorithm 2
- DATA:E1AD (ROM 0x1E1AD) - FX Algorithm 3
- DATA:E231 (ROM 0x1E231) - FX Algorithm 4
- DATA:E2C1 (ROM 0x1E2C1) - FX Algorithm 5

---

## Files

| File | Status |
|------|--------|
| sam8905_core.h | Pending |
| sam8905_core.cpp | Pending |
| test_harness.cpp | Pending |
| extract_test_data.py | Pending |
| Makefile | Pending |

## ROM Source
`/home/jeff/bastel/roms/hohner/keyfox10/kf10_ic27_v2.bin`

---

## Progress Log

*Started: 2025-12-28*

### 2025-12-28 - Initial Implementation Complete

- Created standalone SAM8905 core (sam8905_core.h/cpp)
- Fixed multiple issues:
  - 22.05kHz mode: 4 algorithms × 64 instructions
  - D-RAM word 15 format: I at bit 11, ALG at bits 10-8
  - Proper dB attenuation lookup for MIXL/MIXR
  - Signed 19-bit value handling in accumulator
  - D-RAM waveform format: WF at bits 17-9 (not 8-0)

- Built-in test generates 440Hz sine wave (verified with sox)
- Test harness supports loading A-RAM/D-RAM from binary files

### 2025-12-29 - WA WSP Priority Rule Fix

Fixed a critical bug in WA+WSP handling. The manual's sine algorithm now works correctly.

**Root cause**: Section 8-3 of the programmer's guide states:
> "WPHI WSP takes priority over WA WSP giving a normal WA"

When **both WPHI and WA are active with WSP** (as in `RM PHI,<WA,WPHI,WSP>`):
- WPHI+WSP executes normally (loads PHI, sets WF=0x100 for internal sine)
- WA gets **normal** behavior (just loads A from bus)
- The WA WSP conditional synthesis truth table does NOT apply

The previous implementation unconditionally applied the WA WSP truth table when WSP was set, ignoring this priority rule. This caused A to be set to 0 or 0x200 instead of loading the phase value.

**Fix** (in `sam8905_core.cpp`):
```cpp
// WA (Write A)
if (!BIT(inst, 7)) {
    bool wphi_active = !BIT(inst, 4);

    // "WPHI WSP takes priority over WA WSP giving a normal WA" (Section 8-3)
    if (wsp && !wphi_active) {
        // WA WSP Truth Table - only when WPHI is NOT also active
        // ... conditional synthesis logic ...
    } else {
        // Normal WA: load A from bus
        slot.a = bus;
        slot.clear_rqst = true;
        slot.int_mod = false;
    }
}
```

**Manual's algorithm now works** (Section 7, Example 1):
```
RM      PHI,    <WA,WPHI,WSP>  ;Load A and PHI, WF=0x100 (sine)
RM      DPHI,   <WB>           ;Load frequency
RM      AMP,    <WXY,WSP>      ;X=sin(PHI), Y=AMP, mix
RADD    PHI,    <WM>           ;Store new phase
RSP                            ;Wait for multiplier
              ,<WACC>          ;Accumulate
```

Verified: 440Hz sine wave output (measured 438Hz with sox).

### Usage

```bash
cd mame/sam8905
make           # Build test_harness
make data      # Extract test data from ROM
make test      # Run built-in sine test
make test-algo # Test with minimal algorithm file
```
