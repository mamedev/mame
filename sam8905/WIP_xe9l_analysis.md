# WIP: XE9L Firmware Analysis

## Overview

Analysis of the XE9L ROM (xe9l_v141.bin, 64KB) focusing on MIDI handling and algorithm loading.

## Key Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x71A8 | serial_handler | Main MIDI input handler |
| 0x6C4E | midi_cc_dispatch | Control Change message dispatcher |
| 0x6C7C | algo_mode_dispatch | Algorithm loading mode handler |
| 0x5CB7 | load_algo_dispatch | Dispatches to specific algo loaders |
| 0x906B | load_algo_8d99 | Loads algorithms from 0x8D99 |
| 0x92E4 | load_algo_8e19 | Loads algorithms from 0x8E19 |
| 0x9278 | load_algo_8e99 | Loads algorithms from 0x8E99 |
| 0x931F | load_algo_8f19 | Loads algorithms from 0x8F19 |
| 0x99BC | load_algo_table_1 | Loads main algorithm table |
| 0x52C2 | load_algo | Base algorithm loading function |
| 0x8595 | cc_table_lookup | CC number to handler index lookup |
| 0x4023 | pitch_calc | Calculates pitch offsets using tables at 0x36C7/3747/37C7 |
| 0x5D3C | note_processor | Main note processing (velocity curves, voice allocation) |
| 0x6088 | all_notes_off | Turn off all notes for channel |
| 0x72B2 | midi_note_off_handler | Note Off (0x8x) / Note On (0x9x at 0x72B7) handler |
| 0xA031 | calc_extmem_addr | Calculate EXTMEM address from DPTR base + channel |
| 0x8F99 | delay_loop | Timing/delay loop |
| 0x8FBC | ram_test_init | RAM test and INIT() |
| 0x9147 | sam_memory_test | SAM D-RAM/A-RAM memory test |

## MIDI Message Flow

### serial_handler (0x71A8)

Main loop that processes MIDI input buffer. Uses jumptable at **0x7437** for status bytes:

| Index | Status | Target | Handler |
|-------|--------|--------|---------|
| 0 | 0x8x | 0x72B2 | Note Off |
| 1 | 0x9x | 0x72B7 | Note On |
| 2 | 0xAx | 0x72E8 | Poly Aftertouch |
| 3 | 0xBx | 0x72EB | Control Change |
| 4 | 0xCx | 0x7336 | Program Change |
| 5 | 0xDx | 0x734E | Channel Aftertouch |
| 6 | 0xEx | 0x7360 | Pitch Bend |
| 7 | 0xFx | 0x73AA | System (loops back) |

### Control Change Handler (0x72EB)

Sets up parameters and calls `midi_cc_dispatch`:
- `0x74` = CC number (from 0x79)
- `0x75` = Channel (from 0x7A)
- `0x76` = CC value (from 0x7B)

### midi_cc_dispatch (0x6C4E)

Looks up CC number in table at **0x8571** using FUN_CODE_8595 (XOR-based search), then uses jumptable at **0x73CB** for indices 0-4, or **0x73DA** for indices 5+.

#### CC Number Lookup Table (0x8571, 36 entries)

```
Index:  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35
CC#:   06  60  61  64  65  0C  0D  0E  0F  10  11  12  13  14  15  16  17  18  19  01  05  41  07  0A  0B  40  42  43  5C  7B  7C  7D  7E  7F  50  51
```

Key CC assignments:
- **CC 0x06** (index 0) → algo_mode_dispatch - Algorithm loading trigger
- **CC 0x64** (index 3) → set_mode_flag_2 - Sets 0x1CBD = value
- **CC 0x65** (index 4) → set_mode_flag_1 - Sets 0x1CBC = value
- CC 0x60 (index 1) → increment handler (0x6CE3)
- CC 0x61 (index 2) → decrement handler (0x6D2F)

#### Primary Jumptable (0x73CB, indices 0-4)

| Index | CC# | Target | Handler |
|-------|-----|--------|---------|
| 0 | 0x06 | 0x6C7C | algo_mode_dispatch |
| 1 | 0x60 | 0x6CE3 | increment handler |
| 2 | 0x61 | 0x6D2F | decrement handler |
| 3 | 0x64 | 0x6D79 | set_mode_flag_2 (0x1CBD = value) |
| 4 | 0x65 | 0x6D81 | set_mode_flag_1 (0x1CBC = value) |

If index > 4, uses secondary jumptable at **0x73DA** (indices 5+).

#### Secondary Jumptable (0x73DA, indices 5-35)

| Index | CC# | Target | Handler | Description |
|-------|-----|--------|---------|-------------|
| 5-12 | 0x0C-0x13 | 0x710E | no-op | Effect Control / General Purpose 1-4 (ignored) |
| 13 | 0x14 | 0x6DB0 | gp5_handler | Store to INTMEM[0xAA], set flag at 0xB6 |
| 14 | 0x15 | 0x6DCA | gp6_handler | Similar to above |
| 15 | 0x16 | 0x6DE4 | gp7_handler | Similar to above |
| 16 | 0x17 | 0x6DFE | gp8_handler | Similar to above |
| 17 | 0x18 | 0x6E18 | gp9_handler | Similar to above |
| 18 | 0x19 | 0x6E1B | gp10_handler | Store to EXTMEM |
| 19 | 0x01 | 0x6E1E | modulation | Store value to EXTMEM[0x1184+ch] |
| 20 | 0x05 | 0x6E2C | portamento_time | Store to EXTMEM[0x1CAC+ch] |
| 21 | 0x41 | 0x6E7A | portamento_switch | OFF: clear 0x11B4+ch,0x12C0+ch; ON: enable |
| 22 | 0x07 | 0x6EE0 | volume | Store to EXTMEM[0x1202+ch], scale for 0x78 |
| 23 | 0x0A | 0x6F34 | pan | Lookup table at 0x7451, store to EXTMEM[0x1203+ch] |
| 24 | 0x0B | 0x6F50 | expression | Curve lookup at 0x84F1, store to 0x120A+ch |
| 25 | 0x40 | 0x6F9E | sustain_pedal | Handle sustain state, may call 0x687B |
| 26 | 0x42 | 0x6FD8 | sostenuto | State at 0x1C9C+ch, calls 0x69A3/0x6B86 |
| 27 | 0x43 | 0x7015 | soft_pedal | Flag at 0x1C8C+ch (0=OFF, 1=ON) |
| 28 | 0x5C | 0x7037 | tremolo_depth | Store to 0x11D4, call tremolo_update (0x943D) |
| 29 | 0x7B | 0x705A | all_notes_off | Call 0x6088, exit |
| 30 | 0x7C | 0x7063 | omni_off | no-op (LJMP 0x710E) |
| 31 | 0x7D | 0x7066 | omni_on | no-op (LJMP 0x710E) |
| 32 | 0x7E | 0x7069 | mono_on | All-notes-off + set EXTMEM[0x1F5E+ch]=1 |
| 33 | 0x7F | 0x707D | poly_on | All-notes-off + set EXTMEM[0x1F5E+ch]=0 |
| 34 | 0x50 | 0x7090 | gp5_msb | General Purpose 5 MSB handler |
| 35 | 0x51 | 0x70C8 | gp6_msb | General Purpose 6 MSB handler |

**Note**: 0x710E is just a RET instruction - the common exit point.

### Increment/Decrement Handlers (0x6CE3, 0x6D2F)

CC 0x60 (Data Increment) and CC 0x61 (Data Decrement) behavior depends on mode flags:

| Mode [0x1CBC, 0x1CBD] | Target | Range | Step | Action |
|-----------------------|--------|-------|------|--------|
| [0x00, 0x00] | EXTMEM[0x11E5] | 0-12 | ±1 | Program/bank select |
| [0x00, 0x01] | EXTMEM[0x1CBE] | 0x80-0x7E | ±2 | Transpose, calls pitch_calc (0x4023) |

The mode [0x00, 0x01] is set by sending CC 0x64 value=0x01 (with CC 0x65 value=0x00), enabling transpose adjustment via increment/decrement.

### algo_mode_dispatch (0x6C7C)

Checks mode flags in EXTMEM and dispatches accordingly:

| Mode [0x1CBC, 0x1CBD] | Action |
|-----------------------|--------|
| [0x00, 0x00] | Store clamped value (max 0x0C) to EXTMEM 0x11E5 |
| [0x00, 0x01] | Calculate `(value - 0x40) * 2`, call FUN_CODE_4023 |
| [0x7B, 0x13] | If value < 8, call load_algo_dispatch(value) |

## Algorithm Loading Trigger Sequence

To trigger algorithm loading via the parallel interface, send the following CC sequence:

```
1. CC 0x65 value=0x7B  → Sets mode flag 0x1CBC = 0x7B
2. CC 0x64 value=0x13  → Sets mode flag 0x1CBD = 0x13
3. CC 0x06 value=N     → Triggers load_algo_dispatch with index N (0-7)
```

**Index to algorithm loader mapping:**

| Index N | Function | Description |
|---------|----------|-------------|
| 0 | (no-op) | Just exits |
| 1 | delay_loop | Timing delay |
| 2 | ram_test_init | RAM test + INIT() |
| 3 | sam_memory_test | SAM memory test |
| 4 | load_algo_8d99 | Load algos from 0x8D99 |
| 5 | load_algo_8e99 | Load algos from 0x8E99 |
| 6 | load_algo_8f19 | Load algos from 0x8F19 |
| 7 | load_algo_8e19 | Load algos from 0x8E19 |

**Note**: Mode flags are reset after use by algo_mode_dispatch, so the full sequence must be sent each time.

**RPN/NRPN Pattern**: This follows standard MIDI RPN (Registered Parameter Number) convention:
- CC 0x65 (RPN MSB) sets the parameter "bank" (0x7B = algorithm control)
- CC 0x64 (RPN LSB) sets the sub-parameter (0x13 = algorithm loading mode)
- CC 0x06 (Data Entry MSB) writes the value and triggers the action

### load_algo_dispatch (0x5CB7)

Disables EX0, then uses jumptable at **0x73B3** with index from 0x6F:

```asm
CODE:5cb7: C2A8        CLR 0xa8            ; Disable EX0
CODE:5cb9: E56F        MOV A,0x6f          ; Load index
CODE:5cbb: 9073B3      MOV DPTR,#0x73b3    ; Jumptable base
CODE:5cbe: F8          MOV R0,A
CODE:5cbf: 28          ADD A,R0            ; A = A * 2
CODE:5cc0: 28          ADD A,R0            ; A = A * 3
CODE:5cc1: 73          JMP @A+DPTR
```

| Index | Target | Function | Description |
|-------|--------|----------|-------------|
| 0 | 0x5CC2 | (no-op) | Just exits |
| 1 | 0x8F99 | delay_loop | Timing delay |
| 2 | 0x8FBC | ram_test_init | RAM test + INIT() |
| 3 | 0x9147 | sam_memory_test | SAM memory test |
| 4 | 0x906B | load_algo_8d99 | Load algos from 0x8D99 |
| 5 | 0x9278 | load_algo_8e99 | Load algos from 0x8E99 |
| 6 | 0x931F | load_algo_8f19 | Load algos from 0x8F19 |
| 7 | 0x92E4 | load_algo_8e19 | Load algos from 0x8E19 |

## Note Processing

### note_processor (0x5D3C)

Main note handling function called from `midi_note_off_handler` (0x72B2) for both Note Off and Note On messages.

**Velocity Curve Lookup:**
Uses table at 0x7474 with threshold values:
```
26 2E 36 3B 43 48 4F 54 FF (decimal: 38, 46, 54, 59, 67, 72, 79, 84, 255)
```

The function loops through thresholds, comparing input velocity until it exceeds a threshold. This maps continuous MIDI velocity (0-127) to discrete dynamic levels for stepped response curves.

**Key operations:**
- Checks sustain/mono flags at EXTMEM[0x1F5E+ch]
- Voice allocation tracking at EXTMEM[0x12E0+ch]
- Stores note data to EXTMEM[0x1200-0x1207] region
- Calls `pitch_calc` (indirectly via 0x4D95) for pitch processing

## Algorithm Tables

### Algorithms at 0x8D99 (loaded by load_algo_8d99)

8 algorithms × 32 instructions each at 44kHz:

| Index | ROM Address | A-RAM Slot | Active Instrs |
|-------|-------------|------------|---------------|
| 0 | 0x8D99 | 0x00 | ~18 |
| 1 | 0x8DD9 | 0x20 | ~8 |
| 2 | 0x8E19 | 0x40 | 0 (all NOP) |
| 3 | 0x8E59 | 0x60 | ~7 |
| 4 | 0x8E99 | 0x80 | 0 (all NOP) |
| 5 | 0x8ED9 | 0xA0 | ~9 |
| 6 | 0x8F19 | 0xC0 | 0 (all NOP) |
| 7 | 0x8F59 | 0xE0 | ~20 |

**Note**: The first two algorithms (slots 0x00 + 0x20) may use a register-passing trick
where slot 0 leaves intermediate values in A/B registers for slot 1 to continue processing.

### Main Algorithm Table (load_algo_table_1 at 0x99BC)

Loads 8 algorithms from scattered ROM addresses into A-RAM slots:

| Slot | ROM Address | Description |
|------|-------------|-------------|
| 0x00 | 0x871A | Percussion/rhythm |
| 0x20 | 0x86DA | Percussion/rhythm |
| 0x40 | 0x00EA | CLAVINET |
| 0x60 | 0x00AA | Accordion variants |
| 0x80 | 0x006A | Main instruments |
| 0xA0 | 0x002A | Bass/piano |
| 0xC0 | 0x3503 | Percussion/rhythm |
| 0xE0 | 0x9AFF | Percussion/rhythm |

## Jumptables Summary

| Address | Size | Used By | Purpose |
|---------|------|---------|---------|
| 0x7437 | 8×3 | serial_handler | MIDI status byte dispatch |
| 0x73CB | 5×3 | midi_cc_dispatch | CC handler dispatch (indices 0-4) |
| 0x73DA | 31×3 | midi_cc_dispatch | CC handler dispatch (indices 5-35) |
| 0x73B3 | 8×3 | load_algo_dispatch | Algorithm/test dispatch |
| 0x8571 | 36 bytes | cc_table_lookup | CC number → handler index lookup |
| 0x84F1 | 128 bytes | Expression handler | Expression curve lookup |
| 0x7451 | ~32 bytes | Pan handler | Pan value conversion |
| 0x7474 | 16+ bytes | note_processor | Velocity curve thresholds (38,46,54,59,67,72,79,84,255) |
| 0x9875 | varies | tremolo_update | Tremolo depth jumptable |

Jumptables use 3-byte LJMP entries (02 HH LL format). Lookup tables are direct byte mapping.

## Memory Locations

### Internal RAM (INTMEM)

| Address | Purpose |
|---------|---------|
| 0x6F | Channel / Algorithm index |
| 0x70 | Note number |
| 0x71 | Note on/off flag |
| 0x72 | Voice/slot index |
| 0x73 | Velocity curve index (loop counter) |
| 0x74 | CC number |
| 0x75 | MIDI channel |
| 0x76 | CC value / algorithm parameter |
| 0x77 | Processed CC value |
| 0x79 | First data byte |
| 0x7A | Second data byte |
| 0x7B | Third data byte |
| 0xAA | General purpose CC storage 1 |
| 0xAB | General purpose CC storage 2 |
| 0xB6 | CC change flag |
| 0xB7 | MIDI status byte |
| 0xB8 | Input buffer fill count |
| 0xBA | Input buffer read index |
| 0xBB | Current input byte |

### External RAM (EXTMEM)

| Address | Purpose |
|---------|---------|
| 0x1000+n | Pitch offset LSB (128 entries, written by pitch_calc) |
| 0x1080+n | Pitch offset MSB (128 entries, written by pitch_calc) |
| 0x1100+n | Pitch octave/fine (128 entries, written by pitch_calc) |
| 0x1184+ch | Modulation wheel value (CC 0x01) |
| 0x11E5 | Algorithm/mode value storage |
| 0x1202+ch | Volume value (CC 0x07) |
| 0x1203+ch | Pan value (CC 0x0A) |
| 0x1361 | Input buffer base |
| 0x11B4+ch | Portamento enable flag |
| 0x11D4 | Tremolo depth value (CC 0x5C) |
| 0x1180-0x118B | Tremolo state variables |
| 0x1200-0x1207+ch | Note data block (pitch, velocity, voice state) |
| 0x120A+ch | Expression value (CC 0x0B) |
| 0x12D0+ch | Voice state |
| 0x12E0+ch | Voice allocation count |
| 0x12C0+ch | Portamento state |
| 0x1C8C+ch | Soft pedal flag (CC 0x43) |
| 0x1C9C+ch | Sostenuto state (CC 0x42) |
| 0x1CAC+ch | Portamento time (CC 0x05) |
| 0x1CBC | Mode flag 1 (RPN MSB value) |
| 0x1CBD | Mode flag 2 (RPN LSB value) |
| 0x1CBE | Calculated parameter storage |
| 0x1F5E+ch | Sustain pedal state / Mono-Poly mode flag |

## Open Questions

- [x] What sets mode flags [0x1CBC, 0x1CBD] to [0x7B, 0x13]?
  - **RESOLVED**: CC 0x65 (index 4) sets 0x1CBC, CC 0x64 (index 3) sets 0x1CBD
- [x] What are the functions at 0x6CE3, 0x6D2F, 0x6D79, 0x6D81?
  - **RESOLVED**: 0x6CE3=increment, 0x6D2F=decrement, 0x6D79=set_mode_flag_2, 0x6D81=set_mode_flag_1
- [x] What triggers load_algo_8e19, load_algo_8e99, load_algo_8f19 vs load_algo_8d99?
  - **RESOLVED**: CC 0x06 value 4-7 selects which loader to call (see Algorithm Loading Trigger Sequence)
- [ ] Is there a SysEx handler that enables algorithm loading mode?
  - Likely not needed - mode is set via CC 0x64/0x65 sequence
- [x] What do CC 0x60 (increment) and 0x61 (decrement) handlers actually modify?
  - **RESOLVED**: Depends on mode flags:
    - Mode [0x00, 0x00]: Inc/dec EXTMEM[0x11E5] (range 0-12, likely program/bank select)
    - Mode [0x00, 0x01]: Inc/dec EXTMEM[0x1CBE] by 2 (range 0x80-0x7E signed), then calls pitch_calc (0x4023)
- [x] What are the handlers in the secondary jumptable at 0x73DA (indices 5-35)?
  - **RESOLVED**: See Secondary Jumptable section - handles Volume, Pan, Modulation, Sustain, All Notes Off, Mono/Poly modes, etc.

## Related Files

- `notebooks/xe9l_algo_table_2.ipynb` - Analysis of algorithms at 0x8D99
- `notebooks/xe9l_unused_algorithms.ipynb` - Analysis of percussion algorithms
- `parse_programs_xe9l.py` - Program and drum sound parser
