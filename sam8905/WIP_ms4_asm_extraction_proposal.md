# MS4 Firmware ASM Extraction Proposal

## Objective

Extract reusable 8051 assembly modules from the MS4 firmware (ms4_05_r1_0.bin) for use in new SAM8905-based synthesizer projects.

## Ghidra MCP Tools for Extraction

### Primary Extraction Tools

| Tool | Purpose | Usage |
|------|---------|-------|
| `functions_list` | Enumerate all 144 named functions | Identify extraction candidates |
| `functions_disassemble` | Get assembly for each function | Primary extraction method |
| `functions_get` | Get function metadata (size, signature) | Determine code boundaries |
| `functions_get_variables` | Get local variable info | Document register/RAM usage |
| `analysis_get_callgraph` | Map function dependencies | Identify required includes |
| `xrefs_list` | Find cross-references to/from addresses | Locate data dependencies |
| `memory_read` | Read raw bytes and data tables | Extract lookup tables |
| `data_list` | List defined data items | Find constants and tables |
| `data_list_strings` | List string constants | Extract program names, etc. |

### Extraction Workflow Per Module

```
1. functions_disassemble(address) → Get raw assembly
2. functions_get_variables(address) → Document IRAM/XRAM usage
3. analysis_get_callgraph(address, max_depth=3) → Find dependencies
4. xrefs_list(to_addr=address) → Find callers (for interface documentation)
5. memory_read(table_addresses) → Extract associated data tables
```

## Proposed Module Structure

### Directory Layout

```
sam8905_lib/
├── core/
│   ├── sam_hw.asm          # SAM8905 hardware interface
│   ├── sam_dram.asm        # D-RAM read/write operations
│   ├── sam_aram.asm        # A-RAM algorithm loading
│   └── sam_defs.inc        # Register addresses, masks
├── voice/
│   ├── voice_alloc.asm     # Voice allocation/deallocation
│   ├── voice_trigger.asm   # Note on/off triggering
│   ├── voice_envelope.asm  # Envelope state machine
│   ├── voice_lfo.asm       # LFO generation (sine, ramp, square, noise)
│   ├── voice_modulation.asm # Pitch bend, mod wheel application
│   └── voice_defs.inc      # Voice state structure definitions
├── midi/
│   ├── midi_uart.asm       # UART ISR and buffer management
│   ├── midi_parser.asm     # Running status, message dispatch
│   ├── midi_cc.asm         # CC handler dispatch
│   ├── midi_notes.asm      # Note on/off with sustain
│   ├── midi_program.asm    # Program change handling
│   └── midi_defs.inc       # Buffer addresses, channel config
├── util/
│   ├── dptr_ops.asm        # DPTR arithmetic utilities
│   ├── math_ops.asm        # Multiply, divide, saturate
│   ├── table_lookup.asm    # Generic table search
│   └── util_defs.inc       # Common macros
└── data/
    ├── velocity_curves.asm # Velocity lookup tables
    ├── pitch_tables.asm    # Note-to-frequency tables
    └── lfo_tables.asm      # Sine table for LFO
```

## Function Categories and Extraction Priority

### Priority 1: SAM8905 Hardware Interface (Core)

| Function | Address | Size | Dependencies | Notes |
|----------|---------|------|--------------|-------|
| sam_write_dram | 0xA4BC | ~18 | None | Single D-RAM word write |
| sam_write_aram | 0xAD43 | ~63 | None | 32-word algorithm write |
| sam_dram_write_word15 | 0xA523 | ~25 | FUN_a4e6, FUN_a4ce | Control word write |
| sam_dram_clear_all | 0xA53C | ~77 | sam_write_dram | Clear all 256 D-RAM addresses |
| sam_aram_load_alg0 | 0xD866 | ~35 | sam_write_aram | Load algorithm to slot 0 |

**Extraction command sequence:**
```
functions_disassemble(0xA4BC)  # sam_write_dram
functions_disassemble(0xAD43)  # sam_write_aram
functions_disassemble(0xA523)  # sam_dram_write_word15
functions_disassemble(0xA53C)  # sam_dram_clear_all
```

### Priority 2: Voice Management

| Function | Address | Calls | Notes |
|----------|---------|-------|-------|
| voice_trigger_note | 0xA834 | a589, envelope funcs | Main note trigger entry |
| voice_note_init_dram | 0xA89B | param processor | D-RAM initialization |
| voice_deactivate | 0xA69C | sam_write_dram | Silence single voice |
| voice_kill_channel | 0xA785 | voice_deactivate | Kill all voices on channel |
| voice_assign_algorithm | 0xB4BF | sam_write_aram | Allocate voice + load alg |
| voice_init_slots | 0x9A2D | a9cf, aa12 | Slot allocation |
| find_active_voice | 0xD417 | None | Search voice table |
| voice_pages_clear | 0xB70B | None | Clear voice state pages |

### Priority 3: Envelope and Modulation Engine

| Function | Address | Called from | Notes |
|----------|---------|-------------|-------|
| periodic_voice_update | 0x9BA7 | Main loop | ~11ms tick, walks voice list |
| modulation_write_dram | 0x9FCD | periodic | Write mod param to SAM |
| envelope_tick_volume | 0xA403 | CC#7 handler | Immediate volume update |
| envelope_write_dram | 0xA471 | envelope_tick | Compute + write envelope |
| dram_slot_amplitude_update | 0xA18F | periodic | Amplitude envelope tick |
| dram_slot_apply_mod_depth | 0xA2E3 | amplitude_update | Apply mod wheel depth |
| global_mod_lfo_update | 0xA314 | periodic | Global LFO tick |
| dram_slot_portamento_update | 0xA33E | periodic | Pitch glide processing |

### Priority 4: MIDI Handling

| Function | Address | Notes |
|----------|---------|-------|
| ISR_UART_HANDLER | 0xB630 | RX/TX circular buffer ISR |
| SERIAL_HANDLER | 0xC635 | Main loop MIDI parser |
| handle_note_on_off | 0xB75F | Note processing + sustain |
| handle_all_notes_off | 0xBAE1 | CC#123 panic |
| handle_note_off_sustain | 0xBC31 | Note off with sustain check |
| handle_sustain_release | 0xBDD5 | Sustain pedal release |
| handle_sustain_toggle | 0xBF18 | Sustain on/off transition |
| cc_dispatch | 0xC1A0 | CC lookup + dispatch |
| cc_switch_handler | 0xC2BA | Performance CC switch |
| handle_program_change | 0xC45B | Program change + SAM load |
| midi_panic_all_channels | 0xBDBC | Buffer overflow handler |

### Priority 5: D-RAM Parameter Configuration

| Function | Address | Notes |
|----------|---------|-------|
| dram_param_processor | 0xAA9A | Main param dispatch loop |
| dram_config_dispatch | 0xAB4C | Type-based handler dispatch |
| dram_config_handler_00 | 0xAD8F | Type 0: Frequency/pitch |
| dram_config_handler_08 | 0xADBD | Type 1: Complex/envelope |
| dram_config_handler_10 | 0xB030 | Type 2: Filter/amplitude |
| dram_config_handler_18 | 0xB222 | Type 3 |
| dram_config_handler_20 | 0xB278 | Type 4 |
| dram_config_handler_28 | 0xB2D2 | Type 5 |
| dram_config_handler_30 | 0xB2CF | Type 6 |
| dram_config_apply_velocity | 0xB1EC | Velocity curve application |
| velocity_curve_lookup | 0xB3C5 | Velocity table lookup |

### Priority 6: Utility Functions

| Function | Address | Notes |
|----------|---------|-------|
| indexed_array_access | 0x0003 | Generic XRAM array access |
| dptr_add_2a | 0x0026 | DPTR += 2*A (word index) |
| increment_dptr_hi_low | 0x0035 | DPTR++ with both bytes |
| load_dptr_from_xram | 0xDC9C | Load DPTR from 2-byte pointer |
| dptr_add_r6r7 | 0xDCA8 | DPTR += R6:R7 |
| dptr_increment | 0xDCB3 | DPTR += A |
| sub16_r6r7_minus_a | 0x0013 | 16-bit subtraction |

## Data Tables to Extract

| Name | Address | Size | Notes |
|------|---------|------|-------|
| CC lookup table | 0xD405 | 18 | CC# to handler index |
| Program validity table | 0xC8A1 | 66 | Valid program numbers |
| Program pointer table | 0x0040 | 132 | 66 × 2-byte pointers |
| Velocity curve | 0xB3D5+ | ~128 | Velocity mapping |
| LFO sine table | 0x9833 | 64 | 64-entry sine lookup |
| Pitch tables | 0x9B16+ | var | Note-to-frequency |
| Note split table | 0xD383 | var | Layer split points |

**Extraction:**
```
memory_read(0xD405, 18)    # CC table
memory_read(0xC8A1, 66)    # Program validity
memory_read(0x0040, 132)   # Program pointers
memory_read(0x9833, 64)    # LFO sine
```

## RAM Address Map (Must Document)

### Internal RAM (IRAM) 0x00-0x7F

| Address | Purpose | Module |
|---------|---------|--------|
| 0x08 | Loop counter / temp | Various |
| 0x0E | CC number being processed | midi_cc |
| 0x0F | CC value / data byte 2 | midi_cc |
| 0x10 | CC handler index | midi_cc |
| 0x17 | Timer 1 tick counter | Main loop |
| 0x34 | Current channel | voice/midi |
| 0x38 | Current parameter descriptor | envelope |
| 0x3A | Current voice slot ID | voice |
| 0x3B | Current IRAM state address | envelope |
| 0x41-42 | Mod wheel contribution | modulation |
| 0x4F | Parameter list iterator | envelope |
| 0x54 | Active voice list head | voice |
| 0x70+ | Parameter descriptor list | envelope |

### External RAM (XRAM) Key Regions

| Address | Size | Purpose | Module |
|---------|------|---------|--------|
| 0x1183 | 1 | Mod wheel value | modulation |
| 0x1184 | 8 | Mod wheel sensitivity/ch | modulation |
| 0x1194 | 16 | Pitch bend values (8×2) | modulation |
| 0x11E5 | 1 | Pitch bend range | midi_cc |
| 0x11EE | 8 | Voice pool entries | voice |
| 0x1200 | 96 | Per-channel state (8×12) | voice/midi |
| 0x12C0 | 1 | MIDI base channel | midi |
| 0x12C3 | 1 | SysEx parser state | midi |
| 0x12CC | 1 | Running status | midi |
| 0x12CE | 1 | RX buffer count | midi |
| 0x12CF | 1 | RX write pointer | midi |
| 0x12D0 | 1 | RX read pointer | midi |
| 0x13D4 | 255 | RX circular buffer | midi |
| 0x14D5 | 1024 | Voice status table (8×128) | voice |
| 0x1D15-17 | 3 | RPN state | midi_cc |
| 0x1D18-19 | 2 | SysEx masks | midi |
| 0x1D1A | 5 | Voice enable table 1 | midi |
| 0x1D20 | 5 | Voice enable table 2 | midi |
| 0x1D99 | 1 | Slow periodic counter | main |
| 0x1D9A | 1 | Active sensing timeout | midi |
| 0x1E20 | 8 | All-notes-off flags | midi |
| 0x1E50 | 8 | Sustain pedal values | midi |

## Extraction Steps

### Step 1: Create Include Files

1. Extract SAM8905 register definitions from sam_write_dram/sam_write_aram
2. Document P2=0x80 pattern for SAM access
3. Create sam_defs.inc with:
   - SAM_BASE EQU 0x8000
   - SAM_ADDR EQU 0x8000
   - SAM_DATA_LO EQU 0x8001
   - SAM_DATA_MID EQU 0x8002
   - SAM_DATA_HI EQU 0x8003
   - SAM_CTRL EQU 0x8004

### Step 2: Extract Core SAM Functions

```python
# Pseudo-code for extraction script
for func in ['sam_write_dram', 'sam_write_aram', 'sam_dram_clear_all']:
    asm = functions_disassemble(func_address)
    vars = functions_get_variables(func_address)
    refs = xrefs_list(to_addr=func_address)

    # Convert Ghidra disassembly to SDCC/ASXXXX format
    # Add header comments with:
    # - Input parameters (registers)
    # - Output/modified registers
    # - RAM usage
    # - Dependencies
```

### Step 3: Extract Voice Management

1. Start with voice_deactivate (simplest)
2. Then voice_kill_channel (calls deactivate)
3. Then voice_trigger_note (complex, many dependencies)
4. Document voice state structure at XRAM voice+0x00-0xFF

### Step 4: Extract Envelope Engine

1. Extract LFO sine table (memory_read 0x9833, 64 bytes)
2. Extract periodic_voice_update (core tick function)
3. Document 16-byte parameter state block format
4. Extract individual envelope handlers

### Step 5: Extract MIDI Layer

1. ISR_UART_HANDLER first (standalone)
2. SERIAL_HANDLER (depends on dispatch tables)
3. Individual message handlers
4. Extract CC and program lookup tables

### Step 6: Create Relocatable Modules

For each module:
1. Replace absolute addresses with EQU symbols
2. Replace LCALL with external declarations
3. Document public entry points
4. Create test harness

## Relocation Strategy

### Address Abstraction

Replace hardcoded addresses with symbols:
```asm
; Original
MOV DPTR,#0x1200

; Relocated
MOV DPTR,#CHANNEL_STATE_BASE
```

### Module Interface Pattern

```asm
;--------------------------------------------------------
; MODULE: voice_trigger.asm
;
; PUBLIC:
;   voice_trigger_note - Trigger note on/off for voice
;
; REQUIRES:
;   sam_write_dram (sam_dram.asm)
;   envelope_init (voice_envelope.asm)
;
; INPUTS:
;   R7 = channel (0-7)
;   A = note number (0-127)
;   0x0A = velocity (0-127, 0=note off)
;
; MODIFIES:
;   A, B, R0-R7, DPTR, PSW
;
; RAM USAGE:
;   IRAM 0x34 = channel
;   IRAM 0x3A = voice slot
;   XRAM VOICE_STATUS_BASE + ch*128 + note
;--------------------------------------------------------
```

## Validation

### Per-Module Tests

1. Create MAME test driver that loads extracted module
2. Compare behavior against original firmware
3. Verify SAM D-RAM/A-RAM writes match

### Integration Tests

1. Note on → verify D-RAM initialization
2. Note off → verify voice deactivation
3. CC#7 → verify envelope update
4. Sustain pedal → verify hold/release

## Estimated Effort

| Phase | Tasks | Effort |
|-------|-------|--------|
| 1. Core SAM | 5 functions, defs | 1 day |
| 2. Voice Mgmt | 8 functions | 2 days |
| 3. Envelope | 8 functions, tables | 2 days |
| 4. MIDI | 12 functions | 2 days |
| 5. Utilities | 7 functions | 0.5 day |
| 6. Data Tables | ~10 tables | 0.5 day |
| 7. Documentation | README, examples | 1 day |
| 8. Testing | Test harness | 1 day |

**Total: ~10 days**

## Tools Required

1. **Ghidra + MCP Bridge** - For extraction via tools listed above
2. **SDAS8051** (from SDCC) - Assembler for extracted code
3. **Python script** - Convert Ghidra disassembly to SDAS format
4. **MAME** - Validation against original firmware

## Next Steps

1. [ ] Create sam_defs.inc with register definitions
2. [ ] Extract and test sam_write_dram (simplest function)
3. [ ] Create disasm-to-sdas conversion script
4. [ ] Extract remaining Priority 1 functions
5. [ ] Document voice state structure from voice_trigger_note analysis
6. [ ] Extract Priority 2 functions with dependency tracking
7. [ ] Continue through priorities 3-6
8. [ ] Create integration test suite
9. [ ] Write usage documentation

## Related Documents

- WIP_solton_ms4.md - Main firmware analysis
- WIP_ms4_cc_sysex_analysis.md - MIDI implementation details
- sam8905_programmers_guide.md - SAM8905 D-RAM/A-RAM reference
