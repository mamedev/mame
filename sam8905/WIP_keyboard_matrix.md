# Keyfox10 Keyboard Matrix (CLK_SW/SWSENSE)

## Overview

The CLK_SW section handles increment/decrement style buttons for level controls using a shift register + multiplexer architecture.

## Status: IMPLEMENTED

Started: 2026-01-11

## Hardware Architecture

### Components

```
DATA ──▶ IC5 ──▶ IC5 ──▶ IC5 ──▶ IC5    (4x 74HC175 quad D-flip-flops)
         │       │       │       │
         └───────┴───────┴───────┘
                 │
         SELA, SELB, SELC (3-bit select)
                 │
         ┌───────┴───────┐
         ▼               ▼
        IC4             IC11
     74HC251          74HC251
   (8:1 mux)        (8:1 mux)
         │               │
         └───────┬───────┘
                 ▼
             SWSENSE
```

### Shift Register Chain (4x 74HC175)

- **IC5** (x4) - 74HC175 Quad D-type flip-flops
- Clocked by **CLK_SW** (P1.0)
- DATA input from **P1.6**
- Forms a 16-bit shift register (4 chips × 4 bits each)
- Outputs provide SELA, SELB, SELC select lines for multiplexers

### Multiplexers (74HC251)

**IC4 - 74HC251** (8-to-1 multiplexer):
| Input | Switch | Function    |
|-------|--------|-------------|
| D0    | T9     | RHYTHM-     |
| D1    | T10    | RHYTHM+     |
| D2    | T33    | ACC+        |
| D3    | T32    | ACC-        |
| D4    | T55    | TEMPO+      |
| D5    | T54    | TEMPO-      |
| D6    | T73    | START/S     |
| D7    | -      | (unused?)   |

**IC11 - 74HC251** (8-to-1 multiplexer):
| Input | Switch | Function    |
|-------|--------|-------------|
| D0    | T12    | UPPER+      |
| D1    | T11    | UPPER-      |
| D2    | T34    | LOWER-      |
| D3    | T35    | LOWER+      |
| D4    | T59    | BASS-       |
| D5    | T60    | BASS+       |
| D6    | -      | (unused?)   |
| D7    | -      | (unused?)   |

### Control Signals

| Signal    | Pin  | Direction | Description                           |
|-----------|------|-----------|---------------------------------------|
| DATA      | P1.6 | Out       | Serial data to shift registers        |
| CLK_SW    | P1.0 | Out       | Clock for 74HC175 shift registers     |
| SWSENSE   | ?    | In        | Multiplexer output (selected switch)  |

### Operation

1. Firmware shifts a 3-bit address into the 74HC175 chain via CLK_SW
2. The address bits (SELA, SELB, SELC) select one of 8 inputs on each 74HC251
3. SWSENSE returns the state of the selected switch
4. Process repeats to scan all switches

### Button Functions

These are **increment/decrement** style buttons (not toggle):

**IC4 buttons:**
| Function  | Buttons | Purpose                    |
|-----------|---------|----------------------------|
| RHYTHM    | -/+     | Rhythm volume/level        |
| ACC       | +/-     | Accompaniment level        |
| TEMPO     | +/-     | Tempo adjustment           |
| START/S   | single  | Start/Stop rhythm          |

**IC11 buttons:**
| Function  | Buttons | Purpose                    |
|-----------|---------|----------------------------|
| UPPER     | +/-     | Upper manual level         |
| LOWER     | -/+     | Lower manual level         |
| BASS      | -/+     | Bass level                 |

Total: 13 buttons (7 on IC4, 6 on IC11)

## Additional Components

### IC9 - 74HC164 (Beat LED shift register)

- Clocked by **CLK_BEAT** (P1.1)
- 8-bit shift register for beat indicator LEDs
- Outputs QA-QH drive LEDs via diodes D1-D7
- SPLIT signal involved in routing
- Shows BEAT RIGHT/LEFT position

### Signals List (from schematic)

- MODE
- CLKSW (CLK_SW)
- DATA
- SWSENSE
- PEST
- PESUS
- CLKBEAT (CLK_BEAT)
- CLKLED (CLK_LED)
- ENABLE
- LEDSENSE (SENSE)
- CLKDISP (CLK_DISP)

## Implementation Tasks

### 1. Add State Variables
- [x] 74HC175 shift register state (16 bits = u16 m_kbd_sr)
- [x] Keyboard matrix button state (2x u8 m_kbd_state for IC4, IC11)

### 2. Implement CLK_SW Handler
- [x] kbd_shift_clock() - Shift DATA into 74HC175 chain on CLK_SW rising edge
- [x] Low 3 bits of shift register provide SELA, SELB, SELC

### 3. Implement SWSENSE Input
- [x] SWSENSE shares P1.7 (SENSE) with panel buttons
- [x] read_kbd_mux() returns selected switch state based on 3-bit address
- [x] IC4 and IC11 outputs OR'd together

### 4. Define Input Ports
- [x] KBDMTX0 (IC4): RHYTHM-/+, ACC+/-, TEMPO+/-, START/S (7 buttons)
- [x] KBDMTX1 (IC11): UPPER+/-, LOWER-/+, BASS-/+ (6 buttons)
- [x] Mapped to F1-F12 and EQUALS keys for testing

### 5. Implement Beat LED (CLK_BEAT)
- [x] IC9 - 74HC164 shift register for beat position (m_beat_sr)
- [x] beat_shift_clock() - CLK_BEAT rising edge handler
- [x] 8-bit shift register drives beat position LEDs via outputs QA-QH

## Open Questions (Resolved)

- [x] Which port bit is SWSENSE connected to? → Shares P1.7 (SENSE) with panel
- [ ] What are PEST and PESUS signals?
- [x] How are the two 74HC251 outputs combined into SWSENSE? → OR'd together
- [x] Exact bit mapping of SELA/SELB/SELC from shift register → Low 3 bits of m_kbd_sr
