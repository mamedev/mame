#pragma once

/**
 * SAM8905 Controller Firmware - Device Configuration
 *
 * Device-specific constants. Select ONE device or provide custom values.
 */

#include <stdint.h>

/*============================================================================
 * Device Selection (uncomment ONE)
 *============================================================================*/

/* #define SAM_DEVICE_MS4 */
/* #define SAM_DEVICE_KEYFOX10 */
/* #define SAM_DEVICE_XE9 */

/*============================================================================
 * Solton/Hohner MS4
 *============================================================================*/
#ifdef SAM_DEVICE_MS4

#define SAM_DEVICE_NAME       "MS4"
#define SAM_ROM_SIZE          0x10000   /* 64KB */
#define SAM_RAM_SIZE          0x2000    /* 8KB */
#define SAM_NUM_VOICE_PAGES   16
#define SAM_NUM_PROGRAMS      66
#define SAM_PROGRAM_TABLE     0x0040    /* CODE space, BE pointers */
#define SAM_PROGRAM_IN_CODE   1         /* Programs in CODE (ROM) space */
#define SAM_MIDI_HARDWARE     1         /* Hardware UART */
#define SAM_MIDI_CHANNELS     4         /* Responds to 4 consecutive channels */
#define SAM_DRAM_SLOTS        16        /* D-RAM voice slots */

/* Timer 1 reload for ~5.5ms period (16MHz / 12 / 7333) */
#define SAM_TIMER1_RELOAD_HI  0xE3
#define SAM_TIMER1_RELOAD_LO  0x5B

/* ROM tables */
#define SAM_SINE_TABLE        0x9833    /* 64-byte sine LFO table */
#define SAM_VELOCITY_CURVES   0x98E8    /* Velocity curve tables */

#endif /* SAM_DEVICE_MS4 */

/*============================================================================
 * Wersi Keyfox 10
 *============================================================================*/
#ifdef SAM_DEVICE_KEYFOX10

#define SAM_DEVICE_NAME       "Keyfox10"
#define SAM_ROM_SIZE          0x20000   /* 128KB (CODE+DATA) */
#define SAM_RAM_SIZE          0x8000    /* 32KB */
#define SAM_NUM_VOICE_PAGES   16        /* TBD - may be more with 32KB */
#define SAM_NUM_PROGRAMS      97
#define SAM_PROGRAM_TABLE     0x6285    /* DATA space, BE pointers */
#define SAM_PROGRAM_IN_CODE   0         /* Programs in DATA (RAM/ROM) space */
#define SAM_SOUND_TABLE       0xFFA0    /* Sound table (Keyfox-specific) */
#define SAM_MIDI_HARDWARE     0         /* Bit-banged MIDI */
#define SAM_MIDI_CHANNELS     4
#define SAM_DRAM_SLOTS        16

/* Timer 1 reload (TBD - verify from firmware) */
#define SAM_TIMER1_RELOAD_HI  0xE3
#define SAM_TIMER1_RELOAD_LO  0x5B

#endif /* SAM_DEVICE_KEYFOX10 */

/*============================================================================
 * Hohner XE9 / XE9L
 *============================================================================*/
#ifdef SAM_DEVICE_XE9

#define SAM_DEVICE_NAME       "XE9"
#define SAM_ROM_SIZE          0x10000   /* 64KB */
#define SAM_RAM_SIZE          0x2000    /* 8KB */
#define SAM_NUM_VOICE_PAGES   16
#define SAM_NUM_PROGRAMS      0         /* TBD */
#define SAM_PROGRAM_TABLE     0x0000    /* TBD */
#define SAM_PROGRAM_IN_CODE   1         /* TBD */
#define SAM_MIDI_HARDWARE     1         /* Hardware UART */
#define SAM_MIDI_CHANNELS     4
#define SAM_DRAM_SLOTS        16

#define SAM_TIMER1_RELOAD_HI  0xE3
#define SAM_TIMER1_RELOAD_LO  0x5B

#endif /* SAM_DEVICE_XE9 */

/*============================================================================
 * Validation
 *============================================================================*/
#if !defined(SAM_DEVICE_MS4) && !defined(SAM_DEVICE_KEYFOX10) && !defined(SAM_DEVICE_XE9)
#error "No SAM device selected. Define SAM_DEVICE_MS4, SAM_DEVICE_KEYFOX10, or SAM_DEVICE_XE9"
#endif

/*============================================================================
 * Common Constants
 *============================================================================*/

/* SAM8905 base address in memory map */
#define SAM_BASE_ADDR         0x8000

/* MIDI constants */
#define MIDI_BAUD_RATE        31250
#define MIDI_ACTIVE_SENSE     0xFE
#define MIDI_BUFFER_SIZE      255

/* Voice/slot limits */
#define MAX_VOICES            16
#define MAX_DRAM_SLOTS        16
#define DRAM_WORDS_PER_SLOT   16

/* Envelope/LFO blocks per voice page */
#define ENV_LFO_BLOCKS        7
#define ENV_LFO_BLOCK_SIZE    16
