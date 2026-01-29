# MS4 Unnamed Functions Investigation

**Started**: 2026-01-29
**Source**: Callgraph analysis from MAINLOOP_da30 (depth 10)
**Firmware**: ms4_05_r1_0.bin

## Summary

**Phase 1**: 22 functions found via callgraph from MAINLOOP (depth 10).
**Phase 2**: 23 additional functions found via cc_switch_handler dispatch table.

All have been analyzed and names applied in Ghidra.

## Functions Analyzed

| Address | Original Name | Proposed Name | Category | Status |
|---------|---------------|---------------|----------|--------|
| 0xa4e6 | FUN_CODE_a4e6 | sam_dram_read | SAM I/O | ✓ Known |
| 0xa4ce | FUN_CODE_a4ce | sam_dram_write_wait | SAM I/O | ✓ Known |
| 0xa9cf | FUN_CODE_a9cf | slot_allocate | Voice Mgmt | ✓ Known |
| 0xd8b7 | FUN_CODE_d8b7 | init_midi_channel_tables | Init | ✓ Analyzed |
| 0xb6cd | FUN_CODE_b6cd | midi_tx_queue_byte | MIDI TX | ✓ Analyzed |
| 0xd889 | FUN_CODE_d889 | clear_pitch_bend_ch0_7 | Init | ✓ Analyzed |
| 0xdc8e | FUN_CODE_dc8e | multiply_add_scaled | Math | ✓ Analyzed |
| 0xdc1c | FUN_CODE_dc1c | table_search_match | Utility | ✓ Analyzed |
| 0xa50d | FUN_CODE_a50d | sam_dram_modify_ctrl | SAM I/O | ✓ Analyzed |
| 0xb49f | FUN_CODE_b49f | channel_voices_set_idle | Voice Mgmt | ✓ Analyzed |
| 0xaa12 | FUN_CODE_aa12 | voice_steal_find | Voice Mgmt | ✓ Analyzed |
| 0x9946 | FUN_CODE_9946 | voice_free | Voice Mgmt | ✓ Analyzed |
| 0x99bc | FUN_CODE_99bc | voice_slots_clear | Voice Mgmt | ✓ Analyzed |
| 0xdc48 | FUN_CODE_dc48 | table_search_nomatch | Utility | ✓ Analyzed |
| 0xa589 | FUN_CODE_a589 | voice_envelope_process | Envelope | ✓ Analyzed |
| 0xa693 | FUN_CODE_a693 | slot_clear_mod_depth | Voice Mgmt | ✓ Analyzed |
| 0xdcc5 | FUN_CODE_dcc5 | shift_left_n | Math | ✓ Analyzed |
| 0xc69f | FUN_CODE_c69f | sysex_byte1_handler | MIDI RX | ✓ Analyzed |
| 0xc6b6 | FUN_CODE_c6b6 | sysex_byte2_handler | MIDI RX | ✓ Analyzed |
| 0xc6c6 | FUN_CODE_c6c6 | sysex_byte3_handler | MIDI RX | ✓ Analyzed |
| 0xaa6f | FUN_CODE_aa6f | signed_multiply_sat | Math | ✓ Analyzed |
| 0xaa52 | FUN_CODE_aa52 | signed_multiply_chain | Math | ✓ Analyzed |

---

## CC Handler Functions (Phase 2)

Analysis of cc_switch_handler (0xc2ba) dispatch table at 0xc833.
Formula: `(CC_index - 5) * 3 + 0xc833` → LJMP to handler.

### Secondary Table (0xc824) - CC Indices 0-4

| Address | Original Name | Proposed Name | Purpose | Status |
|---------|---------------|---------------|---------|--------|
| 0xc1ce | FUN_CODE_c1ce | cc_custom_param_set | Set transpose/pitch param | ✓ Renamed |
| 0xc215 | UNKNOWN_FUN_CODE_c215 | cc_custom_param_inc | Increment parameter | ✓ Renamed |
| 0xc261 | FUN_CODE_c261 | cc_custom_param_dec | Decrement parameter | ✓ Renamed |
| 0xc2ab | FUN_CODE_c2ab | cc_rpn_lsb | RPN LSB (stores to 0x1d16) | ✓ Renamed |
| 0xc2b3 | FUN_CODE_c2b3 | cc_rpn_msb | RPN MSB (stores to 0x1d15) | ✓ Renamed |

### Main Table (0xc833) - CC Indices 5+

| Address | Original Name | Proposed Name | Purpose | Status |
|---------|---------------|---------------|---------|--------|
| 0xc2ca | FUN_CODE_c2ca | cc_portamento_time | Stores to 0x1184[ch] | ✓ Renamed |
| 0xc2d8 | FUN_CODE_c2d8 | cc_data_entry | Data Entry MSB (0x1d05[ch]) | ✓ Renamed |
| 0xc303 | FUN_CODE_c303 | cc_expression | Expression with all-notes-off | ✓ Renamed |
| 0xc34c | cc_volume_handler | cc_volume_handler | Volume (0x1202[ch]) | ✓ Already named |
| 0xc374 | FUN_CODE_c374 | cc_balance | Balance via lookup table | ✓ Renamed |
| 0xc390 | cc_sustain_pedal | cc_sustain_pedal | Sustain (0x1e50[ch]) | ✓ Already named |
| 0xc3c4 | FUN_CODE_c3c4 | cc_sostenuto | Sostenuto pedal (0x1cf5[ch]) | ✓ Renamed |
| 0xc3fe | FUN_CODE_c3fe | cc_soft_pedal | Soft pedal flag (0x1ce5[ch]) | ✓ Renamed |
| 0xc41f | FUN_CODE_c41f | cc_all_sound_off | Calls handle_note_off_sustain | ✓ Renamed |
| 0xc428 | FUN_CODE_c428 | cc_reset_controllers | Panic with flag 0 | ✓ Renamed |
| 0xc42f | FUN_CODE_c42f | cc_local_control | Panic with flag 1 | ✓ Renamed |
| 0xc436 | FUN_CODE_c436 | cc_all_notes_off | All notes off, sets 0x1e20=1 | ✓ Renamed |
| 0xc449 | FUN_CODE_c449 | cc_omni_mode_off | All notes off, sets 0x1e20=0 | ✓ Renamed |

### MIDI Message Handlers

| Address | Original Name | Proposed Name | Purpose | Status |
|---------|---------------|---------------|---------|--------|
| 0xc78a | FUN_CODE_c78a | pitch_bend_init_wrapper | Init pitch bend (R3=0) | ✓ Renamed |
| 0xc7d2 | FUN_CODE_c7d2 | midi_program_change_handler | Calls handle_program_change | ✓ Renamed |
| 0xc7e5 | FUN_CODE_c7e5 | cc_pitch_bend_range | Pitch bend range (0x11c8[ch]) | ✓ Renamed |
| 0xc801 | FUN_CODE_c801 | cc_rpn_pitch_bend_apply | Apply pitch bend to ch & ch+4 | ✓ Renamed |

### Helper Functions

| Address | Original Name | Proposed Name | Purpose | Status |
|---------|---------------|---------------|---------|--------|
| 0xbef9 | FUN_CODE_bef9 | sostenuto_release | Release sostenuto-held voices | ✓ Renamed |
| 0xc0d8 | FUN_CODE_c0d8 | sostenuto_engage | Mark voices for sostenuto hold | ✓ Renamed |
| 0x9ab0 | FUN_CODE_9ab0 | pitch_bend_scale_apply | Scale pitch bend by range | ✓ Renamed |

---

## Detailed Analysis

### Initialization Functions

#### init_midi_channel_tables (0xd8b7)
**Called from**: MAINLOOP
**Purpose**: Initialize MIDI channel data tables for all 16 channels
**Operations**:
- Clears TI flag
- For channels 0-15:
  - XRAM[0x1d05+ch] = 0x40 (volume default)
  - XRAM[0x1e50+ch] = 0 (expression)
  - XRAM[0x11c4+ch] = 0 (unknown)
  - XRAM[0x1184+ch] = 0 (mod wheel)
  - pitch_bend_table[ch] = 0 (2 bytes)
  - XRAM[0x1e20+ch] = 0 (aftertouch)
- Sets final state flags

#### clear_pitch_bend_ch0_7 (0xd889)
**Called from**: MAINLOOP (after panic)
**Purpose**: Clear pitch bend for channels 0-7 only
**Operations**: Subset of init_midi_channel_tables

---

### Voice Management Functions

#### slot_allocate (0xa9cf) - CONFIRMED
**Called from**: voice_init_slots
**Purpose**: Allocate D-RAM slots from free list
**Algorithm**:
1. Check if enough slots available (slot_count <= dram_slot_count)
2. If not, call voice_steal_find to free a voice
3. Remove slots from free list, add to active list
4. Update pending_voice_list pointer
5. Return first allocated slot number

#### voice_steal_find (0xaa12)
**Called from**: slot_allocate
**Purpose**: Find a voice to steal when slots exhausted
**Algorithm**:
1. Walk active voice list
2. Check voice flags (0xFB bits 3,5) for steal eligibility
3. If stealable found, call voice_free
4. Second pass: check for voices with non-zero 0xF8

#### voice_free (0x9946)
**Called from**: voice_steal_find, periodic_voice_update
**Purpose**: Remove voice from active list, return to free list
**Algorithm**:
1. Validate voice_page_num (must be 0x00-0x0F)
2. Call voice_slots_clear to release SAM slots
3. Unlink from active_voice_list
4. Update pending_voice_list if needed
5. Add to dram_slot_free_list
6. Increment dram_slot_count

#### voice_slots_clear (0x99bc)
**Called from**: voice_free
**Purpose**: Clear all D-RAM slots for a voice
**Operations**:
1. Call sam_dram_write_word15 (set IDLE)
2. Clear slot assignment bytes in voice page
3. Write zeros to SAM D-RAM (addr, data1-3, ctrl)
4. Loop for all 15 slot addresses

#### channel_voices_set_idle (0xb49f)
**Called from**: voice_assign_algorithm
**Purpose**: Set IDLE bit for all voices on a channel
**Algorithm**: Walk active list, call sam_dram_write_word15 for matching channel

#### slot_clear_mod_depth (0xa693)
**Called from**: voice_envelope_process
**Purpose**: Clear modulation depth byte (offset 6 in slot)
**Simple**: `*(voice_slot_base + 6) = 0`

---

### SAM I/O Functions

#### sam_dram_modify_ctrl (0xa50d)
**Called from**: voice_deactivate
**Purpose**: Read-modify-write D-RAM control word
**Operations**:
1. P2 = 0x80 (select SAM)
2. Read word at address (slot<<4)|0x0F
3. Modify data with param OR 0x80
4. Write back with wait
5. Restore P2

---

### Envelope/Voice Processing

#### voice_envelope_process (0xa589)
**Called from**: voice_trigger_note
**Purpose**: Complex envelope state machine processing
**Operations**:
- Check voice flags (0x20 = active)
- Process envelope segments for up to 7 slots
- Handle envelope rates and targets
- Parse handler type bytes
- Call slot_clear_mod_depth for certain states
- Large function (~180 bytes)

---

### MIDI TX Functions

#### midi_tx_queue_byte (0xb6cd)
**Called from**: MAINLOOP
**Purpose**: Add byte to MIDI transmit ring buffer
**Operations**:
1. Check buffer not full (0x12D1 < 0xFF)
2. Disable interrupts (EA=0)
3. Store byte at XRAM[0x12D5 + write_ptr]
4. Increment write pointer (wrap at 0xFF)
5. Trigger TI if needed
6. Re-enable interrupts

---

### MIDI RX (SysEx) Functions

#### sysex_byte1_handler (0xc69f)
**Called from**: SERIAL_HANDLER
**Purpose**: Handle first SysEx data byte
**Sets**: State to 2 or 4 based on manufacturer ID

#### sysex_byte2_handler (0xc6b6)
**Called from**: SERIAL_HANDLER
**Purpose**: Handle second SysEx data byte (voice mask low)
**Stores**: Byte to XRAM[0x1D18]

#### sysex_byte3_handler (0xc6c6)
**Called from**: SERIAL_HANDLER
**Purpose**: Handle third SysEx data byte (voice mask high)
**Stores**: Byte to XRAM[0x1D19]
**Calls**: sysex_voice_enable

---

### Math/Utility Functions

#### multiply_add_scaled (0xdc8e)
**Purpose**: Fixed-point multiply-add
**Formula**: `result = param2 * param1 + (param1 * param3) >> 8`
**Use**: Interpolation, scaling calculations

#### signed_multiply_sat (0xaa6f)
**Called from**: periodic_voice_update
**Purpose**: Signed 8-bit multiply with saturation
**Algorithm**:
1. Handle sign of both operands
2. Multiply with scaling: `(param2 << 1 | 1) * param1 >> 8`
3. Add to param2
4. Saturate to ±127 on overflow

#### signed_multiply_chain (0xaa52)
**Called from**: dram_slot_apply_mod_depth
**Purpose**: Chained signed multiply (3 operands)
**Formula**: `saturate(param3 + scale(scale(param1, param2), param3))`
**Use**: Modulation depth calculations with multiple scaling factors

#### shift_left_n (0xdcc5)
**Purpose**: Shift param1 left by param2 bits
**Use**: Bit mask generation for SysEx voice enable

#### table_search_match (0xdc1c)
**Called from**: handle_program_change
**Purpose**: Search table for matching value (XOR compare)
**Returns**: Index offset if found

#### table_search_nomatch (0xdc48)
**Called from**: handle_all_notes_off
**Purpose**: Search table for first non-matching value
**Returns**: Index offset if found

---

## Call Hierarchy Summary

```
MAINLOOP_da30
├── init_midi_channel_tables (0xd8b7) - NEW
├── midi_tx_queue_byte (0xb6cd) - NEW
├── clear_pitch_bend_ch0_7 (0xd889) - NEW
├── SERIAL_HANDLER
│   ├── sysex_byte1_handler (0xc69f) - NEW
│   ├── sysex_byte2_handler (0xc6b6) - NEW
│   └── sysex_byte3_handler (0xc6c6) - NEW
│       └── shift_left_n (0xdcc5) - NEW
├── handle_program_change
│   ├── table_search_match (0xdc1c) - NEW
│   └── voice_kill_channel
│       └── voice_deactivate
│           └── sam_dram_modify_ctrl (0xa50d) - NEW
├── init_channel_defaults
│   └── multiply_add_scaled (0xdc8e) - NEW
├── voice_init_slots
│   └── slot_allocate (0xa9cf) - CONFIRMED
│       └── voice_steal_find (0xaa12) - NEW
│           └── voice_free (0x9946) - NEW
│               └── voice_slots_clear (0x99bc) - NEW
├── periodic_voice_update
│   ├── signed_multiply_sat (0xaa6f) - NEW
│   └── dram_slot_apply_mod_depth
│       └── signed_multiply_chain (0xaa52) - NEW
├── handle_all_notes_off
│   ├── table_search_nomatch (0xdc48) - NEW
│   └── voice_trigger_note
│       └── voice_envelope_process (0xa589) - NEW
│           └── slot_clear_mod_depth (0xa693) - NEW
└── channel_voices_set_idle (0xb49f) - NEW
```

---

## Ghidra Rename Commands

```python
# Run these in Ghidra to apply names:
functions_rename(address="0xd8b7", new_name="init_midi_channel_tables")
functions_rename(address="0xb6cd", new_name="midi_tx_queue_byte")
functions_rename(address="0xd889", new_name="clear_pitch_bend_ch0_7")
functions_rename(address="0xdc8e", new_name="multiply_add_scaled")
functions_rename(address="0xdc1c", new_name="table_search_match")
functions_rename(address="0xa50d", new_name="sam_dram_modify_ctrl")
functions_rename(address="0xb49f", new_name="channel_voices_set_idle")
functions_rename(address="0xaa12", new_name="voice_steal_find")
functions_rename(address="0x9946", new_name="voice_free")
functions_rename(address="0x99bc", new_name="voice_slots_clear")
functions_rename(address="0xdc48", new_name="table_search_nomatch")
functions_rename(address="0xa589", new_name="voice_envelope_process")
functions_rename(address="0xa693", new_name="slot_clear_mod_depth")
functions_rename(address="0xdcc5", new_name="shift_left_n")
functions_rename(address="0xc69f", new_name="sysex_byte1_handler")
functions_rename(address="0xc6b6", new_name="sysex_byte2_handler")
functions_rename(address="0xc6c6", new_name="sysex_byte3_handler")
functions_rename(address="0xaa6f", new_name="signed_multiply_sat")
functions_rename(address="0xaa52", new_name="signed_multiply_chain")
# Already known:
functions_rename(address="0xa4e6", new_name="sam_dram_read")
functions_rename(address="0xa4ce", new_name="sam_dram_write_wait")
functions_rename(address="0xa9cf", new_name="slot_allocate")

# Phase 2: CC Handlers (applied)
functions_rename(address="0xc1ce", new_name="cc_custom_param_set")
functions_rename(address="0xc215", new_name="cc_custom_param_inc")
functions_rename(address="0xc261", new_name="cc_custom_param_dec")
functions_rename(address="0xc2ab", new_name="cc_rpn_lsb")
functions_rename(address="0xc2b3", new_name="cc_rpn_msb")
functions_rename(address="0xc2ca", new_name="cc_portamento_time")
functions_rename(address="0xc2d8", new_name="cc_data_entry")
functions_rename(address="0xc303", new_name="cc_expression")
functions_rename(address="0xc374", new_name="cc_balance")
functions_rename(address="0xc3c4", new_name="cc_sostenuto")
functions_rename(address="0xc3fe", new_name="cc_soft_pedal")
functions_rename(address="0xc41f", new_name="cc_all_sound_off")
functions_rename(address="0xc428", new_name="cc_reset_controllers")
functions_rename(address="0xc42f", new_name="cc_local_control")
functions_rename(address="0xc436", new_name="cc_all_notes_off")
functions_rename(address="0xc449", new_name="cc_omni_mode_off")
functions_rename(address="0xc78a", new_name="pitch_bend_init_wrapper")
functions_rename(address="0xc7d2", new_name="midi_program_change_handler")
functions_rename(address="0xc7e5", new_name="cc_pitch_bend_range")
functions_rename(address="0xc801", new_name="cc_rpn_pitch_bend_apply")
functions_rename(address="0xbef9", new_name="sostenuto_release")
functions_rename(address="0xc0d8", new_name="sostenuto_engage")
functions_rename(address="0x9ab0", new_name="pitch_bend_scale_apply")
```

---

## Phase 3: Remaining FUN_CODE Functions

Cleanup of remaining FUN_CODE_xxxx functions found after Phase 2.

### Timer/ISR Functions

| Address | Original Name | New Name | Purpose | Status |
|---------|---------------|----------|---------|--------|
| 0xd440 | FUN_CODE_d440 | timer1_isr_reload | Timer1 ISR - reloads TH1=0xe3, TL1=0x5b | ✓ Renamed |

### Voice Processing Functions

| Address | Original Name | New Name | Purpose | Status |
|---------|---------------|----------|---------|--------|
| 0xb1da | FUN_CODE_b1da | sam_dram_write_idle | Write idle control (0x43,0xea,0xff) to SAM | ✓ Renamed |
| 0xb2f1 | FUN_CODE_b2f1 | voice_note_zone_process | Process note zones/splits | ✓ Renamed |
| 0xb42a | FUN_CODE_b42a | voice_slot_assign_note | Assign MIDI note to voice slot | ✓ Renamed |

### MIDI Functions

| Address | Original Name | New Name | Purpose | Status |
|---------|---------------|----------|---------|--------|
| 0xc6fd | FUN_CODE_c6fd | midi_message_dispatch | MIDI message type dispatcher | ✓ Renamed |
| 0xc69d | thunk_FUN_CODE_c6fd | thunk_midi_message_dispatch | Thunk to dispatcher | ✓ Renamed |
| 0xc78f | FUN_CODE_c78f | midi_note_on_off_wrapper | Wrapper for note on/off | ✓ Renamed |

### Utility Functions

| Address | Original Name | New Name | Purpose | Status |
|---------|---------------|----------|---------|--------|
| 0xd4ff | FUN_CODE_d4ff | calc_param_block_size | Calculate parameter block size | ✓ Renamed |
| 0xd526 | UNKNOWN_FUN_CODE_d526 | program_data_copy | Complex data copy, sets T1=1 | ✓ Renamed |
| 0xd7a3 | UNKNOWN_FUN_CODE_d7a3 | memcpy_to_xram | Copy bytes to XRAM with P2 bank | ✓ Renamed |

### Padding/Data (Not Real Code)

| Address | Original Name | New Name | Reason | Status |
|---------|---------------|----------|--------|--------|
| 0x0040 | FUN_CODE_0040 | DATA_program_ptr_table | Program pointer table location | ✓ Renamed |
| 0x00df | FUN_CODE_00df | DATA_or_garbled_00df | Decompiler confusion/data | ✓ Renamed |
| 0x2e5b | FUN_CODE_2e5b | DATA_or_garbled_2e5b | Decompiler confusion/data | ✓ Renamed |
| 0xc937 | FUN_CODE_c937 | UNUSED_padding_c937 | NOP padding, not real code | ✓ Renamed |
| 0xca58 | FUN_CODE_ca58 | UNUSED_padding_ca58 | NOP padding, not real code | ✓ Renamed |

---

## Statistics

### Phase 1 (MAINLOOP callgraph)
- **Total functions in callgraph**: 60
- **Already named**: 38
- **Newly analyzed**: 22
- **Coverage from MAINLOOP**: ~100% of reachable functions now named

### Phase 2 (CC dispatch handlers)
- **CC handler functions**: 18
- **Helper functions**: 5
- **Total new renames**: 23

### Phase 3 (Remaining FUN_CODE cleanup)
- **Real functions renamed**: 10
- **Data/padding marked**: 5
- **Total Phase 3**: 15

### Overall
- **Total functions renamed**: 60
- **Remaining FUN_CODE_xxxx**: 0 (all functions named!)
- **Status**: ✓ COMPLETE

