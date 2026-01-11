# SAM8905 External Waveform Memory Analysis

## Problem Statement

Piano sounds use external waveform samples (E bit set in word 4), but currently produce only a constant/unclean sound. This suggests the waveform memory connection or address mapping is incorrect.

## Hardware Schematic Analysis

From the schematic image:

### IC20 - SAM8905 (Soundgenerator)
- **MA0-MA15**: Memory Address lower bits (directly to EPROMs?)
- **MAS0-MAS15**: Memory Address upper/supplementary bits (through buffer IC11)

### IC11 - 74HC541 (8-bit Buffer)
- **Inputs A0-A7**: Connected to MAS lines from SAM8905
  - A0 ← MAS0, A1 ← MAS1, ... (needs verification from schematic)
- **Outputs Y0-Y7**: Connected to EPROM upper address lines MDS4-MDS11

### IC17 - 27101 (1Mbit EPROM "Samples")
- 128KB (17 address lines: A0-A16)
- 8-bit data output (D0-D7)
- This is the sample ROM for piano sounds

### IC14 - 27101 (1Mbit EPROM "Rhythmen")
- 128KB (17 address lines: A0-A16)
- 8-bit data output (D0-D7)
- Used for rhythm patterns

## Current Implementation

### SAM8905 Address Generation (sam8905.cpp)
```cpp
// External waveform address calculation
uint32_t addr = ((wf & 0xFF) << 12) | phi;  // WAVE[7:0] | PHI[11:0] = 20 bits
int16_t sample = m_waveform_read(addr);
```

### Driver Callback (keyfox10.cpp)
```cpp
u16 keyfox10_state::sam_snd_waveform_r(offs_t offset)
{
    // offset = 20-bit address from SAM8905
    // ROM address = offset >> 2 (skip lower 2 fractional bits)
    // Upper bit (WA19) selects ROM: 0 = Samples (IC17), 1 = Rhythms (IC14)
    uint32_t rom_addr = (offset >> 2) & 0x1FFFF;  // 17-bit ROM address
    bool use_rhythm = BIT(offset, 19);  // WA19 selects ROM

    u8 sample;
    if (use_rhythm)
        sample = m_rhythms_rom[rom_addr];
    else
        sample = m_samples_rom[rom_addr];

    // Sign-extend 8-bit to 12-bit
    return (int16_t)(int8_t)sample << 4;
}
```

## Potential Issues

### 1. Address Mapping

**SAM8905 address format (20 bits):**
```
| WAVE[7:0] | PHI[11:0] |
|   8 bits  |  12 bits  |
```

**Current mapping to 17-bit ROM address:**
```cpp
rom_addr = (offset >> 2) & 0x1FFFF
```

This means:
- ROM A0 = SAM PHI[2]
- ROM A1 = SAM PHI[3]
- ...
- ROM A9 = SAM PHI[11]
- ROM A10 = SAM WAVE[0]
- ROM A11 = SAM WAVE[1]
- ...
- ROM A16 = SAM WAVE[6]

**Problem**: The `>> 2` shift assumes PHI lower 2 bits are fractional. Is this correct?

Looking at the schematic, the actual address mapping through IC11 might be different. The MAS lines (upper address) go through the buffer separately from MA lines (lower address).

### 2. Wave Format Field Interpretation

Piano word 4 = `0x4E072`:
- If format is `E | WAVE | finalWAVE`:
  - E = bit 19 = 0? or bit 18?
  - WAVE = which bits?
  - finalWAVE = which bits?

In current code:
```cpp
bool internal = (wf & 0x100);  // Bit 8 determines internal/external
```

But WIP doc says `E=1, WAVE=112, final=114` for piano. Need to verify how these fields are packed.

### 3. ROM Bank Selection

**Current implementation:**
```cpp
bool use_rhythm = BIT(offset, 19);  // WA19 selects ROM
```

**Schematic observation:**
- Need to identify which pin from SAM8905 actually selects between IC17 (Samples) and IC14 (Rhythms)
- Could be a different bit or external logic

### 4. Data Format

**EPROMs are 8-bit, but SAM8905 expects 12-bit samples:**

Current conversion:
```cpp
return (int16_t)(int8_t)sample << 4;  // Sign-extend 8→16, shift left 4
```

This gives 12-bit signed value in upper bits. Is this correct?

Some sample ROMs might store:
- Linear 8-bit samples (current assumption)
- Companded (μ-law/A-law) samples
- 12-bit samples split across two bytes

## Schematic Details to Verify

1. **MA/MAS pin connections to EPROMs:**
   - Which SAM pins connect to EPROM A0-A16?
   - What goes through the 74HC541 buffer?

2. **ROM chip select logic:**
   - What determines IC17 vs IC14 selection?
   - Is it a SAM8905 output or external decode logic?

3. **Data bus width:**
   - Are both EPROMs read in parallel for 16-bit data?
   - Or single 8-bit access?

4. **Wave format field packing:**
   - Exact bit positions for E, WAVE, finalWAVE in word 4

## Debugging Steps

### 1. Add Logging
```cpp
u16 keyfox10_state::sam_snd_waveform_r(offs_t offset)
{
    uint32_t rom_addr = (offset >> 2) & 0x1FFFF;
    bool use_rhythm = BIT(offset, 19);

    LOGMASKED(LOG_SAM, "SAM waveform read: offset=%05X → rom_addr=%05X rhythm=%d\n",
              offset, rom_addr, use_rhythm);

    u8 sample = use_rhythm ? m_rhythms_rom[rom_addr] : m_samples_rom[rom_addr];

    LOGMASKED(LOG_SAM, "  sample=%02X (%d)\n", sample, (int8_t)sample);

    return (int16_t)(int8_t)sample << 4;
}
```

### 2. Verify ROM Contents
```bash
# Check if sample ROM has reasonable waveform data
xxd -l 256 /path/to/kf10_ic17.bin

# Look for patterns in piano waveform area
xxd -s $((112 << 10)) -l 4096 /path/to/kf10_ic17.bin | head -20
```

### 3. Test Different Address Mappings

Try variations:
```cpp
// No shift
rom_addr = offset & 0x1FFFF;

// Shift by 1
rom_addr = (offset >> 1) & 0x1FFFF;

// Different bit arrangement
rom_addr = ((offset >> 12) & 0xFF) | ((offset & 0x1FF) << 8);
```

### 4. Verify Wave Format

Add logging to see what waveform addresses are being generated:
```cpp
// In sam8905.cpp get_waveform()
logerror("get_waveform: wf=%05X phi=%03X internal=%d addr=%05X\n",
         wf, phi, internal, addr);
```

## Algorithm 2 Wave Usage

From WIP_algorithm2_analysis.md:
- PC01: `RM 4, <WA>` - Load word4 (wave format) into A
- PC03: `RADD, <WWF>` - WF = A+B (compute waveform address)
- PC19: `RM 4, <WA, WB, WSP>` - WA WSP: check wave cycle end
- PC20: `RADD 4, <WM>` - Update word4

The wave address is computed as `word4 + word5` (wave format + wave offset).

Piano:
- Word 4 = 0x4E072 (wave format)
- Word 5 = 0x00600 (wave offset)
- WF = 0x4E072 + 0x00600 = 0x4E672

This 20-bit value combines with PHI to generate the sample address.

## Findings: Address Mismatch

### Current Address Calculation Trace

```
Piano:
  word4 = 0x4E072
  word5 = 0x00600
  bus = word4 + word5 = 0x4E672

In sam8905.cpp:
  slot.wf = (bus >> 9) & 0x1FF = 0x073  // Only 9 bits stored!

In get_waveform():
  wf & 0x100 = 0 (external mode - correct)
  addr = (0x73 << 12) | phi = 0x73000 | phi

In driver callback:
  rom_addr = (0x73000 >> 2) = 0x1CC00
```

### ROM Data Comparison

**At 0x1CC00 (current calculation):**
```
0001cc00: ffff 0000 ff00 ffff 0000 0000 0000 ffff  // BAD - digital noise!
```

**At 0x1C800 (expected for WAVE=0x72):**
```
0001c800: 0503 00fc f9f7 f4f0 ebe6 e0da d2c6 b8b3  // GOOD - smooth waveform!
```

### Root Cause

The problem is `slot.wf` only stores 9 bits, and the address calculation uses different bit positions:

```cpp
// In WWF receiver:
slot.wf = (bus >> 9) & 0x1FF;  // Extracts bits 9-17 → wf bits 0-8

// In get_waveform:
addr = ((wf & 0xFF) << 12) | phi;  // Uses wf bits 0-7 for WAVE
```

So bus bits 9-16 become the WAVE address. For piano:
- bus = 0x4E672
- bits 9-16 = 0x73 (115 decimal)

But the valid waveform data might be at WAVE=0x72 (114) instead!

**The issue is likely the >> 2 shift in the driver callback:**

```cpp
rom_addr = (offset >> 2) & 0x1FFFF;
```

If we DON'T shift by 2:
- addr = 0x73000
- rom_addr = 0x73000 & 0x1FFFF = 0x13000

Let me check 0x13000:
```
xxd -s 0x13000 -l 64 kf10_ic17.bin
```

### Possible Fixes to Test

1. **Remove the >> 2 shift:**
   ```cpp
   rom_addr = offset & 0x1FFFF;
   ```

2. **Change shift amount:**
   ```cpp
   rom_addr = (offset >> 1) & 0x1FFFF;  // or >> 3, >> 4
   ```

3. **Use different bit arrangement:**
   The PHI might not be in the lower 12 bits - could be interleaved with WAVE

4. **Check the WF register format:**
   The 9-bit wf might need to be combined with PHI differently

## Correct Address Mapping (from SAM Programmer's Guide)

### Key Quote from Guide

> "PHI=0 ;lower part of memory address (upper 9 bits)"
> "PHIWF: Portion of phase recommended to access external waveforms (512 samples/period)"

### Correct 17-bit ROM Address Formula

```
ROM Address = (WAVE[7:0] << 9) | PHI[11:3]
            = WAVE * 512 + (PHI / 8)
```

**Mapping:**
- **ROM A[8:0]** ← PHI[11:3] (upper 9 bits of PHI, 512 sample positions)
- **ROM A[16:9]** ← WAVE[7:0] (8 bits, 256 waveform slots)
- **PHI[2:0]** are fractional bits (for interpolation, not used for addressing)

### Implementation in keyfox10.cpp

```cpp
u16 keyfox10_state::sam_snd_waveform_r(offs_t offset)
{
    // offset = 20-bit address from SAM8905: { WAVE[7:0], PHI[11:0] }
    // Per SAM programmer's guide: PHI upper 9 bits + WAVE 8 bits = 17-bit address
    uint32_t wave = (offset >> 12) & 0xFF;   // WAVE[7:0]
    uint32_t phi = offset & 0xFFF;           // PHI[11:0]
    uint32_t phi_int = (phi >> 3) & 0x1FF;   // PHI[11:3] - upper 9 bits
    uint32_t rom_addr = (wave << 9) | phi_int;  // 17-bit ROM address
    bool use_rhythm = BIT(offset, 19);       // WA19 selects ROM

    u8 sample = use_rhythm ? m_rhythms_rom[rom_addr] : m_samples_rom[rom_addr];
    return (int16_t)(int8_t)sample << 4;     // Sign-extend 8→12 bit
}
```

### Address Examples

| WAVE | PHI | ROM Address | Notes |
|------|-----|-------------|-------|
| 0x70 | 0x000 | 0x0E000 | Piano waveform start |
| 0x70 | 0x008 | 0x0E001 | Next sample (PHI += 8) |
| 0x70 | 0xFFF | 0x0E1FF | End of WAVE=0x70 range |
| 0x73 | 0x000 | 0x0E600 | Current piano WAVE |
| 0x73 | 0x018 | 0x0E603 | PHI=24 → sample 3 |

### ROM Data Verification

**WAVE=0x73 at ROM 0xE600:**
```
0000e600: 050a 0f15 1c25 2e36 3e44 484a 4c4c 4b47  // Valid waveform!
0000e610: 423d 3730 281f 170f 0a06 03fd f6ed e5de
```

This is smooth waveform data (sine-like pattern), confirming the address mapping is correct.

### Waveform Structure

Each WAVE slot = 1024 bytes (10 address bits from PHI[11:2])
- Piano uses WAVE=0x70 to 0x72 (finalWAVE=0x72=114)
- With word5 offset, actual WAVE read is 0x73
- Each waveform block contains 1024 samples

## Current Status

1. [x] Trace exact address calculation
2. [x] Found correct mapping from SAM8905 programmer's guide
3. [x] Verified ROM data at calculated addresses is valid waveform
4. [x] Implemented correct address mapping in keyfox10.cpp
5. [x] WA WSP implementation verified against truth table
6. [x] Sound plays correctly with 1024 samples/wave mapping
7. [ ] CPU envelope control for note-off decay (separate issue)

## Hardware Address Mapping (Final - Verified Working)

Per SAM8905 Programmer's Guide:
> "When the upper bit of wave is 0 [external], the remaining 8 bits together with
> the 12 upper phase bits define the external sampling memory address."

This gives 8 + 12 = 20-bit address. After hardware skips WA[1:0] (fractional):
- **WA19**: ROM chip select (WAVE[7]: 0 = Samples IC17, 1 = Rhythms IC14)
- **WA0-WA1**: Skipped (fractional PHI bits, not connected to ROM)
- **WA[11:2]** → ROM A[9:0] (10 address bits from PHI = 1024 samples/wave)
- **WA[18:12]** → ROM A[16:10] (7 address bits from WAVE = 128 wave slots per ROM)

### ROM Address Formula

```
ROM_ADDR[16:0] = (WAVE[6:0] << 10) | (PHI[11:2])
               = (WAVE * 1024) + (PHI / 4)
```

### Implementation in keyfox10.cpp

```cpp
u16 keyfox10_state::sam_snd_waveform_r(offs_t offset)
{
    // offset = 20-bit address from SAM8905: { WAVE[7:0], PHI[11:0] }
    // Per programmer's guide: 8 bits WAVE + 12 bits PHI = 20 bits
    // After skipping WA[1:0] (fractional): 17 bits for ROM
    // ROM = (WAVE[6:0] << 10) | PHI[11:2] = 7 + 10 = 17 bits
    uint32_t wave = (offset >> 12) & 0x7F;     // WAVE[6:0] - 7 bits
    uint32_t phi = offset & 0xFFF;             // PHI[11:0]
    uint32_t phi_int = (phi >> 2) & 0x3FF;     // PHI[11:2] - 10 bits
    uint32_t rom_addr = (wave << 10) | phi_int; // 17-bit ROM address
    bool use_rhythm = BIT(offset, 19);         // WA19 = WAVE[7] selects ROM
    ...
}
```

### Previous Incorrect Mappings

1. **`(offset >> 2) & 0x1FFFF`** - Combined shift, wrong ROM region
2. **`(WAVE << 9) | PHI[10:2]`** - 512 samples/wave, wrong pitch/loop
3. **`(WAVE << 10) | PHI[11:2]`** - ✓ Correct: 1024 samples/wave

## WA WSP Implementation (Verified Correct)

Per SAM8905 Programmer's Guide Section 8-3, WA WSP:
- Format: `E|WAVE|finalWAVE` on bus
- WAVE (bits 17:9) increments toward finalWAVE (bits 8:0)
- When WAVE == finalWAVE: stops incrementing, optionally triggers IRQ
- E bit (bit 18) controls CLEARRQST for interrupt
- Implementation in sam8905.cpp matches truth table ✓

## Carry Calculation (Fixed 2026-01-08)

### Key Finding: Carry from Bit 12, Not Bit 19

The SAM8905 adder's carry output comes from **bit 12**, not bit 19. This makes sense because:
- PHI (phase) is 12 bits (0x000-0xFFF)
- When PHI overflows (0xF8C + 0xC7 = 0x1053), carry=1
- This carry is used by WA WSP to increment WAVE toward finalWAVE

### Implementation

```cpp
auto update_carry = [&slot]() {
    bool b_neg = BIT(slot.b, 18);
    uint32_t sum = slot.a + slot.b;
    if (!b_neg)
        slot.carry = (sum > 0xFFF);  // Carry from bit 12 (PHI overflow)
    else
        slot.carry = !BIT(sum & MASK19, 18);  // For envelope: positive result
};
```

### Carry Updates Continuously

The carry must be updated whenever A or B changes (after WA, WB, clearB operations), not just during RADD. This is because the hardware adder always computes A+B and the carry output is always available.

### Verified Behavior

```
WA_WSP S2: A=0x00F8C B=0x000C7 wave=070 final=072 carry=1  ← PHI overflow!
WA_WSP S2: A=0x01053 B=0x000C7 wave=071 final=072 carry=1  ← WAVE incremented
WA_WSP S2: A=0x0111A B=0x000C7 wave=072 final=072 carry=1  ← WAVE reached final
```

WAVE successfully increments from 0x70 → 0x71 → 0x72 (finalWAVE), and IRQ latch is set.
