# WIP: MS4 Program Data Extraction

Status: In Progress (2026-01-24)

## Program Structure (Corrected via Firmware Disassembly)

ROM: `ms4_05_r1_0.bin` (64KB, code space only)
Program pointer table: CODE:0x0040, 66 entries, 2-byte big-endian pointers.

### Program Data Layout

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0-7 | 8 | name | ASCII name (space-padded) |
| 8 | 1 | null_term | Null terminator (0x00) |
| 9 | 1 | flags | bit 7 = complex init, bits 3:0 = SAM slot count |
| 10-11 | 2 | aram_ptr | A-RAM data pointer (LE: [10]=DPL, [11]=DPH) |
| 12-14 | 3 | dram_entry0 | First D-RAM parameter entry (value_lo, value_mid, ctrl) |
| 15-16 | 2 | voice_data_ptr | Pointer to voice init data (LE), 0x0000 = none |
| 17+ | var | dram_data | Additional D-RAM parameter data / inline params |

### Previous (Incorrect) Interpretation

The initial plan assumed byte 12 was an "algorithm slot index" and bytes 13-14
were a "voice init pointer". Firmware analysis proved this wrong:

- **Byte 12** is the `value_lo` of the first 3-byte D-RAM parameter entry
- **Bytes 13-14** are `value_mid` and `ctrl` of that same D-RAM entry
- **Bytes 15-16** are the actual voice init data pointer (LE)

The ALG field in D-RAM word 15 is set dynamically by `voice_init_slots` based on
which D-RAM slot is allocated from the free pool, NOT from a program data field.

## Key Differences from Keyfox10 (parse_programs.py)

| | Keyfox10 | MS4 |
|--|----------|-----|
| ROM | 128KB (CODE+DATA) | 64KB (CODE only) |
| Pointer table addr | 0x6285 | 0x0040 |
| Num programs | 97 | 66 |
| Data space | Separate (ROM offset +0x10000) | N/A (all code space) |
| Sound ptr table | DATA:FFA0 | N/A |
| A-RAM pointer | Indirect (algo_ptrs in sound def) | Direct at offset 10-11 |
| D-RAM params | Via sound definition templates | Inline at offset 12+ |

## Firmware Functions (Ghidra project at port 8195)

### handle_program_change (CODE:C45B)

- Reads program index, looks up 2-byte pointer from table at CODE:0040
- Stores program base address at XRAM:14D3:14D4
- Adds 9 to skip name+null, reads flags byte (bit 7 check)
- Non-complex path: calls voice_assign_algorithm
- Complex path: kills old voices, subtracts 9 to restore program_base,
  then calls voice_assign_algorithm + voice_init_slots

### voice_assign_algorithm (CODE:B4BF)

- Allocates from algorithm pool at XRAM:0x11EE (8 slots max)
- Reads program_base from voice slot XRAM (offset 5-6)
- Reads A-RAM data pointer from program_base+10 (2 bytes LE, via MOVC)
- Loads lookup value from INTMEM:0x9E[pool_slot] → stored at INTMEM[channel+0x8E]
- Computes A-RAM start address: `SWAP(lookup_value); RL A` → gives 0,32,64,...,224
- Calls sam_write_aram(data_ptr, start_addr) to upload 32 instructions

### voice_init_slots (CODE:9A2D)

- Entry: INTMEM_35:36 = program_base, INTMEM_34 = channel
- Reads program_base+9 (flags & 0x0F = slot_count)
- Allocates D-RAM slots via linked list allocator (INTMEM:0x53+)
- Per-slot: writes SAM control, sets D-RAM word 15 = `(slot_id << 4) | 0x0F` : 0x00
  - This encodes: ALG = slot_id high nibble, MIXL = 7 (full), MIXR = 7 (full)
- After loop: copies 8 bytes from program_base+22 to XRAM page 0x11

### voice_note_init_dram (CODE:A89B)

- Called during note trigger (after voice_trigger_note at CODE:A834)
- Reads program_base from voice slot XRAM (offset 5-6)
- Reads flags at program_base+9: if zero returns, if bit7 → complex handler
- Advances pointer by 12: INTMEM_41 += 12 → now points to D-RAM param data
- Sets INTMEM_38 = SWAP(slot_ID) for D-RAM address computation
- Calls dram_param_processor

### dram_param_processor (CODE:AA9A)

- Reads from pointer at INTMEM_42:41 (= program_base + 12)
- Format: 3-byte D-RAM entry + 2-byte LE pointer, repeated
  - INTMEM: 4B (val_lo), 43 (val_mid), 44 (ctrl), 45 (ptr_lo), 46 (ptr_hi)
- If pointer (45|46) == 0: terminator → compute D-RAM base from INTMEM_38, loop D-RAM writes
- If pointer != 0: jump to 0xAB73 → copies 7 bytes of voice init data from
  pointed address to XRAM voice slot, then continues processing

### sam_write_aram (CODE:AD43)

- Writes 32 A-RAM instruction words to SAM
- P2=0x80, loops 32×: reg0=addr, reg1=data_lo, reg2=data_hi, reg4=ctrl|2

### sam_write_dram (CODE:A4BC)

- Writes to SAM D-RAM via EXTMEM buffer at P2 page:
  [0]=data_hi, [1]=count, [2]=addr, [3]=data_lo, [4]=ctrl_flags

## A-RAM Algorithms

3 unique A-RAM data blocks, stored consecutively at 64-byte intervals:

| Address | Programs | Description |
|---------|----------|-------------|
| 0x027A | 52 programs | Main algorithm (piano, strings, brass, organ, etc.) |
| 0x02BA | 12 programs | Plucked/struck (marimba, steel guitar, banjo, ukulele, etc.) |
| 0x02FA | 2 programs | Extended strings (dstring3, dkvoi1) |

Each = 32 words × 2 bytes (LE) = 64 bytes = one SAM algorithm at 44.1kHz.
Decode with: `from sam8905_aram_decoder import decode_algorithm`

## Voice Init Data

The LE pointer at program offset 15-16 points to voice initialization data in
CODE space. 7 bytes are copied to the XRAM voice slot by `voice_init_copy_and_envelope`
(CODE:AB73). Additional bytes (7-11) are read in-place for velocity modulation.

### Byte Layout (7 bytes copied to XRAM slot)

| Byte | XRAM Slot | Description |
|------|-----------|-------------|
| 0 | slot+0 | Envelope segment table pointer - low byte (LE) |
| 1 | slot+1 | Envelope segment table pointer - high byte |
| 2 | slot+2 | Control: bit7=ENV_ENABLE, bit6=LEVEL_SIGN |
| 3 | slot+3 | Reserved (always 0x00 in MS4 ROM) |
| 4 | slot+4 | Envelope sustain/attack parameter |
| 5 | slot+5 | Envelope depth/modulation parameter |
| 6 | slot+6 | Envelope rate (processed to rate<<2 when ENV set, 0=default) |

### Additional ROM bytes (read in-place, not copied)

| Byte | Description |
|------|-------------|
| 7 | Velocity-to-level modifier (signed, 0=none; always 0 in MS4 ROM) |
| 8 | Skipped by firmware |
| 9 | Velocity mod amount for slot[5] (always 0 in MS4 ROM) |
| 10 | Velocity mod amount for slot[4] (always 0 in MS4 ROM) |
| 11 | Velocity mod type/rate param (always 0 in MS4 ROM) |

### XRAM Voice Slot Structure (16 bytes per slot)

After `voice_init_copy_and_envelope` completes:

| Offset | Source | Description |
|--------|--------|-------------|
| 0-1 | init[0:1] | Envelope table pointer (advanced during segment scan) |
| 2 | init[2] | Control flags |
| 3 | init[3] | Reserved |
| 4 | init[4] | Attack param (velocity-modulated by ROM byte 10) |
| 5 | init[5] | Depth param (velocity-modulated by ROM byte 9) |
| 6 | init[6] | Rate → overwritten with level (0x7F/0x00) when ENV |
| 7 | cleared | Always 0 |
| 8-9 | cleared | Cleared if (byte[2] & 0x48) != 0x40 |
| 10-11 | from rate | rate << 2 (16-bit, set when ENV enabled) |
| 12 | computed | Velocity-modified level (from ROM byte 7 × velocity_curve_lookup) |
| 13-15 | env table | First matching 3-byte envelope segment entry |

### Envelope Segment Table

Pointed to by bytes[0:1]. Contains 3-byte entries scanned sequentially:
- Termination: entry[2]!=0 OR (entry[1] & 0xF8)!=0 → use this entry
- Non-terminal: entry[0] → slot[9], advance pointer by 3, continue scan
- In all MS4 programs, the first entry always terminates (immediate use)

### Control Byte (byte[2]) Patterns in MS4 ROM

| Value | Meaning | Programs |
|-------|---------|----------|
| 0x40 | LSIGN only (no envelope) | Most programs (piano, organ, guitar, etc.) |
| 0xC0 | ENV+LSIGN (envelope enabled) | Brass, sax, flute, strings, harmonica |

Programs with pointer = 0x0000 are placeholders (dcello9, dpizzah, dparadh).

### Firmware Functions

| Function | Address | Purpose |
|----------|---------|---------|
| voice_init_copy_and_envelope | CODE:AB73 | Copy 7 bytes, scan env table, set up envelope |
| voice_init_next_slot | CODE:AB40 | Advance INTMEM_3b by 16, continue or start D-RAM |
| velocity_curve_lookup | CODE:B3C5 | Table lookup for velocity-based attenuation |

## D-RAM Parameter Entry Format (offset 12-16)

The 5-byte header entry at program offset 12:

```
Byte 0 (val_lo):   D-RAM word bits 7-0
Byte 1 (val_mid):  D-RAM word bits 15-8
Byte 2 (ctrl):     Control byte
        Bit 7:     Conditional offset flag
        Bits 6-3:  D-RAM address nibble (slot offset within 16-word section)
        Bits 2-0:  Mix/routing bits
Byte 3 (ptr_lo):   Voice init data pointer low (LE)
Byte 4 (ptr_hi):   Voice init data pointer high (LE)
```

If pointer != 0: voice init path (copies 7 bytes from pointer to XRAM voice slot).
If pointer == 0: no voice init, begins D-RAM config stream directly.

## D-RAM Config Stream Format

The D-RAM config stream follows the voice slot pointers (offset 17+).
It encodes initial values for 16 D-RAM words per SAM algorithm slot.

### Dispatch Loop (dram_config_dispatch, CODE:AB4C)

- Counter (INTMEM_39) initialized to 16 (0x10) by dram_param_processor
- Each iteration reads one dispatch byte from the stream pointer (DPTR)
- Bits 5:3 of the dispatch byte select a handler
- Bit 7 set = terminator (writes byte value to D-RAM slot, exits)
- Counter decremented after each word; exits when counter == 0

### Handler Table (Ghidra functions at port 8195)

| Bits 5:3 | Dispatch | Function | Bytes | Description |
|----------|----------|----------|-------|-------------|
| 000 | 0x00-0x07 | dram_config_handler_00 (AD8F) | 3 or 4 | Short write: 3 data bytes; if byte[3] bits 7:5==0 then 4th byte is extra data, else byte[3] is next dispatch |
| 001 | 0x08-0x0F | dram_config_handler_08 (ADBD) | 10 | Pitch/frequency: note tables, octave, pitch bend, portamento, modulation |
| 010 | 0x10-0x17 | dram_config_handler_10 (B030) | 9 | Amplitude/level: envelope, velocity scaling, sustain, attack rate |
| 011 | 0x18-0x1F | dram_config_handler_18 (B222) | 4 | D-RAM write: value + optional velocity modulation, writes via sam_write_dram |
| 100 | 0x20-0x27 | dram_config_handler_20 (B278) | 4 | Output routing: writes XRAM 0xF9-FB, **TERMINATES** stream |
| 101 | 0x28-0x2F | dram_config_handler_28 (B2D2) | 1 | Write constant 0x28 to D-RAM word |
| 110 | 0x30-0x37 | dram_config_handler_30 (B2CF) | 0* | Skip remaining: fills all remaining words with skip, **TERMINATES** |
| 111 | 0x38-0x3F | (inline in dispatch) | 1 | Write constant 0x38 to D-RAM word |

*Handler 0x30 does not advance the pointer; it recursively skips until counter==0.

### Stream Termination Conditions

1. **Counter exhausted**: 16 D-RAM words processed (normal completion)
2. **Bit 7 terminator**: dispatch byte with bit 7 set writes value and exits
3. **Handler 0x20**: output config handler does not call dispatch loop
4. **Handler 0x30**: skips all remaining words without consuming stream data

### Helper Functions

| Function | Address | Purpose |
|----------|---------|---------|
| dram_config_apply_velocity | B1EC | Attenuates MIX bits by velocity (INTMEM_3F) |
| dram_config_skip_4bytes | AD82 | Skips 4-byte handler_18 entry (conditional path) |
| modulation_write_dram | (existing) | Writes pitch with modulation via SAM |

### Observed Stream Lengths

| Length | Count | Pattern |
|--------|-------|---------|
| 14 bytes | 46 | 1× handler_08 (10) + 1× handler_00 (3) + terminator (1) |
| 51 bytes | 16 | 1× handler_08 (10) + handler_00 (4) + multiple handler_18 (4 each) + terminator |
| 62 bytes | 2 | Extended strings (dstring3, dkvoi1) |
| 67 bytes | 1 | dkvoi2 |

## Periodic Voice Update System

### periodic_voice_update (CODE:9BA7)

Called by Timer 1 ISR. Iterates the active voice linked list and updates
all LFO/envelope blocks and D-RAM slot modulations per voice.

### Per-Voice XRAM Page Layout (256 bytes, selected by P2)

| Offset | Size | Description |
|--------|------|-------------|
| 0x00-0x6F | 112 | 7 LFO/envelope state blocks (16 bytes each) |
| 0x70+ | ~16 | D-RAM slot ID table (slot count + slot IDs) |
| 0x80-0xFF | 128 | D-RAM slot modulation state (8 bytes per slot, up to 16 slots) |
| 0xFB | 1 | Voice flags |
| 0xFC | 1 | MIDI channel |
| 0xFD | 1 | Next voice pointer (linked list, 0xFF = end) |

### LFO/Envelope Block (16 bytes each, 7 per voice)

| Byte | Description |
|------|-------------|
| 0 | Waveform type (0/5/6/7=sine, 1=ramp, 2=inverted, 3=square, 4=noise) |
| 1 | Frequency/rate |
| 2-3 | Phase accumulator (16-bit, incremented by freq each tick) |
| 4-5 | Amplitude (current output value) |
| 6-7 | Envelope segment pointer (CODE space) |
| 8-9 | Envelope state |
| 10-11 | Target/limit |
| 12-15 | Additional state |

LFO waveform computation:
- sine: table at CODE:9833 (64 entries, 6-bit index from phase high byte)
- ramp: phase high byte = output
- inverted: 0xFF - phase high byte
- square: 0x00 or 0xFF based on phase MSB
- noise: LFSR (x = x*3 + 0x43)

### D-RAM Slot Modulation State (8 bytes per slot)

| Byte | Description |
|------|-------------|
| 0 | Type: 0x10=amplitude, bit7=portamento, 0x38=skip, other=pitch |
| 1 | D-RAM word address (slot_id << 4 | word_offset) |
| 2 | Type-dependent: rate counter (porta) or envelope block index (amp) |
| 3 | Type-dependent: rate value (porta) or velocity scale (amp) |
| 4-5 | Current value (16-bit: pitch or amplitude) |
| 6-7 | Target value (16-bit, portamento only) |

### Global Modulation LFO (XRAM 0x1180-0x1183)

| Address | Name | Description |
|---------|------|-------------|
| 0x1180 | mod_lfo_rate | LFO rate (0=off, higher=faster vibrato) |
| 0x1181 | mod_lfo_phase_lo | Phase accumulator low byte |
| 0x1182 | mod_lfo_phase_hi | Phase accumulator high byte |
| 0x1183 | mod_lfo_output | Sine table lookup result (vibrato depth) |

Phase increment = rate × 32. Sine indexed by (phase_hi >> 1) & 0x3F.
Rate is set by MIDI mod wheel (CC#1) handler.

### Global XRAM Memory Map

| Address | Size | Name | Description |
|---------|------|------|-------------|
| 0x1180 | 1 | mod_lfo_rate | Global vibrato LFO rate |
| 0x1181-82 | 2 | mod_lfo_phase | Global vibrato phase accumulator |
| 0x1183 | 1 | mod_lfo_output | Global vibrato sine output |
| 0x1184 | 16 | mod_sensitivity | Per-channel modulation sensitivity |
| 0x1194 | 32 | pitch_bend | Per-channel pitch bend (16-bit × 16 ch) |
| 0x11EE | 8 | algorithm_pool | SAM algorithm slot allocation pool |
| 0x14D3 | 2 | program_base | Current program pointer (LE) |

### INTMEM Locations

| Address | Name | Description |
|---------|------|-------------|
| 0x53 | dram_slot_free_list | D-RAM slot linked list head |
| 0x54 | active_voice_list_head | Active voice linked list (0xFF=empty) |
| 0x9E | algorithm_slot_lookup | 8-byte pool-to-slot mapping table |

### Voice Update Functions

| Function | Address | Purpose |
|----------|---------|---------|
| periodic_voice_update | 9BA7 | Main timer ISR voice update loop |
| global_mod_lfo_update | A314 | Advance global vibrato LFO (sine) |
| dram_slot_amplitude_update | A18F | Update amplitude via envelope output |
| dram_slot_apply_mod_depth | A2E3 | Apply modulation depth to slot pitch |
| dram_slot_portamento_update | A33E | Smooth pitch glide toward target |

## Tasks

- [x] Copy parse_programs.py to ms4_parse_programs.py
- [x] Update ROM_PATH, PROGRAM_PTR_TABLE, NUM_PROGRAMS
- [x] Remove DATA space functions (MS4 is code-only)
- [x] Parse A-RAM data and decode with sam8905_aram_decoder
- [x] Trace firmware to understand program structure
- [x] Correct program structure (byte 12 is D-RAM param, not algo index)
- [x] Add Ghidra comments to key functions
- [x] Write WIP document
- [x] Update parser to use corrected structure (voice_data_ptr at offset 15-16)
- [x] Parse D-RAM config stream with forward-scanning byte consumption
- [x] Document all D-RAM config handlers in Ghidra (rename + comments)
- [x] Decode voice init data (7+5 bytes: envelope table ptr, control, rate, velocity mods)
- [x] Document periodic_voice_update and voice XRAM page structure
- [x] Label XRAM/INTMEM memory regions in Ghidra (mod LFO, pitch bend, algorithm pool, etc.)
- [x] Rename and comment voice update sub-functions (amplitude, portamento, mod depth, global LFO)
- [ ] Decode D-RAM stream commands (interpret handler_08 pitch, handler_10 amplitude, etc.)
- [ ] Cross-reference D-RAM entries with A-RAM algorithm D-RAM usage

## How to Run

```bash
python3 sam8905/ms4_parse_programs.py
```
