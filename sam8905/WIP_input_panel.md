# Keyfox10 Input Panel Implementation Plan

## Overview

Implement button/LED panel emulation for the Keyfox10 using 74HC574 octal D-flip-flop shift registers in dual-mode operation (LED display and button sensing).

## Status: IN PROGRESS

Started: 2026-01-11

## Hardware Architecture

### Shift Register Chain (74HC574)

The panel uses 9x 74HC574 octal D-flip-flops in a cascaded chain:

```
CPU ──SH/DATA──▶ IC12 ──▶ IC13 ──▶ IC15 ──▶ IC14 ──▶ IC16 ──▶ IC17 ──▶ IC1 ──▶ IC3 ──▶ IC2
                  │        │        │        │        │        │       │       │       │
                  ▼        ▼        ▼        ▼        ▼        ▼       ▼       ▼       ▼
               8 LEDs   8 LEDs   8 LEDs   8 LEDs   8 LEDs   8 LEDs  8 LEDs  8 LEDs  8 LEDs
               8 btns   8 btns   8 btns   8 btns   8 btns   8 btns  8 btns  8 btns  8 btns
```

Total: 72 buttons with corresponding LEDs

### Control Signals

| Signal    | Description                                           |
|-----------|-------------------------------------------------------|
| SH/DATA   | Serial data input to first shift register             |
| CLKLED    | Clock signal - shifts data through chain              |
| ENABLE    | Latch enable - transfers shift register to outputs    |
| OC        | Output Control - when HIGH, outputs float (tri-state) |
| CP        | Clock pulse for shift register                        |
| LEDSENSE  | Input to detect button press when outputs float       |

### Dual-Mode Operation

**LED Mode (OC = LOW):**
- Shift register outputs drive LEDs directly
- 72-bit pattern shifted in via SH/DATA + CLKLED
- ENABLE latches pattern to outputs

**Button Sense Mode (OC = HIGH):**
- Outputs float (tri-state), LEDs off
- Single '1' bit shifted through chain
- If button pressed, LEDSENSE sees the '1'
- Position of '1' identifies which button

```
Sense Sequence:
1. OC = HIGH (outputs float)
2. Shift single '1' into chain
3. For each bit position:
   a. Clock the '1' to next position
   b. Read LEDSENSE
   c. If HIGH, button at this position is pressed
4. After 72 clocks, all buttons scanned
5. OC = LOW, restore LED pattern
```

## Button Names by IC (Top-Down, Left-Right)

### IC12 - Upper Drawbars Section
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

### IC15 - Upper Solo Section
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

### IC2 - Mode Controls
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

### 1. Identify CPU I/O Addresses
- [ ] Find memory-mapped addresses for panel control signals
- [ ] Determine which port bits map to SH/DATA, CLKLED, ENABLE, OC
- [ ] Find LEDSENSE input address

### 2. Add State Variables
- [ ] 72-bit shift register state (9 bytes)
- [ ] 72-bit LED latch state (9 bytes)
- [ ] 72-bit button state (9 bytes)
- [ ] OC (output control) state
- [ ] Current scan position

### 3. Implement Shift Register Logic
- [ ] SH/DATA + CLKLED: Shift data into chain
- [ ] ENABLE: Latch shift register to LED outputs
- [ ] OC control: Switch between LED and sense modes

### 4. Implement Button Sensing
- [ ] LEDSENSE read: Return button state at current scan position
- [ ] Support for multiple simultaneous button presses

### 5. Create MAME Input Definitions
- [ ] Define input ports for all 72 buttons
- [ ] Map to keyboard keys for testing
- [ ] Group by panel section for organization

### 6. Add LED Output Visualization
- [ ] Create layout file for button panel display
- [ ] Map LED states to layout elements

## Files to Modify

1. `src/mame/wersi/keyfox10.cpp` - Add panel I/O handlers and state
2. Create layout file if visual representation needed

## Testing Plan

1. Verify shift register clocking works
2. Test LED pattern updates
3. Test button scanning detects presses
4. Test multiple simultaneous buttons
5. Compare with real hardware behavior if possible

## Notes

- The 74HC574 is edge-triggered (data latched on rising clock edge)
- OC is active-low for output enable on real chip, but schematic shows logic
- Button matrix uses the shift register outputs as column select
- LEDSENSE is likely active-high when button pressed

## Open Questions

- [ ] Exact timing requirements for scan cycle
- [ ] How often does firmware scan buttons vs update LEDs?
- [ ] Are there any buttons outside this shift register chain?
