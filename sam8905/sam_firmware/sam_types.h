#pragma once

/**
 * SAM8905 Controller Firmware - Type Definitions
 *
 * Memory structures matching the DREAM S.A. reference firmware.
 * Based on analysis of MS4 firmware (ms4_05_r1_0.bin).
 *
 * See: WIP_ms4_memory_init.md for detailed memory map documentation.
 */

#include <stdint.h>

/* Platform-specific memory qualifiers */
#ifdef __SDCC
  #define XDATA __xdata
  #define IDATA __idata
  #define CODE  __code
#else
  #define XDATA
  #define IDATA
  #define CODE const
#endif

/*============================================================================
 * INTMEM - Internal RAM (0x00-0x7F direct, 0x80-0xFF indirect)
 *============================================================================*/

/**
 * intmem_t - Direct INTMEM region (0x00-0x7F)
 *
 * On 8051, this is directly addressable internal RAM.
 * Register banks 0-3 at 0x00-0x1F, bit-addressable at 0x20-0x2F.
 */
typedef struct {
    /*--- Register Banks (0x00-0x1F) ---*/
    union {
        uint8_t raw[0x20];
        struct {
            uint8_t bank0[8];   /* 0x00-0x07: R0-R7 working registers */
            uint8_t bank1[8];   /* 0x08-0x0F: MIDI handler params */
            uint8_t bank2[8];   /* 0x10-0x17: MIDI parser / timer ticks */
            uint8_t bank3[8];   /* 0x18-0x1F: unused */
        };
    } regs;

    /*--- Bit-Addressable Area (0x20-0x2F) ---*/
    uint8_t flags_20;          /* 0x20: computation/voice flags */
    uint8_t flags_21;          /* 0x21: MIDI/system flags */
    uint8_t flags_22;          /* 0x22: voice/timer flags */
    uint8_t flags_23;          /* 0x23: (TBD) */
    uint8_t flags_24;          /* 0x24: (TBD) */
    uint8_t flags_25;          /* 0x25: (TBD) */
    uint8_t flags_26;          /* 0x26: (TBD) */
    uint8_t flags_27;          /* 0x27: (TBD) */
    uint8_t flags_28;          /* 0x28: (TBD) */
    uint8_t flags_29;          /* 0x29: (TBD) */
    uint8_t flags_2a;          /* 0x2A: (TBD) */
    uint8_t flags_2b;          /* 0x2B: (TBD) */
    uint8_t flags_2c;          /* 0x2C: (TBD) */
    uint8_t flags_2d;          /* 0x2D: (TBD) */
    uint8_t flags_2e;          /* 0x2E: (TBD) */

    /*--- Copy/Utility Variables (0x2F-0x33) ---*/
    uint8_t copy_count;        /* 0x2F: byte count for copy */
    uint8_t copy_src_lo;       /* 0x30: copy source low */
    uint8_t copy_src_hi;       /* 0x31: copy source high */
    uint8_t copy_dst_lo;       /* 0x32: copy dest low */
    uint8_t copy_dst_hi;       /* 0x33: copy dest high */

    /*--- Voice System Variables (0x34-0x56) ---*/
    uint8_t current_slot_id;   /* 0x34: current D-RAM slot */
    uint8_t program_base_dph;  /* 0x35: program ptr high */
    uint8_t program_base_dpl;  /* 0x36: program ptr low */
    uint8_t sam_ctrl_flags;    /* 0x37: SAM control value */
    uint8_t dram_address_counter; /* 0x38: D-RAM addr during init */
    uint8_t remaining_slots;   /* 0x39: D-RAM slots to process */
    uint8_t voice_page_num;    /* 0x3A: current voice page (P2) */
    uint8_t voice_slot_base;   /* 0x3B: XRAM voice slot R1 base */
    uint8_t midi_note;         /* 0x3C: current MIDI note */
    uint8_t midi_velocity;     /* 0x3D: current velocity */
    uint8_t amplitude_scale;   /* 0x3E: amplitude scaling */
    uint8_t velocity_mix_atten; /* 0x3F: velocity MIX attenuation */
    uint8_t pitch_bend_value;  /* 0x40: pitch bend (signed) */
    uint8_t rom_data_ptr_lo;   /* 0x41: ROM parse ptr low */
    uint8_t rom_data_ptr_hi;   /* 0x42: ROM parse ptr high */
    uint8_t dram_entry_lo;     /* 0x43: D-RAM entry low */
    uint8_t dram_entry_hi;     /* 0x44: D-RAM entry high */
    uint8_t voice_data_ptr_lo; /* 0x45: voice init data ptr low */
    uint8_t voice_data_ptr_hi; /* 0x46: voice init data ptr high */
    uint8_t voice_state_ptr_lo; /* 0x47: voice state XRAM ptr low */
    uint8_t voice_state_ptr_hi; /* 0x48: voice state XRAM ptr high */
    uint8_t voice_state_byte;  /* 0x49: voice state flags */
    uint8_t slot_count;        /* 0x4A: D-RAM slots for voice */
    uint8_t velocity_curve_ptr; /* 0x4B: velocity curve table */
    uint8_t portamento_value;  /* 0x4C: portamento rate (CC5) */
    uint8_t _pad_4d;           /* 0x4D: (TBD) */
    uint8_t voice_list_prev;   /* 0x4E: prev voice in list */
    uint8_t dram_slot_index;   /* 0x4F: current D-RAM slot idx */
    uint8_t _pad_50;           /* 0x50: (TBD) */
    uint8_t lfsr_state;        /* 0x51: noise LFO LFSR */
    uint8_t octave_shift;      /* 0x52: octave transposition */
    uint8_t dram_slot_free_list; /* 0x53: free list head */
    uint8_t active_voice_list_head; /* 0x54: active voice list */
    uint8_t pending_voice_list; /* 0x55: pending release list */
    uint8_t dram_slot_count;   /* 0x56: total D-RAM slots */

    /*--- Per-Channel Tables (0x57-0x77) ---*/
    uint8_t channel_split_program[4]; /* 0x57-0x5A */
    uint8_t _pad_5b_66[12];    /* 0x5B-0x66: (TBD) */
    uint8_t channel_current_prog[4]; /* 0x67-0x6A */
    uint8_t _pad_6b_76[12];    /* 0x6B-0x76: (TBD) */
    uint8_t scratch_77;        /* 0x77: temp storage */

    /*--- Table Search Parameters (0x78-0x7D) ---*/
    uint8_t search_ptr_hi;     /* 0x78: search ptr DPH */
    uint8_t search_ptr_lo;     /* 0x79: search ptr DPL */
    uint8_t search_value;      /* 0x7A: value to match */
    uint8_t search_count_hi;   /* 0x7B: search len high */
    uint8_t search_count_lo;   /* 0x7C: search len low */
    uint8_t _pad_7d;           /* 0x7D: (TBD) */

    /*--- D-RAM Free List start (0x7E-0x7F) ---*/
    uint8_t dram_free_list_0;  /* 0x7E: slot 0 next ptr */
    uint8_t dram_free_list_1;  /* 0x7F: slot 1 next ptr */

} intmem_t;

/**
 * intmem_upper_t - Indirect INTMEM region (0x80-0xB2)
 *
 * On 8051, addresses 0x80-0xFF require indirect addressing via @R0/@R1.
 * This region continues the D-RAM free list and channel tables.
 */
typedef struct {
    uint8_t dram_free_list[14];   /* 0x80-0x8D: slots 2-15 next ptrs */
    uint8_t channel_algorithm[16]; /* 0x8E-0x9D: channel→algo map */
    uint8_t channel_highest_note[4]; /* 0x9E-0xA1: highest note/ch (note: doc says 0x9F-0xA2) */
    uint8_t _pad_a2_ae[13];       /* 0xA2-0xAE: (TBD) */
    uint8_t channel_note_count[4]; /* 0xAF-0xB2: note count/ch */
} intmem_upper_t;

/*============================================================================
 * EXTMEM - External RAM (0x0000-0x1FFF, 8KB typical)
 *============================================================================*/

/**
 * Voice page layout (256 bytes per page)
 *
 * Pages 0x00-0x0F at EXTMEM 0x0000-0x0FFF
 * Extended pages at EXTMEM 0x14D5-0x1CD4
 */
typedef struct {
    uint8_t env_lfo_state[0x70]; /* 0x00-0x6F: 7 blocks × 16 bytes */
    uint8_t dram_slot_flags[16]; /* 0x70-0x7F: D-RAM init flags */
    uint8_t modulation_state[0x78]; /* 0x80-0xF7: modulation data */
    uint8_t output_route_base;   /* 0xF8: output routing base */
    uint8_t output_route_1;      /* 0xF9: routing byte 1 */
    uint8_t output_route_2;      /* 0xFA: routing byte 2 */
    uint8_t output_route_3;      /* 0xFB: routing byte 3 / flags */
    uint8_t slot_id;             /* 0xFC: slot ID (nibble-swapped) */
    uint8_t _pad_fd;             /* 0xFD */
    uint8_t voice_next;          /* 0xFE: next voice in list */
    uint8_t _pad_ff;             /* 0xFF */
} voice_page_t;

/**
 * extmem_t - Full external RAM layout
 *
 * 8KB for MS4/XE9, 32KB for Keyfox 10
 */
typedef struct {
    /*--- Voice Pages (0x0000-0x0FFF) ---*/
    voice_page_t voice_page[16];

    /*--- Pitch Tables (0x1000-0x117F) ---*/
    uint8_t pitch_table_lo[128];  /* 0x1000-0x107F */
    uint8_t pitch_table_mid[128]; /* 0x1080-0x10FF */
    uint8_t pitch_table_hi[128];  /* 0x1100-0x117F */

    /*--- Modulation State (0x1180-0x11B3) ---*/
    uint8_t mod_lfo_rate;         /* 0x1180 */
    uint8_t mod_lfo_phase_lo;     /* 0x1181 */
    uint8_t mod_lfo_phase_hi;     /* 0x1182 */
    uint8_t mod_lfo_output;       /* 0x1183 */
    uint8_t mod_wheel_sens[16];   /* 0x1184-0x1193 */
    uint8_t pitch_bend[32];       /* 0x1194-0x11B3: 16 × 2 bytes */

    /*--- Voice Control (0x11B4-0x11FF) ---*/
    uint8_t _region_11b4[33];     /* 0x11B4-0x11D4 */
    uint8_t voice_ctrl_1[16];     /* 0x11D5-0x11E4 */
    uint8_t _pad_11e5;            /* 0x11E5 */
    uint8_t program_init_copy[8]; /* 0x11E6-0x11ED */
    uint8_t algorithm_pool[8];    /* 0x11EE-0x11F5 */
    uint8_t _region_11f6[10];     /* 0x11F6-0x11FF */

    /*--- Channel Data (0x1200-0x12BF) ---*/
    uint8_t channel_data[192];    /* 0x1200-0x12BF */

    /*--- MIDI Subsystem (0x12C0-0x14D4) ---*/
    uint8_t midi_base_channel;    /* 0x12C0 */
    uint8_t _midi_12c1[2];        /* 0x12C1-0x12C2 */
    uint8_t sysex_state;          /* 0x12C3 */
    uint8_t _midi_12c4[8];        /* 0x12C4-0x12CB */
    uint8_t midi_running_status;  /* 0x12CC */
    uint8_t midi_rx_read_pos;     /* 0x12CD */
    uint8_t midi_rx_count;        /* 0x12CE */
    uint8_t midi_rx_write_pos;    /* 0x12CF */
    uint8_t _midi_12d0;           /* 0x12D0 */
    uint8_t midi_tx_count;        /* 0x12D1 */
    uint8_t midi_tx_write_pos;    /* 0x12D2 */
    uint8_t midi_tx_read_pos;     /* 0x12D3 */
    uint8_t midi_current_byte;    /* 0x12D4 */
    uint8_t midi_tx_buffer[255];  /* 0x12D5-0x13D3 */
    uint8_t midi_rx_buffer[255];  /* 0x13D4-0x14D2 */
    uint8_t current_program_base[2]; /* 0x14D3-0x14D4 (LE ptr) */

    /*--- Extended Voice Pages (0x14D5-0x1CD4) ---*/
    uint8_t extended_voice_page[8][256];

    /*--- Voice Control 2 (0x1CD5-0x1CE4) ---*/
    uint8_t voice_ctrl_2[16];

    /*--- Uncharacterized (0x1CE5-0x1FFF) ---*/
    uint8_t _region_1ce5[795];

} extmem_t;

/*============================================================================
 * Bit Flag Definitions
 *============================================================================*/

/* flags_20 (0x20) - Computation/Voice flags */
#define FLAG20_ROM_ACCESS_MODE    0x02  /* bit 1: MOVC vs MOVX */
#define FLAG20_SIGN               0x04  /* bit 2: negative value */
#define FLAG20_LFO_BYPASS         0x08  /* bit 3: LFO bypass */
#define FLAG20_VOICE_RELEASE      0x10  /* bit 4: voice releasing */
#define FLAG20_VOICE_ALLOC_ACTIVE 0x40  /* bit 6: allocation active */
#define FLAG20_AMPLITUDE_MODE     0x80  /* bit 7: amplitude mode */

/* flags_21 (0x21) - MIDI/System flags */
#define FLAG21_ENV_UPDATE         0x01  /* bit 0: env needs D-RAM update */
#define FLAG21_LAYER_SELECT       0x02  /* bit 1: dual-voice mode */
#define FLAG21_ALGO_POOL_FULL     0x04  /* bit 2: algo pool exhausted */
#define FLAG21_TX_IDLE            0x08  /* bit 3: TX buffer empty */
#define FLAG21_OMNI_MODE          0x10  /* bit 4: receive all channels */
#define FLAG21_TX_OVERFLOW        0x20  /* bit 5: TX overflow */
#define FLAG21_RX_OVERFLOW        0x40  /* bit 6: RX overflow */
#define FLAG21_MIDI_DATA2_PENDING 0x80  /* bit 7: waiting 2nd byte */

/* flags_22 (0x22) - Voice/Timer flags */
#define FLAG22_T1_SAVE            0x01  /* bit 0: saved T1 state */
#define FLAG22_VOICE_ENABLE_CHG   0x10  /* bit 4: voice enable changed */
#define FLAG22_NOTE_TRIGGER       0x20  /* bit 5: note trigger pending */
#define FLAG22_MULTI_VOICE        0x40  /* bit 6: multi-voice mode */

/*============================================================================
 * Register Bank Access Macros
 *============================================================================*/

/* Bank 2 tick counters */
#define TICK_SLOW(im)  ((im)->regs.raw[0x16])  /* Bank2 R6: slow tick */
#define TICK_FAST(im)  ((im)->regs.raw[0x17])  /* Bank2 R7: fast tick */

/* Linked list end marker */
#define VOICE_LIST_END  0xFF
