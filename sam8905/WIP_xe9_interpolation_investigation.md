# XE9 WWE / 74HC244 Interpolation Investigation

Date: 2026-01-27

## Objective

Investigate whether the XE9 v141 firmware uses the WWE (Write Waveform External) mechanism
to read interpolation coefficients from the 74HC244 buffer on the sound board.

## Background: WWE Trigger Mechanism

Per the SAM8905 programming guide, the /WWE signal is activated by a 3-instruction
clearB sequence in the A-RAM microcode:

```
PC N:   <clearB, WSP>    -- first clearB with WSP set
PC N+1: <clearB>         -- second clearB
PC N+2: <clearB>         -- third clearB: /WWE goes LOW here
```

/WWE going LOW puts the SAM8905 data bus (WD[0:11]) into high-impedance state.
This allows external hardware to drive the bus. On the XE9 board, a 74HC244 buffer
is connected to drive PHI[0:2] onto WD[0:2] when /WWE is low AND WA16=0 (ROM selected).

The purpose is **waveform interpolation**: the ROM address starts at WA3 (skipping
WA0-WA2), so each waveform period has 512 address steps but 4096 phase steps.
The low 3 bits of PHI represent the fractional position between ROM samples.
By reading PHI[0:2] back via the 74HC244 during a /WWE cycle, the algorithm can
compute a weighted average between adjacent ROM samples for smoother playback.

## Investigation Method

### Tools Used
- **Ghidra** (ports 8194=XE9L, 8195=MS4, 8197=XE9R) for firmware structure analysis
- **Jupyter notebook** (`sam8905/notebooks/xe9_analysis.ipynb`) for binary scanning
- **MS4 Ghidra project** as reference (better annotated, same program format)

### Step 1: Identify Program and Algorithm Tables

Used MS4 (port 8195) as the reference model since it's better annotated in Ghidra:

- MS4 `handle_program_change` at CODE:c45b reads a pointer table at CODE:0040
  (66 entries, 2-byte big-endian)
- MS4 `voice_assign_algorithm` at CODE:b4bf reads the A-RAM data pointer from
  program offset 10-11 (little-endian)
- Program format (shared across MS4/XE9):
  - Offset 0-7: ASCII name (space-padded)
  - Offset 8: null terminator
  - Offset 9: flags (bit 7 = complex init, bits 3:0 = SAM slot count)
  - Offset 10-11: A-RAM data pointer (little-endian)
  - Offset 12+: voice init / D-RAM data

For XE9L, Ghidra string search found 29 program name strings. By searching the ROM
for big-endian pointer representations of these known addresses, found:

- **Primary table** at 0x33AD: 128 entries, 2-byte big-endian
- **Secondary table** at 0x34AF: 42 entries (alternate/high-octave patches)

For XE9R, used the same program format to scan the ROM:

- **111 programs** found via pattern matching (8-byte ASCII + null + valid flags)
- Program pointer table location not directly identified (different firmware layout)

### Step 2: Extract and Decode Algorithms

**A-RAM algorithm format**: 32 words × 15 bits, stored as little-endian 16-bit values
at fixed ROM addresses. Standard positions: 0x002A, 0x006A, 0x00AA, 0x00EA.

#### XE9L Algorithms (4 found)

| Address | Active Instructions | Used By | clearB Count |
|---------|-------------------|---------|-------------|
| 0x002A  | 28                | 3 programs (EPIANO2, B16_8, B8) | 0 |
| 0x006A  | 29                | 24 programs (BDCLA0, DXPIA1, HARPSI1...) | 0 |
| 0x00AA  | 30                | 4 programs (ACCA2, MUSETC3, SPACEH, BANJO) | 1 single |
| 0x00EA  | 30                | 1 program (CLAVINET) | 0 |

XE9L algorithm 0x006A matches MS4 algorithm at 0x02BA.
XE9L algorithm 0x00AA matches MS4 algorithm at 0x027A.

#### XE9R Algorithms (3 found)

| Address | Active Instructions | Used By | clearB Count |
|---------|-------------------|---------|-------------|
| 0x002A  | 29                | 71 programs | 0 |
| 0x006A  | 30                | 34 programs | 1 single |
| 0x00AA  | 30                | 4 programs | 0 |

All 3 XE9R algorithms are **completely different** from XE9L algorithms.
XE9R data at 0x00EA is NOT a valid algorithm (values exceed 15-bit range).

**XE9R has no program names in common with XE9L** -- they contain entirely
separate instrument sets (left and right keyboard sections are independent synths).

Additional XE9R A-RAM pointer anomalies:
- `rflute` → 0x31F7: valid-looking algorithm data at non-standard address
- `RFLUTE` → 0x0063: points into middle of algorithm at 0x006A (invalid offset)

### Step 3: Search for clearB Sequences

Searched for the WWE trigger pattern: 3+ consecutive instructions with clearB
(bit 2 = 0) active.

**XE9L results**: Zero clearB sequences of length >= 2 across all 4 algorithms.
Only algorithm 0x00AA has a single isolated clearB instruction (PC01: `RSP, <clearB>`).

**XE9R results**: Zero clearB sequences of length >= 2 across all 3 valid algorithms.
Only algorithm 0x006A has a single isolated clearB instruction (PC01: `RSP, <clearB>`).

### Step 4: Full ROM Scan

Scanned both entire 64KB ROMs for ANY 64-byte block that:
1. Contains only valid 15-bit values (all words <= 0x7FFF)
2. Ends with at least one 0x7FFF NOP
3. Has at least 10 active (non-NOP) instructions
4. Contains 3+ consecutive clearB instructions

**Result: No matching blocks found in either XE9L or XE9R.**

Also performed a raw byte scan (ignoring the algorithm-block constraints):
- XE9L: longest consecutive clearB run = 231 words at 0x7816 (not valid algorithm data)
- XE9R: longest consecutive clearB run = 152 words at 0x750C (not valid algorithm data)

These long runs occur in ROM data regions where bit 2 happens to be 0 by coincidence --
the data is not valid A-RAM instructions.

## Findings

### Neither XE9L nor XE9R v141 firmware uses the WWE/74HC244 interpolation mechanism.

The hardware exists on the XE9 board:
- 74HC244 buffer connected to drive PHI[0:2] onto WD[0:2]
- /WWE and WA16 chip select logic present in schematic
- SRAM with /WWE write enable

But the firmware's algorithms never trigger /WWE because they never execute
3 consecutive clearB instructions with WSP on the first.

### Possible Explanations

1. **Reserved for future firmware versions**: The XE9 hardware was designed to support
   interpolation, but v141 firmware doesn't use it. A later firmware revision may have
   added algorithms with WWE.

2. **Used by a different product**: The XE9 PCB may be shared with another Hohner/Wersi
   product that does use interpolation (the same SAM8905 + 74HC244 circuit exists on
   multiple boards).

3. **SRAM-only usage**: The /WWE signal is also the SRAM write enable. The firmware may
   use SRAM writes (which don't require the 3-clearB sequence -- that's specifically
   for reading the interpolation coefficient). However, no SRAM write patterns were
   found either.

## Note: sam8905.cpp Implementation Considerations

### WWE Control Mechanism (from Prog Guide Section 9)

The programming guide clarifies that WSP during clearB **directly controls /WWE level**:

- `clearB + WSP` → /WWE=1 (HIGH, inactive): SAM drives WD bus with data
- `clearB` (no WSP) → /WWE=0 (LOW, active): write pulse / WD bus high-impedance
- No clearB → /WWE returns to 0 (normal operation)

This is NOT a multi-instruction state machine -- WSP acts as direct polarity control.

**SRAM write sequence** (from prog guide Section 9):
```
RM      WF,     <WWF>              ;activate memory chip select (WCS/)
RM      PHI,    <WPHI>             ;PHIreg=PHI
RM      DATA,   <WXY>              ;prepare to output data on WD pins
RSP             <ClearB,WSP>       ;WOE/=1 (disable outputs from external mem)
RSP             <ClearB,WSP>       ;WOE/=1, apply data to WD pins
RSP             <ClearB>           ;WOE/=0 (write pulse)
RSP             <ClearB,WSP>       ;WOE/=1
RSP             <ClearB,WSP>       ;WOE/=1 data still applied
                                   ;data removed
                                   ;WOE/=0 (back to normal operation)
```

Key observation: it is the **second** `<clearB,WSP>` that applies SAM Y data onto
the WD bus. The first `<clearB,WSP>` only disables external memory outputs.
The transition from `<clearB,WSP>` to `<clearB>` (without WSP) drives /WWE LOW.

**Interpolation readback** (74HC244 on XE9 board) -- just 2 instructions:
```
RSP             <clearB,WSP>       ;/WWE=1, ext mem outputs disabled, bus quiet
RSP             <clearB,WXY>       ;/WWE=0, SAM never drove WD (no 2nd clearB+WSP)
                                   ;74HC244 drives PHI[0:2] onto WD[0:2]
                                   ;WXY latches into X = interpolation coefficient
```

By skipping the second `<clearB,WSP>` (which is what would put SAM data on the bus),
the WD bus stays undriven from the SAM side. When /WWE then goes LOW on the next
`<clearB>` (here combined with WXY), the 74HC244 can freely drive PHI[0:2] onto
WD[0:2] without bus contention. WXY on that same cycle latches the result into X.

PHI[0:2] represents the fractional phase between ROM samples (ROM addressing
starts at WA3, so each period has 512 address steps but 4096 phase steps --
the low 3 bits are the sub-sample fraction for interpolation).

### Current sam8905.cpp Implementation

The current implementation triggers WWE on `emitter_sel == 3 && wsp` (single
instruction check). This needs to be corrected:

1. During clearB: check WSP to determine /WWE level
   - WSP=1 → /WWE=1 (SAM drives WD)
   - WSP=0 → /WWE=0 (write pulse or high-Z for readback)
2. When /WWE=0 and WXY is active on the same instruction,
   `get_waveform()` should return the 74HC244 output (`m_phi & 0x07`)
   instead of normal ROM data
3. Machine driver should signal whether a 74HC244 is present
   (Keyfox10 and MS4 do not have one; XE9 does)

## Summary Tables

### XE9L Program Distribution

| A-RAM Ptr | Algorithm | Programs (main table) | Programs (alt table) |
|-----------|-----------|----------------------|---------------------|
| 0x002A    | 28 active | 3 | 0 |
| 0x006A    | 29 active | 24 | 25 |
| 0x00AA    | 30 active | 4 | 17 |
| 0x00EA    | 30 active | 1 | 0 |

### XE9R Program Distribution

| A-RAM Ptr | Algorithm | Programs |
|-----------|-----------|----------|
| 0x002A    | 29 active | 71 |
| 0x006A    | 30 active | 34 |
| 0x00AA    | 30 active | 4 |
| 0x0063    | (invalid) | 1 (RFLUTE - points mid-algorithm) |
| 0x31F7    | non-standard | 1 (rflute - valid data) |

## Files

- Notebook: `sam8905/notebooks/xe9_analysis.ipynb`
- XE9 waveform docs: `sam8905/WIP_xe9_waveform_memory.md`
- SAM8905 emulation: `src/devices/sound/sam8905.cpp` (lines 340-359)
- XE9L firmware: `xe9l_v141.bin` (64KB)
- XE9R firmware: `xe9r_v141.bin` (64KB)
