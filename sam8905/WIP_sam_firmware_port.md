# SAM8905 Controller Firmware - C Port

Low-level C implementation of the DREAM S.A. SAM8905 controller firmware, targeting
SDCC for 8-bit architectures. The goal is a 1:1 translation that preserves the
original firmware's structure and behavior.

## Background

The SAM8905 DSP chip was sold by DREAM S.A. along with reference firmware for 8051
microcontrollers. This firmware (or close variants) appears in multiple products:

- **Solton/Hohner MS4** - Best analyzed, used as reference (ms4_05_r1_0.bin)
- **Wersi Keyfox 10** - Similar structure, different program data
- **Hohner XE9/XE9L** - Similar structure, different program data

All share the same hardware pattern: 8051 + SAM8905 + XRAM, with MIDI control.
The C port aims to be usable across all these platforms.

**Reference documentation (MS4-specific analysis):**
- `WIP_ms4_memory_init.md` - INTMEM/EXTMEM memory maps, boot sequence
- `WIP_solton_ms4.md` - Function addresses, MIDI handlers, envelope system

## Device Variations

| Feature | MS4 | Keyfox 10 | XE9/XE9L |
|---------|-----|-----------|----------|
| CPU | 80C32 | 80C32 | 80C32 |
| ROM size | 64KB | 128KB (CODE+DATA) | 64KB |
| RAM size | 8KB | 32KB | 8KB |
| MIDI | Hardware UART | Bit-banged | Hardware UART |
| Programs | 66 | 97 | TBD |
| Program table | CODE:0x0040 | DATA:0x6285 | TBD |
| Sound table | N/A | DATA:0xFFA0 | TBD |
| SAM base | 0x8000 | 0x8000 | 0x8000 |

The core voice management, envelope, and SAM interface code is largely identical.
Main differences are:
- Program/sound data location and format
- MIDI implementation (hardware vs bit-banged)
- Available RAM for voice pages
- Number of simultaneous voices

## Design Goals

1. **Structural fidelity**: Mirror the original assembly as closely as possible
2. **SDCC compatibility**: Target 8-bit architectures (8051, Z80, etc.)
3. **No dynamic allocation**: All memory statically defined
4. **Index-based parameters**: Use uint8_t indices instead of pointers
5. **Global memory structs**: INTMEM and EXTMEM as global structs
6. **Incremental porting**: Port function-by-function with verification

## SDCC Considerations

```c
// Use fixed-width types
#include <stdint.h>

// Avoid pointers in function parameters - use indices
void process_voice(uint8_t voice_page);      // Good
void process_voice(voice_t *voice);          // Avoid

// Use __xdata, __idata, __code for memory spaces when targeting 8051
__xdata uint8_t extmem[0x2000];
__idata uint8_t intmem[0x80];
__code const uint8_t sine_table[64];

// For portable code, use macros
#ifdef __SDCC
  #define XDATA __xdata
  #define IDATA __idata
  #define CODE __code
#else
  #define XDATA
  #define IDATA
  #define CODE const
#endif
```

## Memory Architecture

### INTMEM (128 bytes) - `intmem_t`

Based on WIP_ms4_memory_init.md. Directly maps 8051 internal RAM 0x00-0x7F.

```c
typedef struct {
    // Register banks (0x00-0x1F)
    union {
        uint8_t raw[0x20];
        struct {
            uint8_t bank0[8];      // 0x00-0x07: R0-R7 (working regs)
            uint8_t bank1[8];      // 0x08-0x0F: MIDI handler params
            uint8_t bank2[8];      // 0x10-0x17: MIDI parser / timer ticks
            uint8_t bank3[8];      // 0x18-0x1F: unused
        };
    } regs;

    // Bit-addressable area (0x20-0x2F)
    uint8_t flags_20;             // 0x20: computation/voice flags
    uint8_t flags_21;             // 0x21: MIDI/system flags
    uint8_t flags_22;             // 0x22: voice/timer flags
    uint8_t flags_23;             // 0x23: (TBD)
    uint8_t flags_24;             // 0x24: (TBD)
    uint8_t flags_25;             // 0x25: (TBD)
    uint8_t flags_26;             // 0x26: (TBD)
    uint8_t flags_27;             // 0x27: (TBD)
    uint8_t flags_28;             // 0x28: (TBD)
    uint8_t flags_29;             // 0x29: (TBD)
    uint8_t flags_2a;             // 0x2A: (TBD)
    uint8_t flags_2b;             // 0x2B: (TBD)
    uint8_t flags_2c;             // 0x2C: (TBD)
    uint8_t flags_2d;             // 0x2D: (TBD)
    uint8_t flags_2e;             // 0x2E: (TBD)

    // Copy/utility variables (0x2F-0x33)
    uint8_t copy_count;           // 0x2F
    uint8_t copy_src_lo;          // 0x30
    uint8_t copy_src_hi;          // 0x31
    uint8_t copy_dst_lo;          // 0x32
    uint8_t copy_dst_hi;          // 0x33

    // Voice system variables (0x34-0x56)
    uint8_t current_slot_id;      // 0x34
    uint8_t program_base_dph;     // 0x35
    uint8_t program_base_dpl;     // 0x36
    uint8_t sam_ctrl_flags;       // 0x37
    uint8_t dram_address_counter; // 0x38
    uint8_t remaining_slots;      // 0x39
    uint8_t voice_page_num;       // 0x3A
    uint8_t voice_slot_base;      // 0x3B
    uint8_t midi_note;            // 0x3C
    uint8_t midi_velocity;        // 0x3D
    uint8_t amplitude_scale;      // 0x3E
    uint8_t velocity_mix_atten;   // 0x3F
    uint8_t pitch_bend_value;     // 0x40
    uint8_t rom_data_ptr_lo;      // 0x41
    uint8_t rom_data_ptr_hi;      // 0x42
    uint8_t dram_entry_lo;        // 0x43
    uint8_t dram_entry_hi;        // 0x44
    uint8_t voice_data_ptr_lo;    // 0x45
    uint8_t voice_data_ptr_hi;    // 0x46
    uint8_t voice_state_ptr_lo;   // 0x47
    uint8_t voice_state_ptr_hi;   // 0x48
    uint8_t voice_state_byte;     // 0x49
    uint8_t slot_count;           // 0x4A
    uint8_t velocity_curve_ptr;   // 0x4B
    uint8_t portamento_value;     // 0x4C
    uint8_t _pad_4d;              // 0x4D: (TBD)
    uint8_t voice_list_prev;      // 0x4E
    uint8_t dram_slot_index;      // 0x4F
    uint8_t _pad_50;              // 0x50: (TBD)
    uint8_t lfsr_state;           // 0x51
    uint8_t octave_shift;         // 0x52
    uint8_t dram_slot_free_list;  // 0x53
    uint8_t active_voice_list_head; // 0x54
    uint8_t pending_voice_list;   // 0x55
    uint8_t dram_slot_count;      // 0x56

    // Per-channel tables (0x57-0x77)
    uint8_t channel_split_program[4];  // 0x57-0x5A
    uint8_t _pad_5b_66[12];            // 0x5B-0x66: (TBD)
    uint8_t channel_current_prog[4];   // 0x67-0x6A
    uint8_t _pad_6b_76[12];            // 0x6B-0x76: (TBD)
    uint8_t scratch_77;                // 0x77

    // Table search parameters (0x78-0x7C)
    uint8_t search_ptr_hi;        // 0x78
    uint8_t search_ptr_lo;        // 0x79
    uint8_t search_value;         // 0x7A
    uint8_t search_count_hi;      // 0x7B
    uint8_t search_count_lo;      // 0x7C

    uint8_t _pad_7d;              // 0x7D: (TBD)

    // D-RAM free list (0x7E-0x8D) - extends beyond 0x80!
    // Note: 8051 INTMEM is only 0x00-0x7F, but we model the full range
    // used by the firmware. On 8051, 0x80+ would be SFRs accessed differently.
} intmem_t;

// Separate array for the extended INTMEM area (0x7E-0xB2)
// On real 8051, this would be indirect addressing into upper IRAM
typedef struct {
    uint8_t dram_free_list[16];        // 0x7E-0x8D
    uint8_t channel_algorithm[16];     // 0x8E-0x9D
    uint8_t algorithm_slot_lookup[8];  // 0x9E-0xA5
    uint8_t _pad_a6_a8[3];             // 0xA6-0xA8: (TBD) - overlap with note tracking?
    uint8_t channel_highest_note[4];   // 0x9F-0xA2 (note: overlaps with above!)
    uint8_t _pad_a3_ae[12];            // 0xA3-0xAE: (TBD)
    uint8_t channel_note_count[4];     // 0xAF-0xB2
} intmem_upper_t;
```

**Note**: The 8051 has 256 bytes of IRAM (0x00-0xFF) but only 0x00-0x7F is directly
addressable. 0x80-0xFF requires indirect addressing (@R0/@R1). The firmware uses
addresses 0x7E-0xB2 which spans both regions.

### EXTMEM (8KB) - `extmem_t`

Based on WIP_ms4_memory_init.md. Maps 8051 external RAM 0x0000-0x1FFF.

```c
typedef struct {
    // Voice pages (0x0000-0x0FFF) - 16 pages × 256 bytes
    uint8_t voice_page[16][256];

    // Pitch tables (0x1000-0x117F)
    uint8_t pitch_table_lo[128];       // 0x1000-0x107F
    uint8_t pitch_table_mid[128];      // 0x1080-0x10FF
    uint8_t pitch_table_hi[128];       // 0x1100-0x117F

    // Modulation state (0x1180-0x11B3)
    uint8_t mod_lfo_rate;              // 0x1180
    uint8_t mod_lfo_phase_lo;          // 0x1181
    uint8_t mod_lfo_phase_hi;          // 0x1182
    uint8_t mod_lfo_output;            // 0x1183
    uint8_t mod_wheel_sensitivity[16]; // 0x1184-0x1193
    uint8_t pitch_bend_table[32];      // 0x1194-0x11B3 (16 × 2 bytes)

    // Voice control areas (0x11B4-0x11FF)
    uint8_t _region_11b4[33];          // 0x11B4-0x11D4: (TBD)
    uint8_t voice_ctrl_area_1[16];     // 0x11D5-0x11E4
    uint8_t _pad_11e5;                 // 0x11E5: (TBD)
    uint8_t program_init_copy[8];      // 0x11E6-0x11ED
    uint8_t algorithm_pool[8];         // 0x11EE-0x11F5
    uint8_t _region_11f6[10];          // 0x11F6-0x11FF: (TBD)

    // Channel data (0x1200-0x12BF)
    uint8_t channel_data[192];         // 0x1200-0x12BF: (TBD - needs characterization)

    // MIDI subsystem (0x12C0-0x14D4)
    uint8_t midi_base_channel;         // 0x12C0
    uint8_t _midi_12c1[2];             // 0x12C1-0x12C2: (TBD)
    uint8_t sysex_state;               // 0x12C3
    uint8_t _midi_12c4[8];             // 0x12C4-0x12CB: (TBD)
    uint8_t midi_running_status;       // 0x12CC
    uint8_t midi_rx_read_pos;          // 0x12CD
    uint8_t midi_rx_count;             // 0x12CE
    uint8_t midi_rx_write_pos;         // 0x12CF
    uint8_t _midi_12d0;                // 0x12D0: (TBD)
    uint8_t midi_tx_count;             // 0x12D1
    uint8_t midi_tx_write_pos;         // 0x12D2
    uint8_t midi_tx_read_pos;          // 0x12D3
    uint8_t midi_current_byte;         // 0x12D4
    uint8_t midi_tx_buffer[255];       // 0x12D5-0x13D3
    uint8_t midi_rx_buffer[255];       // 0x13D4-0x14D2
    uint8_t current_program_base[2];   // 0x14D3-0x14D4 (little-endian ptr)

    // Extended voice pages (0x14D5-0x1CD4) - 8 pages × 256 bytes
    uint8_t extended_voice_page[8][256];

    // Voice control area 2 (0x1CD5-0x1CE4)
    uint8_t voice_ctrl_area_2[16];

    // Uncharacterized region (0x1CE5-0x1FFF)
    uint8_t _region_1ce5[795];

} extmem_t;
```

### Global State

```c
// Global memory - platform-specific qualifiers
IDATA intmem_t g_intmem;
IDATA intmem_upper_t g_intmem_upper;  // 0x7E-0xB2
XDATA extmem_t g_extmem;

// Current register bank (for functions that need it)
static uint8_t g_current_bank;

// Macro to access bank registers
#define BANK_R(bank, reg) g_intmem.regs.raw[(bank) * 8 + (reg)]
#define R0 BANK_R(g_current_bank, 0)
#define R1 BANK_R(g_current_bank, 1)
// ... etc

// Timer tick counters (Bank 2 R6/R7)
#define TICK_SLOW  g_intmem.regs.raw[0x16]
#define TICK_FAST  g_intmem.regs.raw[0x17]
```

## Bit Flag Definitions

```c
// flags_20 (0x20) - Computation/Voice flags
#define FLAG_ROM_ACCESS_MODE    0x02  // bit 1: MOVC vs MOVX
#define FLAG_SIGN               0x04  // bit 2: negative value indicator
#define FLAG_LFO_BYPASS         0x08  // bit 3: LFO waveform bypass
#define FLAG_VOICE_RELEASE      0x10  // bit 4: voice in release phase
#define FLAG_VOICE_ALLOC_ACTIVE 0x40  // bit 6: voice allocation in progress
#define FLAG_AMPLITUDE_MODE     0x80  // bit 7: amplitude special update

// flags_21 (0x21) - MIDI/System flags
#define FLAG_ENV_UPDATE         0x01  // bit 0: envelope needs D-RAM update
#define FLAG_LAYER_SELECT       0x02  // bit 1: dual-voice mode
#define FLAG_ALGO_POOL_FULL     0x04  // bit 2: algorithm pool exhausted
#define FLAG_TX_IDLE            0x08  // bit 3: TX buffer empty
#define FLAG_OMNI_MODE          0x10  // bit 4: receive all channels
#define FLAG_TX_OVERFLOW        0x20  // bit 5: TX buffer overflow
#define FLAG_RX_OVERFLOW        0x40  // bit 6: RX buffer overflow
#define FLAG_MIDI_DATA2_PENDING 0x80  // bit 7: waiting for 2nd data byte

// flags_22 (0x22) - Voice/Timer flags
#define FLAG_T1_SAVE            0x01  // bit 0: saved T1 state
#define FLAG_VOICE_ENABLE_CHG   0x10  // bit 4: voice enable changed
#define FLAG_NOTE_TRIGGER       0x20  // bit 5: note trigger pending
#define FLAG_MULTI_VOICE        0x40  // bit 6: multi-voice mode
```

## SAM8905 Interface

```c
// SAM register indices (offset from base 0x8000)
#define SAM_REG_DATA     0  // 0x8000: address/data
#define SAM_REG_DATA1    1  // 0x8001: data byte 1
#define SAM_REG_DATA2    2  // 0x8002: data byte 2
#define SAM_REG_DATA3    3  // 0x8003: data byte 3
#define SAM_REG_CTRL     4  // 0x8004: control

// SAM control byte values
#define SAM_CTRL_RESET   0x40
#define SAM_CTRL_DRAM_WR 0x05
#define SAM_CTRL_ARAM_WR 0x03

// Hardware abstraction (implement per platform)
void sam_write_reg(uint8_t reg, uint8_t value);
uint8_t sam_read_reg(uint8_t reg);

// Higher-level SAM operations
void sam_write_dram(uint8_t addr, uint16_t value);
void sam_write_aram(uint8_t slot, const uint8_t *data);  // 64 bytes
```

## ROM Data Access

```c
// ROM is up to 64KB at CODE space (actual size varies by device)
CODE const uint8_t g_rom[0x10000];

// Access macros
#define ROM_BYTE(addr) g_rom[(addr)]
#define ROM_WORD_LE(addr) ((uint16_t)g_rom[(addr)] | ((uint16_t)g_rom[(addr)+1] << 8))
#define ROM_WORD_BE(addr) (((uint16_t)g_rom[(addr)] << 8) | (uint16_t)g_rom[(addr)+1])

// Program pointer table (device-specific location and count)
// MS4: 0x0040, 66 programs
// Keyfox: 0x6285, 97 programs (in DATA space)
// These should be defined in device-specific config header
extern const uint16_t PROGRAM_PTR_TABLE;
extern const uint8_t NUM_PROGRAMS;

static inline uint16_t get_program_ptr(uint8_t program_num) {
    uint16_t addr = PROGRAM_PTR_TABLE + program_num * 2;
    return ROM_WORD_BE(addr);
}
```

## Function Porting Strategy

Port functions in dependency order, starting with leaf functions (no calls to other
unported functions). Use the original addresses as function name suffixes for traceability.

### Phase 1: Utilities and Memory Access

| Original | C Function | Status |
|----------|-----------|--------|
| DCA8 | `dptr_add_r6r7()` | [ ] |
| DCB3 | `add_a_to_dptr()` | [ ] |
| DC1C | `table_search_match()` | [ ] |
| DC9C | `load_dptr_from_xram()` | [ ] |

### Phase 2: SAM Interface

| Original | C Function | Status |
|----------|-----------|--------|
| A4BC | `sam_write_dram()` | [ ] |
| AD43 | `sam_write_aram()` | [ ] |
| A53C | `sam_dram_clear_all()` | [ ] |
| A523 | `sam_dram_write_word15()` | [ ] |

### Phase 3: Memory Initialization

| Original | C Function | Status |
|----------|-----------|--------|
| D454 | `extmem_clear_all()` | [ ] |
| B70B | `voice_pages_clear()` | [ ] |
| 9904 | `slot_manager_init()` | [ ] |
| 9B16 | `pitch_table_init()` | [ ] |
| D463 | `ram_self_test()` | [ ] |
| 98AD | `init_pitch_and_voices()` | [ ] |

### Phase 4: MIDI Subsystem

| Original | C Function | Status |
|----------|-----------|--------|
| B630 | `isr_uart_handler()` | [ ] |
| C635 | `serial_handler()` | [ ] |
| B75F | `handle_note_on_off()` | [ ] |
| C1A0 | `cc_dispatch()` | [ ] |
| C45B | `handle_program_change()` | [ ] |
| 9AB0 | `handle_pitch_bend()` | [ ] |
| BDC | `midi_panic_all_channels()` | [ ] |

### Phase 5: Voice Management

| Original | C Function | Status |
|----------|-----------|--------|
| 9A2D | `voice_init_slots()` | [ ] |
| AB40 | `voice_init_next_slot()` | [ ] |
| AB73 | `voice_init_copy_and_envelope()` | [ ] |
| AB4C | `dram_config_dispatch()` | [ ] |
| B4BF | `voice_assign_algorithm()` | [ ] |
| A785 | `voice_kill_channel()` | [ ] |
| A69C | `voice_deactivate()` | [ ] |

### Phase 6: D-RAM Config Handlers

| Original | C Function | Status |
|----------|-----------|--------|
| AD8F | `dram_config_handler_00()` | [ ] |
| ADBD | `dram_config_handler_08()` | [ ] |
| B030 | `dram_config_handler_10()` | [ ] |
| B222 | `dram_config_handler_18()` | [ ] |
| B278 | `dram_config_handler_20()` | [ ] |
| B2D2 | `dram_config_handler_28()` | [ ] |
| B2CF | `dram_config_handler_30()` | [ ] |

### Phase 7: Envelope/Modulation System

| Original | C Function | Status |
|----------|-----------|--------|
| 9BA7 | `periodic_voice_update()` | [ ] |
| 9FCD | `modulation_write_dram()` | [ ] |
| A403 | `envelope_tick_volume()` | [ ] |
| A471 | `envelope_write_dram()` | [ ] |
| A18F | `volume_envelope_update()` | [ ] |

### Phase 8: Main Loop

| Original | C Function | Status |
|----------|-----------|--------|
| D440 | `timer1_isr()` | [ ] |
| DA30 | `main_loop()` | [ ] |
| DCBC | `reset_entry()` | [ ] |

## Porting Template

```c
/**
 * original_function_name (CODE:XXXX)
 *
 * Original disassembly:
 *   CODE:xxxx: MOV A, R7
 *   CODE:xxxx: ...
 *
 * Parameters: (document register bank usage)
 *   Bank 1 R0 = ...
 *
 * Returns:
 *   A = ...
 */
void original_function_name_xxxx(void) {
    // Port assembly here, preserving structure
}
```

## Testing Strategy

1. **Unit tests**: Test each ported function against expected behavior
2. **Memory dumps**: Compare INTMEM/EXTMEM state after init with MAME trace
3. **MIDI trace**: Send known MIDI sequences, compare SAM D-RAM writes
4. **Integration**: Run in MAME with logging, compare against original ROM

## Discovered Addresses

Track newly identified INTMEM/EXTMEM addresses found during porting:

| Address | Name | Found in | Description |
|---------|------|----------|-------------|
| | | | |

## Task List

### Setup
- [x] Create `sam_firmware/` directory structure
- [x] Create `sam_types.h` with struct definitions
- [x] Create `sam_hw.h` with SAM8905 interface
- [x] Create `sam_rom.h` with ROM access macros
- [x] Create `sam_config.h` for device-specific constants
- [x] Create `sam_firmware.h` main header
- [x] Create build system (Makefile for SDCC + host testing)
- [x] Create `test_main.c` for host-based unit tests

### Phase 1: Utilities
- [ ] Port `dptr_add_r6r7` (DCA8)
- [ ] Port `add_a_to_dptr` (DCB3)
- [ ] Port `table_search_match` (DC1C)
- [ ] Port `load_dptr_from_xram` (DC9C)
- [ ] Test utilities

### Phase 2: SAM Interface
- [x] Port `sam_write_dram` (A4BC) - `sam_dram_write()` in sam_hw.h
- [x] Port `sam_write_aram` (AD43) - `sam_aram_write_slot()` in sam_hw.c
- [x] Port `sam_dram_clear_all` (A53C) - `dram_clear_all()` in sam_hw.c
- [x] Port `sam_dram_write_word15` (A523) - `sam_init_slots()` in sam_hw.c
- [ ] Test SAM writes against MAME trace

### Phase 3: Memory Init
- [x] Port `extmem_clear_all` (D454) - `sam_extmem_clear_all()` in sam_init.c
- [x] Port `voice_pages_clear` (B70B) - `sam_voice_pages_clear()` in sam_init.c
- [x] Port `slot_manager_init` (9904) - `sam_slot_manager_init()` in sam_init.c
- [ ] Port `pitch_table_init` (9B16) - placeholder only, needs exact algorithm
- [ ] Verify pitch table against ROM/MAME

### Phase 4: MIDI (basic)
- [ ] Port `isr_uart_handler` (B630)
- [ ] Port `serial_handler` (C635)
- [ ] Test MIDI RX buffering

### Phase 5-8: Remaining
- [ ] (expand as Phase 1-4 complete)

## Notes

- The 8051 PSW register bank select (RS0/RS1) affects R0-R7 location. Track current
  bank when porting functions that switch banks.
- MOVX @DPTR vs MOVC A,@A+DPTR: external RAM vs code ROM. Track which access mode
  is used.
- The firmware uses both big-endian (MIDI, some pointers) and little-endian (most
  internal pointers) byte order. Document each.
