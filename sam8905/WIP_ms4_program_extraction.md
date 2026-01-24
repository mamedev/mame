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

The LE pointer at program offset 15-16 points to 7 bytes of voice initialization
data in CODE space. This data is copied to the XRAM voice slot during note init
by dram_param_processor (at 0xAB73 path).

Example (dpiano27): ptr=0x18C8 → data: `12 09 40 00 00 00 00`

Programs with pointer = 0x0000 appear to be placeholders (dcello9, dpizzah, dparadh).

## D-RAM Parameter Entry Format

Same 3-byte format as Keyfox10:

```
Byte 0 (val_lo):   D-RAM word bits 7-0
Byte 1 (val_mid):  D-RAM word bits 15-8
Byte 2 (ctrl):     Control byte
        Bit 7:     Conditional offset flag
        Bits 6-3:  D-RAM address nibble (slot offset within 16-word section)
        Bits 2-0:  Mix/routing bits
```

## Tasks

- [x] Copy parse_programs.py to ms4_parse_programs.py
- [x] Update ROM_PATH, PROGRAM_PTR_TABLE, NUM_PROGRAMS
- [x] Remove DATA space functions (MS4 is code-only)
- [x] Parse A-RAM data and decode with sam8905_aram_decoder
- [x] Trace firmware to understand program structure
- [x] Correct program structure (byte 12 is D-RAM param, not algo index)
- [x] Add Ghidra comments to key functions
- [x] Write WIP document
- [ ] Update parser to use corrected structure (voice_data_ptr at offset 15-16)
- [ ] Decode voice init data (7 bytes at pointer)
- [ ] Parse D-RAM parameter entries from offset 12+
- [ ] Cross-reference D-RAM entries with A-RAM algorithm D-RAM usage

## How to Run

```bash
python3 sam8905/ms4_parse_programs.py
```
