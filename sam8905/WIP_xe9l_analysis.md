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

Jumptables use 3-byte LJMP entries (02 HH LL format). Lookup table is direct byte mapping.

## Memory Locations

### Internal RAM (INTMEM)

| Address | Purpose |
|---------|---------|
| 0x6F | Algorithm index for load_algo_dispatch |
| 0x74 | CC number |
| 0x75 | MIDI channel |
| 0x76 | CC value / algorithm parameter |
| 0x77 | Processed CC value |
| 0x79 | First data byte |
| 0x7A | Second data byte |
| 0x7B | Third data byte |
| 0xB7 | MIDI status byte |
| 0xB8 | Input buffer fill count |
| 0xBA | Input buffer read index |
| 0xBB | Current input byte |

### External RAM (EXTMEM)

| Address | Purpose |
|---------|---------|
| 0x11E5 | Algorithm/mode value storage |
| 0x1361 | Input buffer base |
| 0x1CBC | Mode flag 1 |
| 0x1CBD | Mode flag 2 |
| 0x1CBE | Calculated parameter storage |

## Open Questions

- [x] What sets mode flags [0x1CBC, 0x1CBD] to [0x7B, 0x13]?
  - **RESOLVED**: CC 0x65 (index 4) sets 0x1CBC, CC 0x64 (index 3) sets 0x1CBD
- [x] What are the functions at 0x6CE3, 0x6D2F, 0x6D79, 0x6D81?
  - **RESOLVED**: 0x6CE3=increment, 0x6D2F=decrement, 0x6D79=set_mode_flag_2, 0x6D81=set_mode_flag_1
- [x] What triggers load_algo_8e19, load_algo_8e99, load_algo_8f19 vs load_algo_8d99?
  - **RESOLVED**: CC 0x06 value 4-7 selects which loader to call (see Algorithm Loading Trigger Sequence)
- [ ] Is there a SysEx handler that enables algorithm loading mode?
  - Likely not needed - mode is set via CC 0x64/0x65 sequence
- [ ] What do CC 0x60 (increment) and 0x61 (decrement) handlers actually modify?
- [ ] What are the handlers in the secondary jumptable at 0x73DA (indices 5-35)?

## Related Files

- `notebooks/xe9l_algo_table_2.ipynb` - Analysis of algorithms at 0x8D99
- `notebooks/xe9l_unused_algorithms.ipynb` - Analysis of percussion algorithms
- `parse_programs_xe9l.py` - Program and drum sound parser
