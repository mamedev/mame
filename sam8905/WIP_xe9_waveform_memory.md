# XE9 External Waveform Memory Analysis

## Hardware Configuration

The XE9 has two identical sound generator sections (left and right side of keyboard),
each with its own CPU, SAM8905, and waveform ROM bank. They are NOT stereo channels
from a single SAM - they are independent synthesizer sections.

### ROM Banks (Per Section)
- 6× 27C040 (512KB each) = 3MB total per section
- Left section: XEL0-XEL5 (xel01.bin - xel51.bin)
- Right section: XER0-XER5 (xer01.bin - xer51.bin)
- Each ROM: 19 address bits (A0-A18), 8 data bits

### Address Decoding (from schematic)

**SAM8905 Output Address Format:**
```
WA[19:0] = { WAVE[7:0], PHI[11:0] }
         = 8 bits waveform + 12 bits phase
```

**ROM Address Generation:**
- WA3-WA11 → RA0-RA8 (9 bits = 512 samples per period)
- WA through 74LS174 latches → RA9-RA18 (10 more bits)
- Total: 19 bits per ROM chip

**Chip Select Decoding (74LS139):**
- U15A: WA18, WA19 → /CS0, /CS1, /CS2, /CS3
- U15B: Additional decoding → /CS4, /CS5

### RAM (SAM SRAM)
- 2× HY6264AJ (8KB SRAM each) = 16KB total
- WA1-WA13 → SRAM A0-A12 (13 bits = 8KB per chip)
- WA16 → chip select (selects which 8KB chip)
- /WCS = chip select, /WWE = write enable

**Address Latching (74HC174):**
- Latches capture upper address bits on /WCS (not clock)
- Possible two-stage addressing via consecutive WPHI:
  1. WPHI (bank) - load waveform/bank bits onto WA bus
  2. WWF - /WCS triggers, 74HC174 latches upper address
  3. WPHI (phase) - load phase bits for actual access

## MAME Implementation

### Simplified Model (First Pass)

For initial implementation, treat ROMs as linear space:

```cpp
// 6 ROMs × 512KB = 3MB linear space
// WA[19:17] = ROM select (0-5)
// WA[16:0] = address within ROM (128KB used)

u16 xe9_state::sam_waveform_r(offs_t offset, bool is_right)
{
    // offset = 20-bit WA address from SAM8905
    uint32_t rom_select = (offset >> 17) & 0x07;  // 3 bits = 0-7
    uint32_t rom_addr = offset & 0x1FFFF;         // 17 bits

    if (rom_select >= 6)
        return 0;  // Invalid bank

    // Select ROM bank
    const uint8_t *rom = is_right ? m_samples_r : m_samples_l;
    uint32_t linear_addr = (rom_select * 0x80000) + rom_addr;

    // Read 8-bit sample, sign-extend to 12-bit
    int8_t sample = rom[linear_addr];
    return (int16_t)sample << 4;
}
```

### ROM Definition

```cpp
ROM_REGION(0x300000, "samples_l", 0)  // 3MB left channel
ROM_LOAD("xel01.bin", 0x000000, 0x80000, CRC(...))
ROM_LOAD("xel11.bin", 0x080000, 0x80000, CRC(...))
ROM_LOAD("xel21.bin", 0x100000, 0x80000, CRC(...))
ROM_LOAD("xel31.bin", 0x180000, 0x80000, CRC(...))
ROM_LOAD("xel41.bin", 0x200000, 0x80000, CRC(...))
ROM_LOAD("xel51.bin", 0x280000, 0x80000, CRC(...))

ROM_REGION(0x300000, "samples_r", 0)  // 3MB right channel
ROM_LOAD("xer01.bin", 0x000000, 0x80000, CRC(...))
ROM_LOAD("xer11.bin", 0x080000, 0x80000, CRC(...))
ROM_LOAD("xer21.bin", 0x100000, 0x80000, CRC(...))
ROM_LOAD("xer31.bin", 0x180000, 0x80000, CRC(...))
ROM_LOAD("xer41.bin", 0x200000, 0x80000, CRC(...))
ROM_LOAD("xer51.bin", 0x280000, 0x80000, CRC(...))
```

## Reference: Keyfox10 Waveform Read

From keyfox10.cpp (2 ROMs × 128KB):
```cpp
u16 keyfox10_state::sam_snd_waveform_r(offs_t offset)
{
    uint32_t wave = (offset >> 12) & 0x7F;   // WAVE[6:0]
    uint32_t phi = offset & 0xFFF;           // PHI[11:0]
    uint32_t phi_int = (phi >> 2) & 0x3FF;   // PHI[11:2] = 10 bits
    uint32_t rom_addr = (wave << 10) | phi_int;
    bool use_rhythm = BIT(offset, 19);       // WA19 selects ROM

    u8 sample = use_rhythm ? m_rhythms_rom[rom_addr] : m_samples_rom[rom_addr];
    return (int16_t)(int8_t)sample << 4;
}
```

## Open Questions

1. Exact bit mapping from WA to RA for each ROM
2. How WA18-WA19 decode to all 6 chip selects (need /CS4, /CS5 logic)
3. Whether left/right channels are selected by SAM or are separate signals
4. RAM interpolation buffer behavior (not needed for basic sound)

## Files

- Schematic: Screenshot analyzed
- ROMs: hohner/ms4/xel0*.bin, xer0*.bin (6 each, 512KB each)
- Firmware: xe9l_v141_uart.bin, xe9r_v141_uart.bin (64KB each)
