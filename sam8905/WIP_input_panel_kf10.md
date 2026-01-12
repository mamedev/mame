# Keyfox10 Input Panel - Correct Mapping (74HC299)

## Status: IN PROGRESS

Started: 2026-01-12

## Overview

The KF10 input panel uses 10x 74HC299 bidirectional shift registers for button scanning and LED control. This document replaces WIP_input_panel.md which was based on the KF1 rack schematic (74HC574).

## Hardware Architecture

### 74HC299 Operation

The 74HC299 is an 8-bit bidirectional shift register with parallel load capability.

**Pin configuration:**
- S1 tied HIGH
- S0 connected to ~MODE

| MODE | ~MODE (S0) | S1,S0 | Operation |
|------|------------|-------|-----------|
| 0    | 1          | 1,1   | Parallel load (from HI inputs -> all 1s) |
| 1    | 0          | 1,0   | Shift right |

### Sensing Sequence

1. **MODE=0**: Parallel load all 1s from tied-high inputs
2. **Switch to MODE=1**: Enter shift right mode
3. **Shift a 0 through the chain**: DATA=0, clock pulses
4. **Read SENSE**: When the 0 reaches a pressed button position, SENSE goes low
5. **Count clocks**: Position of detected button = clock count when SENSE went low

### Shift Register Chain (10x 74HC299)

```
DATA --> KF2:4 --> KF2:3 --> KF2:2 --> KF2:1 --> KF1:12 --> KF1:11 --> KF1:10 --> KF1:2 --> KF1:1 --> KF3:1 --> (output)
          IC0      IC1       IC2       IC3        IC4        IC5        IC6       IC7      IC8      IC9
```

Total: 80 bits (10 ICs x 8 bits)

### Control Signals

| Signal   | Pin  | Direction | Description                        |
|----------|------|-----------|-----------------------------------|
| DATA     | P1.6 | Out       | Serial data input to first SR     |
| CLK_LED  | P1.2 | Out       | Clock (shifts on rising edge)     |
| MODE     | P1.5 | Out       | 0=parallel load, 1=shift right    |
| SENSE    | P1.7 | In        | Output of last SR in chain        |

## Button Mapping

### IC0 - KF2:4 (Upper Solo Controls)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | PRESET1 | PRESET 1 |
| 1   | PRESET2 | PRESET 2 |
| 2   | REVMODE | REV.MODE |
| 3   | SOLO | SOLO |
| 4   | PORTAM | PORTAMENTO |
| 5   | WCHORD | WERSI-CHORD |
| 6   | SUSTU | SUSTAIN (upper) |
| 7   | MEMCARD | MEM-CARD |

### IC1 - KF2:3 (Upper Solo Voices 2)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | SYNBRS | SYNTHE-BRASS |
| 1   | MUSETT | MUSETTE |
| 2   | JZFLUT | JAZZ-FLUTE |
| 3   | SYNTHE | SYNTHE |
| 4   | JZGUIT | JAZZ-GUIT |
| 5   | VOCAL | VOCAL |
| 6   | HAWGUI | HAWAI-GUIT |
| 7   | STRING | STRINGS |

### IC2 - KF2:2 (Upper Solo Voices 1)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | VIBES | VIBES |
| 1   | SAX | SAXOPHON |
| 2   | HONKY | HONKY-TONK |
| 3   | CLARIN | CLARINET |
| 4   | EPIANO | E-PIANO |
| 5   | TRUMPT | TRUMPET |
| 6   | PIANO | PIANO |
| 7   | TRMBON | TROMBONE |

### IC3 - KF2:1 (Upper Drawbars)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | WVSLOW | WV.SLOW |
| 1   | ENSEMB | ENSEMBLE |
| 2   | WVFAST | WV.FAST |
| 3   | CHURCH | CHURCH |
| 4   | PERC23 | PERC 2/3 |
| 5   | ORGAN | FULL ORGAN |
| 6   | FLUT4U | FLUTE 4 |
| 7   | JAZZ2 | JAZZ 2 |

### IC4 - KF1:12 (Mixed: Upper + Lower controls)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | FLUT16 | FLUTE 16 |
| 1   | FLUT8U | FLUTE 8 |
| 2   | DYNAMI | DYNAMIC |
| 3   | JAZZ1 | JAZZ 1 |
| 4   | SUSTL | SUSTAIN (lower) |
| 5   | SPLIT | SPLIT |
| 6   | DRUMS | DRUMS |
| 7   | MEMCBL | MEM-CARD (B+L) |

### IC5 - KF1:11 (Lower Section)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | GUITAR | GUITAR |
| 1   | SYNSTR | SYNTHE STR |
| 2   | FLUT2 | FLUTE 2 |
| 3   | STRNGL | STRINGS |
| 4   | FLUT4L | FLUTE 4 |
| 5   | EPIANL | E-PIANO |
| 6   | FLUT8L | FLUTE 8 |
| 7   | PIANOL | PIANO |

### IC6 - KF1:10 (Bass Section)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | SUSTB | SUSTAIN (bass) |
| 1   | AUTOBS | AUTO BASS |
| 2   | SLAPBS | SLAP BASS |
| 3   | BASS2 | BASS 2 |
| 4   | SYNBS | SYNTHE BASS |
| 5   | BASS1 | BASS 1 |
| 6   | TRANSP | TRANSPOSE |
| 7   | MIDI | MIDI |

### IC7 - KF1:2 (Rhythm Section 2)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | BANK2 | 2ND BANK |
| 1   | TUNE | TUNE |
| 2   | MARCH | MARCH |
| 3   | MEMCRH | MEM-CARD (R) |
| 4   | WALTZ | WALTZ |
| 5   | BEGUIN | BEGUINE |
| 6   | SAMBA | SAMBA |
| 7   | BOSSA | BOSSA |

### IC8 - KF1:1 (Rhythm Section 1)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | RHYTHM | RHYTHM |
| 1   | BALLAD | BALLAD |
| 2   | BIGBND | BIG BAND |
| 3   | JZROCK | JAZZ ROCK |
| 4   | FOXTR | FOXTROT |
| 5   | BEAT8 | 8 BEAT |
| 6   | ROCKNR | ROCK'N'ROLL |
| 7   | DISCO | DISCO |

### IC9 - KF3:1 (Accompaniment Controls)

| Bit | Button | Panel Label |
|-----|--------|-------------|
| 0   | ACC3 | ACC. 3 |
| 1   | MEMCSG | MEM-CARD (S) |
| 2   | ACC2 | ACC. 2 |
| 3   | SYNCST | SYNC START |
| 4   | ACC1 | ACC. 1 |
| 5   | FILL | FILL |
| 6   | ACCBAS | ACC. BASS |
| 7   | INTRO | INTRO/END |

## Key Differences from KF1 (74HC574)

| Aspect | KF1 (74HC574) | KF10 (74HC299) |
|--------|---------------|----------------|
| Chip type | 74HC574 octal D-FF | 74HC299 bidirectional SR |
| LED latch | Separate latch (ENABLE) | No latch, direct output |
| Shift direction | Left (into LSB) | Right (into MSB) |
| Sense method | Shift 1, detect AND | Parallel load 1s, shift 0, detect output |
| MODE function | LED power only | Controls S0 (shift vs parallel load) |
| ENABLE signal | Latches to output | Not used for panel |

## Implementation Notes

### Shift Register Logic

```cpp
// 74HC299 shift right: MSB receives DATA, each bit shifts to next lower position
void keyfox10_state::panel_shift_clock()
{
    // Only shift when MODE=1 (shift right mode)
    if (!(m_port1 & P1_MODE))
        return;  // MODE=0 means parallel load mode, no shift on clock

    u8 data_in = (m_port1 & P1_DATA) ? 1 : 0;

    // Shift right through all 10 ICs
    // Carry from IC[n] bit 0 goes to IC[n+1] bit 7
    u8 carry = data_in;
    for (int i = 0; i < 10; i++)
    {
        u8 next_carry = m_panel_sr[i] & 0x01;  // Save LSB before shift
        m_panel_sr[i] = (m_panel_sr[i] >> 1) | (carry << 7);  // Shift right, carry into MSB
        carry = next_carry;
    }
}

// Parallel load when MODE changes to 0
void keyfox10_state::panel_parallel_load()
{
    // When MODE=0 (~MODE=1), S1=1, S0=1 = parallel load
    // Parallel inputs are tied high, so load all 1s
    for (int i = 0; i < 10; i++)
        m_panel_sr[i] = 0xff;
}
```

### SENSE Logic

```cpp
// SENSE is the output of the last shift register (IC9 bit 0)
// When a 0 reaches a pressed button position, SENSE goes low
u8 sense_bit = m_panel_sr[9] & 0x01;

// If button is pressed at this position, output is pulled low
// (This is simplified - actual hardware may have different logic)
```

## Files to Modify

1. `src/mame/wersi/keyfox10.cpp` - Input ports, shift logic
2. `src/mame/layout/keyfox10.lay` - Button labels
3. `sam8905/WIP_input_panel.md` - Mark as obsolete

## Testing

1. Press buttons in MAME and verify correct response
2. Check LED patterns match button presses
3. Verify 7-segment display shows correct values for different modes
