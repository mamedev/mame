# Keyfox1 Rack Input Panel Implementation Plan

## Overview

Implement button/LED panel emulation for the Keyfox10 using 74HC574 octal D-flip-flop shift registers in dual-mode operation (LED display and button sensing).

## Status: IMPLEMENTED

Started: 2026-01-11

## Hardware Architecture

### Shift Register Chain (10x 74HC574)

The panel uses 10x 74HC574 octal D-flip-flops in a cascaded chain:

```
CPU ──DATA──▶ IC8 ──▶ IC12 ──▶ IC13 ──▶ IC15 ──▶ IC14 ──▶ IC16 ──▶ IC17 ──▶ IC1 ──▶ IC3 ──▶ IC2
               │       │        │        │        │        │        │       │       │       │
               ▼       ▼        ▼        ▼        ▼        ▼        ▼       ▼       ▼       ▼
            8 btn/LED  8 btn/LED ... (80 buttons with LEDs total)
```

Total: 80 buttons with corresponding LEDs

### Control Signals (from Port 1)

| Signal    | Pin  | Direction | Description                                    |
|-----------|------|-----------|------------------------------------------------|
| DATA      | P1.6 | Out       | Serial data input to first shift register      |
| CLK_LED   | P1.2 | Out       | Clock signal - shifts data through chain       |
| ENABLE    | P1.4 | Out       | Latch enable - transfers shift register to outputs |
| MODE      | P1.5 | Out       | Mode control (HIGH = sense mode)               |
| SENSE     | P1.7 | In        | Combined sense input (NAND gate logic)          |

**Note:** CLK_SW (P1.0) is used for a different input section (keyboard matrix - to be documented separately).

### SENSE Signal Logic

The SENSE signal combines LED panel sense and keyboard matrix sense via NAND gates:

```
SENSE = NAND(NAND(MODE, LED_SENSE), SW_SENSE)
      = (MODE & LED_SENSE) | ~SW_SENSE
```

**Hardware details:**
- **Q2 (BC640 PNP)**: MODE controls transistor that powers LEDs
  - MODE=0: Q2 ON, +LED powered, LEDs can light
  - MODE=1: Q2 OFF, LEDs unpowered (required for sensing)
- **IC5 (74HC175) ~CLR**: Receives ~MODE
  - MODE=0: ~CLR=1, keyboard matrix shift registers enabled
  - MODE=1: ~CLR=0, keyboard matrix cleared (address = 0)
- **LED_SENSE**: Active-high (button connects shift register output to sense)
- **SW_SENSE**: Active-low (button pulls mux output low)

**Truth table:**
| MODE | LED_SENSE | SW_SENSE | SENSE |
|------|-----------|----------|-------|
| 0    | X         | HIGH     | 0     |
| 0    | X         | LOW      | 1     |
| 1    | 0         | HIGH     | 0     |
| 1    | 0         | LOW      | 1     |
| 1    | 1         | X        | 1     |

### Dual-Mode Operation

**LED Mode (MODE = LOW):**
- Shift register outputs drive LEDs directly
- 80-bit pattern shifted in via DATA + CLK_LED
- ENABLE latches pattern to outputs

**Button Sense Mode (MODE = HIGH):**
- Outputs float (tri-state), LEDs off
- Single '1' bit shifted through chain via DATA + CLK_LED
- If button pressed, SENSE sees the '1'
- Position of '1' identifies which button

```
Sense Sequence:
1. MODE = HIGH (outputs float)
2. Shift single '1' into chain via CLK_LED
3. For each bit position:
   a. Clock the '1' to next position (CLK_LED rising edge)
   b. Read SENSE (P1.7)
   c. If HIGH, button at this position is pressed
4. After 80 clocks, all buttons scanned
5. MODE = LOW, restore LED pattern
```

## Button Names by IC

### IC8 - Status (First in chain)
| Bit | Button Name |
|-----|-------------|
| 0   | TUNE        |
| 1   | TRANSP.     |
| 2   | PRES2       |
| 3   | PRES1       |
| 4   | MIDI        |
| 5   | RHYTHM      |
| 6   | LOWER       |
| 7   | UPPER       |

### IC12 - Upper Drawbars
| Bit | Button Name |
|-----|-------------|
| 0   | JAZZ1       |
| 1   | FLUTE 16    |
| 2   | JAZZ2       |
| 3   | FLUTE 8     |
| 4   | ORGAN       |
| 5   | FLUTE 4     |
| 6   | CHURCH      |
| 7   | PERC2 2/3   |

### IC13 - Upper Voices 1
| Bit | Button Name |
|-----|-------------|
| 0   | TROMBONE    |
| 1   | PIANO       |
| 2   | TRUMPET     |
| 3   | E-PIANO     |
| 4   | CLARINET    |
| 5   | HONKYTONK   |
| 6   | SAXOPHON    |
| 7   | VIBES       |

### IC15 - Upper Solo
| Bit | Button Name |
|-----|-------------|
| 0   | STRINGS     |
| 1   | HAW.GUIT    |
| 2   | VOCAL       |
| 3   | JAZZ.GUIT   |
| 4   | SYNTHE      |
| 5   | J.FLUTE     |
| 6   | MUSETTE     |
| 7   | SYN.BRASS   |

### IC14 - Upper Controls
| Bit | Button Name  |
|-----|--------------|
| 0   | WV.SLOW      |
| 1   | WV.FAST      |
| 2   | ENSEMBLE     |
| 3   | SUSTAIN      |
| 4   | PORTAMEN.    |
| 5   | SOLO         |
| 6   | WERSICHORD   |
| 7   | DYNAMIC      |

### IC16 - Bass Section
| Bit | Button Name |
|-----|-------------|
| 0   | BASS1       |
| 1   | BASS2       |
| 2   | SYN.BASS    |
| 3   | SLAP.BASS   |
| 4   | FLUTE 8     |
| 5   | FLUTE 4     |
| 6   | SUSTAIN     |
| 7   | AUTO. B.    |

### IC17 - Lower Section
| Bit | Button Name |
|-----|-------------|
| 0   | FLUTE 2     |
| 1   | PIANO       |
| 2   | E-PIANO     |
| 3   | STRINGS     |
| 4   | SYN.STR6    |
| 5   | GUITAR      |
| 6   | SUSTAIN     |
| 7   | DRUMS       |

### IC1 - Rhythm Section 1
| Bit | Button Name |
|-----|-------------|
| 0   | DISCO       |
| 1   | ROCK&ROLL   |
| 2   | BEAT 8      |
| 3   | FOXTROTT    |
| 4   | J.ROCK      |
| 5   | BIG BAND    |
| 6   | BALLAD      |
| 7   | SAMBA       |

### IC3 - Rhythm Section 2
| Bit | Button Name |
|-----|-------------|
| 0   | BOSA        |
| 1   | WALTZ       |
| 2   | BEGUIN      |
| 3   | MARCH       |
| 4   | 2nd BANK    |
| 5   | INTRO       |
| 6   | FILL        |
| 7   | SYNC-ST     |

### IC2 - Mode Controls (Last in chain)
| Bit | Button Name |
|-----|-------------|
| 0   | SPLIT       |
| 1   | SONG        |
| 2   | ACC. 3      |
| 3   | ACC. 2      |
| 4   | ACC. 1      |
| 5   | BASS        |
| 6   | RHYTHM      |
| 7   | REV.MODE    |

## Implementation Tasks

### 1. Add State Variables
- [x] 80-bit LED shift register state (10 bytes)
- [x] 80-bit button scan shift register (10 bytes)
- [x] 80-bit button state from inputs (10 bytes)

### 2. Implement Shift Register Logic
- [x] DATA + CLK_LED: Shift data into chain (led_shift_clock())
- [ ] ENABLE: Latch shift register to LED outputs (not yet needed)

### 3. Implement Button Sensing
- [x] SENSE read: Return button state at current scan position
- [x] Support for multiple simultaneous button presses

### 4. Create MAME Input Definitions
- [x] Define input ports for all 80 buttons (PANEL0-PANEL9)
- [x] Map first 8 buttons to keyboard keys 1-8 for testing
- [x] Group by panel section for organization

## Files Modified

1. `src/mame/wersi/keyfox10.cpp` - Added panel I/O handlers and state

## Testing Results

Firmware activity observed:
- CLK_LED pulses detected - LED pattern being shifted
- Both SW shift and LED shift occurring (need to remove SW shift for this panel)

## Notes

- The 74HC574 is edge-triggered (data latched on rising clock edge)
- Button matrix uses the shift register outputs as column select
- SENSE is active-high when button pressed
- CLK_SW is NOT used for this panel - it's for a separate keyboard matrix

## Open Questions

- [x] Which clock is used? → CLK_LED only for this panel
- [ ] Exact timing requirements for scan cycle
- [ ] How often does firmware scan buttons vs update LEDs?
