# SAM8905 Implementation Review

Review of `sam8905.cpp` against the programmer's guide and Keyfox10 analysis.

## Build Integration
- [x] Added to `scripts/src/sound.lua` under SOUNDS["HOHNER"]

---

## Correct Implementation Details

### Memory Sizes
- **A-RAM**: 256 x 15-bit ✓ (code uses `uint16_t[256]`)
- **D-RAM**: 256 x 19-bit ✓ (code uses `uint32_t[256]` with MASK19)

### Micro-instruction Format (Appendix III)
```
| I14-I12 | I11-I10 | I9  | I8 | I7 | I6 | I5 | I4 | I3 | I2 | I1 | I0 |
|   MAD   |  CMD    | WSP | WA | WB | WM |WPHI|WXY |clrB|WWF |WACC|
```
- ✓ MAD at bits 14-11 (though guide says 14-12, implementation uses 11)
- ✓ CMD (emitter) at bits 10-9
- ✓ WSP at bit 8
- ✓ WA at bit 7 (active low)
- ✓ WB at bit 6 (active low)
- ✓ WM at bit 5 (active low)
- ✓ WPHI at bit 4 (active low)
- ✓ WXY at bit 3 (active low)
- ✓ clearB at bit 2 (active low)
- ✓ WWF at bit 1 (active low)
- ✓ WACC at bit 0 (active low)

---

## Issues Found

### 1. **MAD Field Width** - MINOR
Code extracts MAD as 4 bits (`(inst >> 11) & 0xF`), but guide shows MAD as 3 bits (I14-I12).
For D-RAM addressing (16 words per slot), 4 bits makes sense for the address.

**Verdict**: Likely correct for D-RAM addressing, but verify against actual ROM usage.

### 2. **D-RAM Word 15 Format** - CRITICAL
From German hardware documentation:
```
| 18-12 | 11 | 10-8 | 7 | 6-0 |
|   ?   | I  | ALG  | M |  ?  |
```

- **I** (bit 11): Idle - slot produces no sound when set
- **ALG** (bits 10-8): Algorithm number (0-7)
- **M** (bit 7): Interrupt mask - no interrupt when set

Code at line 195-197:
```cpp
if (BIT(param15, 7)) continue; // Idle bit - WRONG!
uint8_t alg = param15 & 0x7F;  // WRONG!
```

**Problems**:
- Idle bit is at bit 11, not bit 7
- ALG is at bits 10-8, not bits 6-0
- Code checks M (interrupt mask) as Idle

**Fix needed**:
```cpp
bool idle = BIT(param15, 11);         // I bit at 11
if (idle) continue;
uint8_t alg = (param15 >> 8) & 0x7;   // ALG bits 10-8
```

### 3. **Algorithm Addressing for 22.05kHz Mode** - CRITICAL
From programmer's guide (§4):
- 44.1 kHz: 8 algorithms × 32 instructions, ALG uses 3 bits
- 22.05 kHz: 4 algorithms × 64 instructions, ALG uses 2 bits

Code at line 199-200:
```cpp
uint16_t pc_start = alg << 5; // 44.1kHz = 32 instructions per block
for (int pc = 0; pc < 32; ++pc) {
```

**Keyfox10 uses 22.05kHz mode** (SSR=1), which means:
- Only 4 algorithm slots
- 64 instructions per algorithm
- ALG field uses only 2 bits (bits 7-6)

**Fix needed**:
```cpp
// Check SSR bit in control register
bool ssr = BIT(m_control_reg, 3);
if (ssr) {
    // 22.05kHz mode: 4 algorithms × 64 instructions
    uint8_t alg_22k = (param15 >> 6) & 0x3;  // Only 2 bits
    uint16_t pc_start = alg_22k << 6;  // 64 instructions per block
    for (int pc = 0; pc < 64; ++pc) { ... }
} else {
    // 44.1kHz mode: 8 algorithms × 32 instructions
    ...
}
```

### 4. **Sample Rate Calculation** - NEEDS VERIFICATION
Code at line 21:
```cpp
m_stream = stream_alloc(0, 2, clock() / 1024);
```

From programmer's guide (§3):
- With 45.1584 MHz quartz: 44.1 kHz or 22.05 kHz
- 45.1584 MHz / 1024 = 44,100 Hz ✓

This is correct for 44.1kHz mode. For 22.05kHz mode, it should be clock() / 2048.

### 5. **Waveform Generator** - PARTIAL
Code implements internal sine and ramps, but:
- Sine formula uses different scaling than guide
- Ramp implementations look approximately correct
- Constants table (get_constant) values don't match Appendix I exactly

From Appendix I, MAD=0 should give X=0.0004883, which is 0x40 / 0x10000 ≈ 0.00098.
Code uses 0x00040, which when scaled would give different value.

### 6. **MIXL/MIXR Attenuation** - INCORRECT
From programmer's guide (§5):
```
000 = no signal
001 = -36 dB
010 = -30 dB
...
111 = 0 dB
```

Code at line 179:
```cpp
slot.l_acc += (slot.mul_result * slot.mix_l) >> 3;
```

This is linear scaling, not dB-based logarithmic attenuation.

**Fix needed**: Use lookup table for dB attenuation:
```cpp
static const uint16_t mix_atten[8] = {
    0,      // 000: mute
    16,     // 001: -36dB (1/64)
    32,     // 010: -30dB (1/32)
    64,     // 011: -24dB (1/16)
    128,    // 100: -18dB (1/8)
    256,    // 101: -12dB (1/4)
    512,    // 110: -6dB (1/2)
    1024    // 111: 0dB (1/1)
};
```

### 7. **CPU Interface Addressing** - PARTIAL
From programmer's guide (§10-2):
- A2=1: Control byte
- A2=0, A1=0, A0=0: Address register
- A2=0, A1=0, A0=1: Data LSB
- A2=0, A1=1, A0=0: Data NSB (or MSB for A-RAM)
- A2=0, A1=1, A0=1: Data MSB (D-RAM only)

Code uses `offset & 7` with cases 0-4, which partially matches.

### 8. **Reserved Instructions** - NOT IMPLEMENTED
From programmer's guide (§4):
> addresses 30 and 31 [62 and 63] are reserved for micro-processor access

Code executes all 32 instructions without skipping reserved ones.

---

## Summary

| Category | Status |
|----------|--------|
| Basic structure | ✓ Correct |
| Memory sizes | ✓ Correct |
| Instruction decode | ✓ Mostly correct |
| D-RAM word 15 format | ✗ Wrong bit positions |
| 22.05kHz mode | ✗ Not implemented |
| Attenuation | ✗ Linear instead of dB |
| Waveform accuracy | ~ Approximate |
| Reserved instructions | ✗ Not skipped |

## Priority Fixes

1. **HIGH**: Fix D-RAM word 15 bit positions (I at bit 11, ALG at bits 10-8, M at bit 7)
2. **HIGH**: Add SSR mode support for 22.05kHz (Keyfox10 uses this)
3. **MEDIUM**: Implement proper dB attenuation for MIXL/MIXR
4. **LOW**: Skip reserved instruction slots 30-31 (or 62-63)
5. **LOW**: Verify waveform/constant accuracy against guide
