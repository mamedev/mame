# Keyfox10 7-Segment Display

## Overview

The Keyfox10 has a 3-digit 7-segment LED display for showing tempo, preset numbers, or similar status information. The display uses 3x 74HC164 shift registers in a cascaded chain.

## Status: IMPLEMENTED

Started: 2026-01-11

## Hardware Architecture

### Display Chain
```
DATA (P1.6) ──▶ IC7 ──▶ IC6 ──▶ IC10
                │       │       │
                ▼       ▼       ▼
            DISPLAY1  DISPLAY2  DISPLAY3
             (left)   (middle)  (right)
              SE67F    SE67F     SE67F
```

### Components

- **IC7, IC6, IC10**: 74HC164 8-bit serial-in, parallel-out shift registers
- **DISPLAY1, DISPLAY2, DISPLAY3**: SE67F common-cathode 7-segment LED displays

### 74HC164 Output to Segment Mapping

| Output | Pin | Segment | Display Position |
|--------|-----|---------|------------------|
| QA     | 3   | a       | top              |
| QB     | 4   | b       | upper-right      |
| QC     | 5   | c       | lower-right      |
| QD     | 6   | d       | bottom           |
| QE     | 10  | e       | lower-left       |
| QF     | 11  | f       | upper-left       |
| QG     | 12  | g       | middle           |
| QH     | 13  | dp      | decimal point    |

### Control Signals

| Signal   | Port Pin | Direction | Description                        |
|----------|----------|-----------|-----------------------------------|
| DATA     | P1.6     | Out       | Serial data input to IC7          |
| CLK_DISP | P1.3     | Out       | Clock (shifts on rising edge)     |

### Shift Register Operation

1. Firmware sets DATA bit on P1.6
2. CLK_DISP rising edge shifts data through chain:
   - DATA → IC7 (DISPLAY1)
   - IC7.QH → IC6 (DISPLAY2)
   - IC6.QH → IC10 (DISPLAY3)
3. 24 clock pulses load all 3 displays
4. No latch needed - outputs update immediately on clock

### Active-Low Encoding

The hardware uses **active-low** segment encoding:
- Logic 0 = segment ON (LED lit)
- Logic 1 = segment OFF (LED dark)

Initial state: `0xFF` = all segments off

## Segment Layout
```
    ─── a ───
   │         │
   f         b
   │         │
    ─── g ───
   │         │
   e         c
   │         │
    ─── d ───  ● dp
```

## Implementation Details

### Existing Driver Code

The shift register logic is already implemented in `keyfox10.cpp`:

```cpp
// State variable
u8 m_disp_sr[3] = {0xff, 0xff, 0xff};  // Active low, init all off

// Shift function - called on CLK_DISP rising edge
void keyfox10_state::disp_shift_clock()
{
    u8 data_in = (m_port1 & P1_DATA) ? 1 : 0;
    u8 ic7_carry = BIT(m_disp_sr[0], 7);
    u8 ic6_carry = BIT(m_disp_sr[1], 7);

    m_disp_sr[2] = (m_disp_sr[2] << 1) | ic6_carry;  // IC10
    m_disp_sr[1] = (m_disp_sr[1] << 1) | ic7_carry;  // IC6
    m_disp_sr[0] = (m_disp_sr[0] << 1) | data_in;    // IC7
}
```

### Added for Visual Output

- `output_finder<3> m_digits` - MAME output for digit segments
- `output_finder<3> m_digit_dp` - MAME output for decimal points
- `update_display()` - Converts active-low to MAME format
- `keyfox10.lay` - Layout file for visual display

## Implementation Tasks

- [x] Document hardware architecture
- [x] Document segment mapping
- [x] Create layout file (keyfox10.lay)
- [x] Add output_finder to driver
- [x] Add update_display() function
- [x] Reference layout in machine config

## Files Modified

1. `src/mame/wersi/keyfox10.cpp` - Added display output support
2. `src/mame/layout/keyfox10.lay` - NEW layout file

## Testing Results

- Display appears in MAME window
- Firmware writes patterns to display shift registers
- Segments light up according to shifted data

## Notes

- The 74HC164 is a simple shift register with no output latch
- CLK_DISP clocks all three shift registers simultaneously via CP pin
- The clear (CLR) pins are active-low and appear tied high (not used)
- Display likely shows tempo (3 digits: 060-240 BPM) or preset number
