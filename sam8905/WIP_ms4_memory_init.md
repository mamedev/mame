# MS4 Memory Initialization Analysis

Analysis of the Hohner MS4 (ms4_05_r1_0.bin) boot sequence and memory layout,
traced from the 80C52 reset vector through all initialization routines.

## Boot Sequence

```
Reset vector (CODE:0000) → LJMP START_i_dcbc
  START_i_dcbc:
    CLR EA (disable interrupts)
    → LJMP START_ii_da24

  START_ii_da24:
    CLR IE (all interrupts off)
    MOV 0x8004, #0x40  (SAM control: reset/init)
    Clear all SFRs (0x80-0xFF)
    Clear INTMEM 0x08-0x7F
    → LCALL START_iii_d463 (RAM self-test)
    → JMP MAINLOOP_da30

  MAINLOOP_da30:
    LCALL extmem_clear_all (D454)  - clears EXTMEM 0x0000-0x1FFF
    LCALL dummy_reti (D4FE)        - clears pending interrupts
    Configure ports: P1=#0xFF, P3=#0xFF
    Configure Timer 2: T2CON=#0x34, RCAP2H=#0xFF, RCAP2L=#0xF0
      → 16MHz / (32 * 16) = 31250 baud (MIDI)
    Configure UART: SCON=#0x5C (Mode 1, REN=1, TI=1)
    LCALL voice_pages_clear (B70B) - clears extended voice pages
    LCALL init_pitch_and_voices (98AD)
    Configure interrupts: IP=#0x10, IE=#0x98
      → EA=1, ES=1 (serial), ET1=1 (timer1)
    MOV EXTMEM:1180, #0x38 (mod LFO rate = 0x38)
    → Enter main event loop
```

## RAM Self-Test (START_iii_d463)

Tests 8KB XRAM (0x0000-0x1FFF) with walking-bit pattern:
- Writes 0x55 to all bytes, reads back and verifies
- Writes 0xAA to all bytes, reads back and verifies
- On failure: halts (infinite loop)


## EXTMEM (8KB XRAM) Memory Map

| Address Range   | Size  | Name                    | Init Value | Description |
|-----------------|-------|-------------------------|------------|-------------|
| 0x0000-0x00FF   | 256   | voice_page_00           | 0x00       | Voice state page 0 (P2=0x00) |
| 0x0100-0x01FF   | 256   | voice_page_01           | 0x00       | Voice state page 1 |
| ...             | ...   | ...                     | ...        | ... |
| 0x0F00-0x0FFF   | 256   | voice_page_0f           | 0x00       | Voice state page 15 |
| 0x1000-0x107F   | 128   | pitch_table_lo          | computed   | Pitch freq table - low byte |
| 0x1080-0x10FF   | 128   | pitch_table_mid         | computed   | Pitch freq table - mid byte |
| 0x1100-0x117F   | 128   | pitch_table_hi          | computed   | Pitch freq table - high byte |
| 0x1180          | 1     | mod_lfo_rate            | 0x38       | Global vibrato LFO rate |
| 0x1181          | 1     | mod_lfo_phase_lo        | 0x00       | LFO phase accumulator low |
| 0x1182          | 1     | mod_lfo_phase_hi        | 0x00       | LFO phase accumulator high |
| 0x1183          | 1     | mod_lfo_output          | 0x00       | LFO sine output (signed) |
| 0x1184-0x1193   | 16    | mod_wheel_sensitivity   | 0x00       | Per-channel mod sensitivity |
| 0x1194-0x11B3   | 32    | pitch_bend_table        | 0x00       | Per-channel pitch bend (16 × 2 bytes) |
| 0x11B4-0x11D4   | 33    | (unknown)               | 0x00       | Cleared by extmem_clear_all |
| 0x11D5-0x11E4   | 16    | voice_ctrl_area_1       | 0x00       | Per-voice mod/portamento state |
| 0x11E5          | 1     | (unknown)               | 0x00       | |
| 0x11E6-0x11ED   | 8     | program_init_copy       | from ROM   | Copied from program_base+22 |
| 0x11EE-0x11F5   | 8     | algorithm_pool          | 0x00       | SAM algorithm slot pool |
| 0x11F6-0x14D2   | 733   | (uncharacterized)       | 0x00       | |
| 0x14D3-0x14D4   | 2     | current_program_base    | 0x00       | Current program pointer (LE) |
| 0x14D5-0x1CD4   | 2048  | extended_voice_pages    | 0x00       | 8 pages × 256 bytes |
| 0x1CD5-0x1CE4   | 16    | voice_ctrl_area_2       | 0x00       | Per-voice status/flags |
| 0x1CE5-0x1FFF   | 795   | (uncharacterized)       | 0x00       | |

### Voice Page Layout (pages 0x00-0x0F, 256 bytes each)

Each page is addressed via P2 register (8051 MOVX @R0 with P2 as high byte).
Pages represent D-RAM voice slots. During voice_init_slots:

| Offset   | Size | Init    | Description |
|----------|------|---------|-------------|
| 0x42     | 1    | 0xFF    | Voice active marker |
| 0x70-0x73| 4    | 0x0F    | D-RAM init flags (per-word mask) |
| 0x74-0x7F| 12   | 0xFF    | D-RAM init flags (unused words) |
| 0xFB     | 1    | 0x20    | SAM control flags |
| 0xFC     | 1    | slot_id | Slot ID (nibble-swapped) |

### Extended Voice Page Layout (16-byte slots via voice_slot_base)

Within extended voice pages, each voice uses 16-byte slots addressed by
`voice_slot_base` (INTMEM:3b). The slot base increments by 16 for each
D-RAM word being configured.

**Envelope slot (from voice_init_copy_and_envelope):**

| Offset | Size | Description |
|--------|------|-------------|
| 0      | 1    | Envelope segment table pointer - low byte (LE) |
| 1      | 1    | Envelope segment table pointer - high byte |
| 2      | 1    | Control flags (bit7=env enable, bit6=level sign) |
| 3      | 1    | Reserved (0x00) |
| 4      | 1    | Envelope sustain/attack parameter |
| 5      | 1    | Envelope depth/modulation parameter |
| 6      | 1    | Envelope rate |
| 7      | 1    | State (cleared to 0) |
| 8-9    | 2    | Envelope phase (conditionally cleared) |
| 10-11  | 2    | Rate << 2 (16-bit processed rate) |
| 0xC    | 1    | Level sign / velocity modifier |
| 0xD-0xF| 3    | Current envelope segment entry |

**Pitch slot (from dram_config_handler_08):**

| Offset | Size | Description |
|--------|------|-------------|
| 0      | 1    | Velocity sensitivity |
| 1      | 1    | Pitch value - low byte |
| 2      | 1    | Pitch value - mid byte |
| 3      | 1    | Pitch flags (bit7=mod, bit5=portamento, bit3=MIX) |
| 4      | 1    | Pitch bend range (& 0x7F); bit7=portamento active |
| 5      | 1    | Fine tune - low byte |
| 6      | 1    | Fine tune - high byte |
| 7      | 1    | Pitch sign / octave indicator |
| 8      | 1    | (reserved) |
| 9-11   | 3    | Portamento target pitch (24-bit) |
| 0xC    | 1    | Portamento rate |
| 0xD    | 1    | Portamento direction (complement of pitch delta) |
| 0xE-0xF| 2    | Modulation state (cleared to 0) |

### Pitch Frequency Tables

Computed at boot by `pitch_table_init` (CODE:9B16). Three 128-byte banks
form a 24-bit phase increment per MIDI note:

```
note_freq[n] = (pitch_table_hi[n] << 16) | (pitch_table_mid[n] << 8) | pitch_table_lo[n]
```

The computation uses a base frequency constant and octave division via
successive right-shifts. The LFO sine table at CODE:9833 (64 bytes) provides
the vibrato waveform data (indexed by `(phase_hi >> 1) & 0x3F`).


## INTMEM (128 bytes) Memory Map

### Register Banks (0x00-0x1F)

| Address | Size | Description |
|---------|------|-------------|
| 0x00-0x07 | 8 | Register Bank 0 (R0-R7, default) |
| 0x08-0x0F | 8 | Register Bank 1 (voice update context) |
| 0x10-0x17 | 8 | Register Bank 2 (ISR context) |
| 0x18-0x1F | 8 | Register Bank 3 (unused?) |

### Bit-Addressable Area (0x20-0x2F)

Used for various flag bits. Specific bit assignments TBD.

### Voice System Variables (0x30-0x5F)

| Address | Name                  | Init  | Description |
|---------|-----------------------|-------|-------------|
| 0x34    | current_slot_id       | -     | Current D-RAM slot being processed |
| 0x35    | program_base_dph      | -     | Program data pointer high byte |
| 0x36    | program_base_dpl      | -     | Program data pointer low byte |
| 0x37    | sam_ctrl_flags        | -     | SAM control register value |
| 0x38    | dram_address_counter  | -     | D-RAM address counter during init |
| 0x39    | remaining_slots       | 0x10  | Remaining D-RAM slots to process |
| 0x3a    | voice_page_num        | -     | Current voice page (P2 value for XRAM) |
| 0x3b    | voice_slot_base       | -     | XRAM voice slot R1 base |
| 0x3c    | midi_note             | -     | Current MIDI note number |
| 0x3d    | midi_velocity         | -     | Current MIDI velocity |
| 0x3e    | amplitude_scale       | -     | Amplitude scaling factor (for env) |
| 0x3f    | velocity_mix_atten    | -     | Velocity MIX attenuation (bit7=MIXL/MIXR sel) |
| 0x40    | pitch_bend_value      | -     | Current pitch bend (signed) |
| 0x41    | rom_data_ptr_lo       | -     | ROM parse pointer low byte |
| 0x42    | rom_data_ptr_hi       | -     | ROM parse pointer high byte |
| 0x45    | voice_data_ptr_lo     | -     | Voice init data pointer low |
| 0x46    | voice_data_ptr_hi     | -     | Voice init data pointer high |
| 0x4a    | slot_count            | -     | Number of D-RAM slots for voice |
| 0x4c    | portamento_value      | -     | Portamento rate (CC5 value) |
| 0x4f    | dram_slot_index       | -     | Current D-RAM slot write index |
| 0x52    | octave_shift          | 0     | Octave transposition counter |
| 0x53    | dram_slot_free_list   | 0x00  | Free list head (linked list) |
| 0x54    | active_voice_list_head| 0xFF  | Active voice list (0xFF=empty) |
| 0x55    | pending_voice_list    | 0xFF  | Pending voice release list |
| 0x56    | dram_slot_count       | 0x10  | Total D-RAM slots available |

### D-RAM Free List (0x7E-0x8D)

16-byte linked list for SAM D-RAM slot allocation.

Initialized by `slot_manager_init` (CODE:9904):
```
INTMEM[0x7E] = 0x01  (slot 0 → next = slot 1)
INTMEM[0x7F] = 0x02  (slot 1 → next = slot 2)
...
INTMEM[0x8C] = 0x0F  (slot 14 → next = slot 15)
INTMEM[0x8D] = 0xFF  (slot 15 → end of list)
```

Head at INTMEM:53 = 0x00 (first free = slot 0).
Allocator function: `FUN_CODE_a9cf` (removes head, returns slot page number).

### Per-Channel Algorithm Assignments (0x8E-0x9D)

16 bytes initialized from table at CODE:DC0B:
```
Channel:  0    1    2    3    4    5    6    7    8-15
Value:    01   02   03   05   04   04   04   04   00...
```

Maps MIDI channel number to SAM algorithm slot assignment.
Channels 0-3 get unique algorithms (1,2,3,5), channels 4-7 share algorithm 4,
channels 8-15 are unassigned (0).

### Algorithm Slot Lookup (0x9E-0xA5)

8 bytes. Maps algorithm pool index to actual D-RAM slot number.
Initialized to 0x00 during slot_manager_init.


## SAM8905 D-RAM Initialization

During boot, `sam_dram_clear_all` (CODE:A53C) clears all 256 D-RAM words to 0,
then sets word 15 of each of the 16 slots to configure ALG/MIX:

```
For slot N:
  D-RAM address = (N << 4) | 0x0F  (word 15 of slot N)
  Value = (N_swapped << 8) | 0x0F00 | 0x0000
  Where N_swapped = (N >> 4) | (N << 4)  (nibble swap)
```

This sets each slot's control word with algorithm routing and full mix (0x0F = MIXL=1, MIXR=7).


## Initialization Functions

| Address | Name                    | Called By        | Purpose |
|---------|-------------------------|------------------|---------|
| D454    | extmem_clear_all        | MAINLOOP_da30    | Clear 8KB EXTMEM to 0 |
| D4FE    | dummy_reti              | MAINLOOP_da30    | Clear pending interrupt flags |
| B70B    | voice_pages_clear       | MAINLOOP_da30    | Clear EXTMEM 0x14D5-0x1CE4 |
| 98AD    | init_pitch_and_voices   | MAINLOOP_da30    | ROM checksum + voice/pitch init |
| 9904    | slot_manager_init       | init_pitch_and_voices | Free list + pool + ctrl areas |
| A53C    | sam_dram_clear_all      | slot_manager_init | Clear SAM D-RAM + set word 15 |
| 9B16    | pitch_table_init        | init_pitch_and_voices | Compute 128-note pitch table |
| DCA8    | dptr_add_r6r7           | pitch_table_init  | DPTR += R6:R7 utility |
| A523    | sam_dram_write_word15   | sam_dram_clear_all | Write ALG/MIX control word |

## Voice Init Functions (Note-On Path)

| Address | Name                        | Purpose |
|---------|-----------------------------|---------|
| 9A2D    | voice_init_slots            | Allocate D-RAM slots, init voice pages, copy program data |
| AB40    | voice_init_next_slot        | Advance to next D-RAM word, dispatch config handler |
| AB73    | voice_init_copy_and_envelope| Copy 7-byte envelope data to slot, process envelope table |
| AB4C    | dram_config_dispatch        | Dispatch loop: reads byte, selects handler by bits 5:3 |
| AD82    | dram_config_skip_4bytes     | Skip 4-byte entry (conditional path in handler_18) |
| AD8F    | dram_config_handler_00      | Short D-RAM write (3-4 bytes) |
| ADBD    | dram_config_handler_08      | Pitch/frequency setup (10 bytes) |
| B030    | dram_config_handler_10      | Amplitude/level + envelope (9 bytes) |
| B1EC    | dram_config_apply_velocity  | Attenuate MIX bits by velocity |
| B222    | dram_config_handler_18      | D-RAM write + velocity mod (4 bytes) |
| B278    | dram_config_handler_20      | Output routing config (4 bytes, TERMINATES) |
| B2D2    | dram_config_handler_28      | Write constant 0x28 (1 byte) |
| B2CF    | dram_config_handler_30      | Skip remaining / re-dispatch (1 byte) |

### D-RAM Config Stream Format

The voice init data (from program ROM) is a byte stream parsed by
`voice_init_next_slot`. Each entry starts with a dispatch byte:

```
Dispatch byte format:
  Bit 7:   1 = terminator (end of stream for this D-RAM word)
  Bits 5-3: Handler type (0x00-0x38, shifted right 3 for jump table)
  Bits 2-0: Sub-flags (handler-specific)
```

Handler types:
- **0x00**: Short D-RAM write (3-4 bytes: dispatch + value_lo + value_hi [+ extra])
- **0x08**: Pitch/frequency (10 bytes: dispatch + vel_sens + note_offset + ctrl + bend_range + fine_lo + fine_hi + skip + port_rate + port_depth)
- **0x10**: Amplitude/level (9 bytes: dispatch + base_level + amplitude + env_ctrl + rate + unused + sustain + vel_sens + mod_amt)
- **0x18**: D-RAM write + velocity mod (4 bytes: dispatch_value + mod1 + mod2 + mod3)
- **0x20**: Output routing (4 bytes: dispatch + route1→page[F9] + route2→page[FA] + route3_bits→page[FB]). TERMINATES.
- **0x28**: Write constant 0x28 to slot (1 byte, just dispatch). Simple placeholder.
- **0x30**: Skip remaining words (1 byte). Re-enters dispatch loop for remaining words without consuming stream data. TERMINATES when counter=0.
- **0x38**: Inline default (1 byte). Writes 0x38 to slot, advances to next word.

Between D-RAM word entries, a 2-byte voice_data_ptr is read. If non-zero,
`voice_init_copy_and_envelope` is called to set up the envelope for that word.

### Handler 0x10: Amplitude/Level (9 bytes)

Most complex handler. Controls D-RAM amplitude with velocity scaling and envelope.

```
Stream: [dispatch] [base_level] [amplitude] [env_ctrl] [rate] [unused] [sustain] [vel_sens] [mod_amt]

dispatch bits:
  bit 0: output_sel (1=apply velocity to D-RAM output)
  bit 1: copy_level (1=copy computed level to slot[7])
  bit 2: phase_inv (1=invert phase in D-RAM write)

env_ctrl bits:
  bit 7: envelope_enable (store to dram_slot_index for periodic update)
  bit 6: no_velocity_scaling (skip velocity * amplitude attenuation)
  bit 4: special_mode (store to dram_slot_index, different D-RAM write path)
  bit 0: modulation_enable (apply mod to MIX attenuation)

rate bits:
  bit 3: portamento_flag (register for periodic amplitude portamento)

Velocity scaling (when vel_sens != 0):
  amplitude -= amplitude * (127-velocity) * vel_sens / 256

Phase inversion (bit 2 set, no velocity scaling):
  Writes complement of amplitude to D-RAM word N-1 as negative phase offset
```

### Handler 0x18: D-RAM Write + Velocity Mod (4 bytes)

Simple word write with optional MIX velocity attenuation.

```
Stream: [dispatch_value] [mod_byte_1] [mod_byte_2] [mod_byte_3]

dispatch_value: stored to voice_slot_base[0]
If dispatch bit 0 set:
  mod_byte_1 stored to voice_slot_base[1]
  dram_config_apply_velocity(mod_byte_2, mod_byte_3) called
  → attenuates MIXR or MIXL bits by velocity_mix_atten (INTMEM:3F)
Writes to SAM D-RAM at current address
```

### Handler 0x20: Output Routing (4 bytes, TERMINATES)

Configures the voice output routing in the voice page. This is the
last handler called for a voice - it terminates the dispatch loop.

```
Stream: [dispatch] [route1] [route2] [route3]

route1 → XRAM page[0xF9]  (output routing byte 1)
route2 → XRAM page[0xFA]  (output routing byte 2)
route3 bits 2:0 → XRAM page[0xFB] low 3 bits (merge with existing)

Sets voice_slot_base = 0xF8 (point to output routing area of voice page)
Optionally writes to SAM D-RAM or applies velocity attenuation
```

### Handler 0x28: Write Constant (1 byte)

Placeholder/padding handler. Writes constant 0x28 to the voice slot
and advances the stream pointer by 1 byte (just the dispatch byte itself).

### Handler 0x30: Skip Remaining (1 byte, may TERMINATE)

Advances to next D-RAM word without consuming additional stream data.
Then re-enters the dispatch loop. If a new dispatch byte is found, it
dispatches to the appropriate handler. Terminates when the remaining
word counter reaches 0 or a terminator byte is encountered.

### dram_config_apply_velocity

Attenuates the MIX routing bits based on velocity:
```
If velocity_mix_atten bit 7 = 0:
  MIXR = max(0, slot[1] bits 2:0 - velocity_mix_atten)
If velocity_mix_atten bit 7 = 1:
  MIXL = max(0, slot[1] bits 5:3 - (velocity_mix_atten & 0x7F))
```
This provides velocity-sensitive output level by reducing the SAM
D-RAM MIX routing values for quieter notes.


## Voice Init ROM Data Format

From `voice_init_copy_and_envelope`, the voice init data block is 12 bytes:

```
Bytes copied to XRAM slot (7 bytes):
  [0]: Envelope table ptr low (LE)
  [1]: Envelope table ptr high
  [2]: Control flags
  [3]: Reserved (0x00)
  [4]: Sustain/attack
  [5]: Depth/modulation
  [6]: Rate

Bytes read in-place (5 bytes, not copied):
  [7]: Velocity-to-level modifier
  [8]: (skipped)
  [9]: Velocity mod for slot[5]
  [10]: Velocity mod for slot[4]
  [11]: Velocity curve type
```


## Task List

- [x] Trace boot sequence from reset vector
- [x] Document EXTMEM memory map
- [x] Document INTMEM memory map
- [x] Add Ghidra labels to pitch table regions
- [x] Add Ghidra labels to voice control areas
- [x] Add Ghidra labels to INTMEM voice system variables
- [x] Add Ghidra labels to D-RAM free list and per-channel data
- [x] Document voice_init functions and their memory usage
- [x] Document D-RAM config stream format
- [x] Trace dram_config_handler_10 through _30 (remaining handlers)
- [ ] Characterize EXTMEM 0x11F6-0x14D2 region
- [ ] Characterize EXTMEM 0x1CE5-0x1FFF region
- [ ] Map bit-addressable INTMEM (0x20-0x2F) flag assignments
- [ ] Document Timer 1 ISR (0xD440) periodic voice update trigger
