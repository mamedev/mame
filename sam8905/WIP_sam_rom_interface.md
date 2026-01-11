# SAM8905 External Sample ROM Interface

Analysis of the Keyfox10 SAM8905 to external sample ROM connections.

---

## Overview

The SAM8905 DSP supports external waveform memory for sample playback. The Keyfox10 uses two 128KB ROMs containing sample data for sounds that cannot be synthesized internally.

From the SAM8905 programmer's guide:
> "For application involving sampling, up to 1M words of 12 bits sampling memory (RAM or ROM) can be directly connected."

---

## Component List

| IC | Part | Function |
|----|------|----------|
| IC20 | SAM8905 | Sound Generator DSP |
| IC17 | 27C101 (128KB) | "Samples" ROM |
| IC14 | 27C101 (128KB) | "Rhythmen" ROM |
| IC11 | 74HC541 | Octal Buffer |
| IC10 | 74HC86 | XOR Gates |
| IC25 | 44.0MHz OSC | Master Clock |
| IC31 | 74HC14 | Schmitt Trigger Inverter |

---

## Signal Groups

### Waveform Address (SAM8905 Outputs)

| Signal | Width | Description |
|--------|-------|-------------|
| WA0-WA19 | 20 bits | Waveform Address from SAM8905 |
| WAS0-WAS18 | 19 bits | Buffered waveform address |

### Waveform Data (SAM8905 Inputs)

| Signal | Width | Description |
|--------|-------|-------------|
| MDS0-MDS11 | 12 bits | Sample data input to SAM8905 |

### Control Signals

| Signal | Description |
|--------|-------------|
| WWE | Write Waveform Enable (active low) |
| SAMOCS | SAM Output Chip Select |
| CLK2 | Clock signal |

---

## Address Path

### Address Composition

From the programmer's guide, external waveform addresses are composed of:

```
WA[19:0] = { WAVE[7:0], PHI[11:0] }
              8 bits     12 bits
           (waveform)   (phase)
```

- **WAVE[7:0]**: Selects which waveform (0-255)
- **PHI[11:0]**: Phase position within the waveform (0-4095 samples per period)

### ROM Address Mapping

The 27C101 ROMs have 17-bit addresses (128K × 8-bit):

```
ROM Address A[16:0] = WAS[18:2]
```

**Key observation**: WAS0 and WAS1 are NOT connected to ROM address lines.

This means:
- Only 17 address bits reach the ROMs
- The lower 2 bits (fractional phase) are handled differently
- Each ROM provides 128K × 8-bit samples

### Dual ROM Configuration

Both IC17 ("Samples") and IC14 ("Rhythmen") share the same address bus:
- Address: WAS2-WAS18 → A0-A16
- Data: D0-D7 → MDS4-MDS11

ROM selection appears to be via the upper WAVE bits or separate chip enables.

---

## Data Path

### ROM to SAM8905

```
ROM Data (8-bit)          SAM8905 Input (12-bit)
    D7  ─────────────────────► MDS11
    D6  ─────────────────────► MDS10
    D5  ─────────────────────► MDS9
    D4  ─────────────────────► MDS8
    D3  ─────────────────────► MDS7
    D2  ─────────────────────► MDS6
    D1  ─────────────────────► MDS5
    D0  ─────────────────────► MDS4
                               MDS3  ◄── ?
                               MDS2  ◄── ?
                               MDS1  ◄── ?
                               MDS0  ◄── ?
```

The ROM provides 8 bits, but SAM8905 expects 12-bit waveform data.

---

## 74HC541 Buffer Analysis (IC11)

The 74HC541 is an octal non-inverting buffer with 3-state outputs. It routes low address bits to the data bus.

### IC11 Pin Mapping (74HC541)

| Pin | Function | Signal In | Signal Out | Description |
|-----|----------|-----------|------------|-------------|
| 1   | OE1/     | -         | -          | Output Enable 1 (active low) |
| 2   | A1       | DGND      | -          | Input tied to ground |
| 3   | A2       | DGND      | -          | Input tied to ground |
| 4   | A3       | DGND      | -          | Input tied to ground |
| 5   | A4       | WAS0      | -          | Fractional phase bit 0 |
| 6   | A5       | WAS1      | -          | Fractional phase bit 1 |
| 7   | A6       | WAS2      | -          | Fractional phase bit 2 |
| 8   | A7       | WAS3      | -          | Fractional phase bit 3 |
| 9   | A8       | DGND      | -          | Input tied to ground |
| 10  | GND      | -         | -          | Ground |
| 11  | Y8       | -         | MDS11      | → SAM data bit 11 (=0) |
| 12  | Y7       | -         | MDS10      | → SAM data bit 10 |
| 13  | Y6       | -         | MDS9       | → SAM data bit 9 |
| 14  | Y5       | -         | MDS8       | → SAM data bit 8 |
| 15  | Y4       | -         | MDS7       | → SAM data bit 7 |
| 16  | Y3       | -         | MDS6       | → SAM data bit 6 (=0) |
| 17  | Y2       | -         | MDS5       | → SAM data bit 5 (=0) |
| 18  | Y1       | -         | MDS4       | → SAM data bit 4 (=0) |
| 19  | OE2/     | -         | -          | Output Enable 2 (active low) |
| 20  | VCC      | -         | -          | +5V |

### Address-to-Data Signal Mapping

| Address Input | 74HC541 Pin | Data Output | 74HC541 Pin | Purpose |
|---------------|-------------|-------------|-------------|---------|
| GND (0)       | A1 (pin 2)  | MDS4        | Y1 (pin 18) | Fixed 0 |
| GND (0)       | A2 (pin 3)  | MDS5        | Y2 (pin 17) | Fixed 0 |
| GND (0)       | A3 (pin 4)  | MDS6        | Y3 (pin 16) | Fixed 0 |
| WAS0          | A4 (pin 5)  | MDS7        | Y4 (pin 15) | Frac bit 0 |
| WAS1          | A5 (pin 6)  | MDS8        | Y5 (pin 14) | Frac bit 1 |
| WAS2          | A6 (pin 7)  | MDS9        | Y6 (pin 13) | Frac bit 2 |
| WAS3          | A7 (pin 8)  | MDS10       | Y7 (pin 12) | Frac bit 3 |
| GND (0)       | A8 (pin 9)  | MDS11       | Y8 (pin 11) | Fixed 0 |

### Signal Flow Diagram

```
                    74HC541 (IC11)
                 ┌─────────────────┐
     DGND ──────►│ A1          Y1 │──────► MDS4  (=0)
     DGND ──────►│ A2          Y2 │──────► MDS5  (=0)
     DGND ──────►│ A3          Y3 │──────► MDS6  (=0)
     WAS0 ──────►│ A4          Y4 │──────► MDS7  (frac[0])
     WAS1 ──────►│ A5          Y5 │──────► MDS8  (frac[1])
     WAS2 ──────►│ A6          Y6 │──────► MDS9  (frac[2])
     WAS3 ──────►│ A7          Y7 │──────► MDS10 (frac[3])
     DGND ──────►│ A8          Y8 │──────► MDS11 (=0)
                 └─────────────────┘
```

**Note**: Inputs A1-A3 and A8 are tied to ground, outputting 0 to MDS4-MDS6 and MDS11.
Inputs A4-A7 receive fractional address bits WAS0-WAS3, output to MDS7-MDS10.

### Purpose: Address Readback Register

The 74HC541 OE (Output Enable) pins are connected to SAM8905 **WWE (Write Waveform Enable)**.

**Bus control:**
- **WWE inactive (normal read)**: 74HC541 disabled (high-Z), ROMs drive MDS lines
- **WWE active**: 74HC541 enabled, outputs WAS[3:0] → MDS[10:7]

**Hypothesis: Temporary register for DSP computation**

When WWE is activated, the SAM8905 can read back its own low waveform address bits as a 12-bit value. This effectively provides a way to capture the current phase position into a DSP register (X, A, etc.) for later computation.

**Use case example:**
1. SAM outputs waveform address (WA[19:0])
2. Algorithm activates WWE via micro-instruction
3. SAM reads MDS[11:0] which now contains WAS[3:0] << 7
4. Value is loaded into X or A register
5. Can be used in subsequent multiply/add operations

This provides access to the low address bits (sub-sample position) without requiring extra hardware registers. The shifted position (bits 10:7) scales the 4-bit value into a useful range for 12-bit DSP math.

**Data format when 74HC541 active:**
```
MDS[11]   = 0 (fixed)
MDS[10:7] = WAS3-WAS0 (low address bits)
MDS[6:4]  = 0 (fixed)
MDS[3:0]  = ? (other source)
```

### Address to Data Value Table

Assuming MDS[3:0] = 0, the 74HC541 produces these 12-bit values:

| WAS[3:0] | MDS[11:0] Binary | MDS Hex | MDS Decimal |
|----------|------------------|---------|-------------|
| 0000     | 0000 0000 0000   | 0x000   | 0           |
| 0001     | 0000 1000 0000   | 0x080   | 128         |
| 0010     | 0001 0000 0000   | 0x100   | 256         |
| 0011     | 0001 1000 0000   | 0x180   | 384         |
| 0100     | 0010 0000 0000   | 0x200   | 512         |
| 0101     | 0010 1000 0000   | 0x280   | 640         |
| 0110     | 0011 0000 0000   | 0x300   | 768         |
| 0111     | 0011 1000 0000   | 0x380   | 896         |
| 1000     | 0100 0000 0000   | 0x400   | 1024        |
| 1001     | 0100 1000 0000   | 0x480   | 1152        |
| 1010     | 0101 0000 0000   | 0x500   | 1280        |
| 1011     | 0101 1000 0000   | 0x580   | 1408        |
| 1100     | 0110 0000 0000   | 0x600   | 1536        |
| 1101     | 0110 1000 0000   | 0x680   | 1664        |
| 1110     | 0111 0000 0000   | 0x700   | 1792        |
| 1111     | 0111 1000 0000   | 0x780   | 1920        |

**Formula:** `MDS = WAS[3:0] << 7` (address bits shifted left by 7)

**Question**: What controls the 74HC541 OE pins? This would clarify when this buffer is active vs the ROMs.

---

## WWE (Write Waveform Enable)

From programmer's guide Section 9:
> "Writing data to an external RAM is done also with a specific algorithm, but specific micro-instructions are needed to control the memory signals"

### Purpose

WWE enables writing to external waveform **RAM** (not applicable to ROM).

For the Keyfox10 with ROMs:
- WWE would be inactive during normal operation
- The write capability is unused
- Included for compatibility with RAM-based systems

### Write Timing (from guide)

When using external RAM, the write cycle is:
1. `RSP <ClearB,WSP>` - WOE/=1 (disable outputs)
2. `RSP <ClearB,WSP>` - Apply data to WD pins
3. `RSP <ClearB>` - WOE/=0 (write pulse)
4. `RSP <ClearB,WSP>` - WOE/=1

Complete write cycle: 443 ns

---

## Timing Considerations

### External Memory Access Time

From programmer's guide Section 6:
> "Availability of the wave sample (WWF or WPHI to WXY):
> External wave memory: (t_acc + 44.2)/44.2 cycles"

For 250ns access time: 7 cycles needed

### ROM Speed Requirements

The 27C101 ROMs likely have ~150-250ns access time, requiring:
- 5-7 NOP cycles between WPHI and WXY in algorithms
- This limits polyphony when using external samples

### Algorithm Example (from guide)

```
RM      WF,     <WWF>              ;WFreg=wave select
RM      PHI,    <WA,WPHI>          ;Areg=PHIreg=D-RAM(PHI)
RSP                                ;nops for memory access
RSP
RSP
RSP
RSP
RM      DPHI,   <WB>               ;Breg=D-RAM(DPHI)
RADD    PHI,    <WM>               ;D-RAM(PHI)= Areg + Breg
RM      AMP,    <WXY,WSP>          ;X=wave(PHI) Y=AMP
```

---

## Summary

The Keyfox10 external sample interface:

1. **Two 128KB ROMs** provide 8-bit PCM samples
2. **20-bit address** from SAM8905 maps to 17-bit ROM address
3. **Lower 2-4 address bits** routed to data bus via 74HC541
4. **Fractional bits** likely used for interpolation in DSP algorithms
5. **WWE signal** supports RAM writes but unused with ROMs
6. **7 cycle latency** for external memory access

This design allows efficient sample playback with interpolation support, maximizing quality while minimizing ROM bandwidth requirements.

---

*Document created: 2024-12-30*
*Source: Keyfox10 schematic analysis*
