/**
 * test_program.h - Simple Test Sound Program for SAM8905 Emulator
 *
 * A minimal sound program with fixed values (no envelopes) for testing.
 * Uses a simple FM algorithm from MS4 program 0 (dpiano27).
 *
 * D-RAM word layout for this algorithm:
 *   Word 0-3: operator parameters
 *   Word 4: waveform select (internal sine = 0x20000)
 *   Word 10: waveform select
 *   Word 15: control (algorithm select + mix)
 */

#ifndef TEST_PROGRAM_H
#define TEST_PROGRAM_H

#include <stdint.h>

/*
 * A-RAM algorithm from MS4 address 0x027A (used by dpiano27 and many others)
 * 32 x 15-bit instruction words
 */
static const uint16_t TEST_ALGORITHM[32] = {
    0x08EF, 0x7EFB, 0x50BD, 0x28F7, 0x78FD, 0x4CCF, 0x59F7, 0x11EF,
    0x20FD, 0x086F, 0x3ADF, 0x113F, 0x42DF, 0x18BF, 0x7CF7, 0x43DF,
    0x30BF, 0x2876, 0x41EF, 0x20FD, 0x38EE, 0x2BDF, 0x087F, 0x00BF,
    0x0ADF, 0x48F7, 0x113F, 0x12DF, 0x18BF, 0x13DE, 0x7FFF, 0x7FFF,
};

/*
 * D-RAM stream for simple fixed-amplitude sound
 *
 * Format breakdown:
 *   [0x08] Handler 0x08 (pitch setup), 10 bytes:
 *     [0x08, vel_sens, note_off, ctrl, bend, fine_lo, fine_hi, skip, porta_r, porta_d]
 *     - vel_sens=0x00: no velocity sensitivity on pitch
 *     - note_off=0x00: no note offset (use MIDI note directly)
 *     - ctrl=0x00: no special pitch control
 *     - bend=0x02: pitch bend range (2 semitones)
 *     - fine_tune=0x0000: no fine tuning
 *
 *   [0x10] Handler 0x10 (amplitude setup), 9 bytes:
 *     [0x10, level, amp, env_ctrl, atk, unused, sustain, vel_sens, mod]
 *     - level=0x3F: full base level (63)
 *     - amp=0x7F: full amplitude (127)
 *     - env_ctrl=0x00: no envelope (instant attack, infinite sustain)
 *     - atk=0x00: instant attack
 *     - sustain=0x7F: full sustain
 *     - vel_sens=0x00: no velocity sensitivity on amplitude
 *
 *   [0x20] Handler 0x20 (output routing, TERMINATES), 4 bytes:
 *     [0x20, route1, route2, route3]
 *     - route3 bit 5 (0x20) = voice active flag
 */
static const uint8_t TEST_DRAM_STREAM[] = {
    /* Handler 0x08: Pitch (10 bytes) */
    0x08,       /* dispatch byte: handler 0x08 */
    0x00,       /* velocity sensitivity = 0 */
    0x00,       /* note offset = 0 */
    0x00,       /* control flags = 0 */
    0x02,       /* bend range = 2 semitones */
    0x00, 0x00, /* fine tune = 0 */
    0x00,       /* skip */
    0x00,       /* portamento rate = 0 */
    0x00,       /* portamento depth = 0 */

    /* Handler 0x10: Amplitude (9 bytes) */
    0x10,       /* dispatch byte: handler 0x10 */
    0x3F,       /* base level = 63 (max) */
    0x7F,       /* amplitude = 127 (max) */
    0x00,       /* envelope control = 0 (no envelope, instant on) */
    0x00,       /* attack rate = 0 (instant) */
    0x00,       /* unused */
    0x7F,       /* sustain level = 127 (max) */
    0x00,       /* velocity sensitivity = 0 */
    0x00,       /* modulation amount = 0 */

    /* Handler 0x20: Output routing (4 bytes, TERMINATES) */
    0x20,       /* dispatch byte: handler 0x20 */
    0x00,       /* route byte 1 */
    0x00,       /* route byte 2 */
    0x20,       /* route byte 3: bit 5 = voice active */
};

#define TEST_DRAM_STREAM_SIZE sizeof(TEST_DRAM_STREAM)

/*
 * Program header structure (matches MS4 format)
 * Offsets:
 *   0-7:   Name (8 bytes, space-padded)
 *   8:     Null terminator
 *   9:     Flags (bit7=complex, bits3:0=slot_count)
 *   10-11: A-RAM pointer (LE) - not used in test
 *   12-14: D-RAM entry0 (not used)
 *   15-16: Voice init pointer (LE) - not used
 *   17+:   D-RAM stream
 */
static const uint8_t TEST_PROGRAM_HEADER[] = {
    /* Name */
    'T', 'e', 's', 't', 'S', 'n', 'd', ' ',
    0x00,       /* null terminator */
    0x01,       /* flags: 1 slot, simple init */
    0x00, 0x00, /* A-RAM pointer (unused) */
    0x00, 0x00, 0x00, /* D-RAM entry0 */
    0x00, 0x00, /* voice init pointer */
    /* D-RAM stream follows at offset 17 */
};

#define TEST_PROGRAM_HEADER_SIZE sizeof(TEST_PROGRAM_HEADER)
#define TEST_PROGRAM_DRAM_STREAM_OFFSET 17

/*
 * Waveform words - which D-RAM words need internal sine (0x20000)
 * From algorithm analysis: words 4, 10, 15 use WWF
 */
static const uint8_t TEST_WAVEFORM_WORDS[] = { 4, 10, 15 };
#define TEST_NUM_WAVEFORM_WORDS 3

/*
 * Internal sine waveform value
 * WF register format: bit 8 set = internal, bits 0-7 = type
 * 0x100 = internal sine, stored left-shifted by 9 for D-RAM format
 */
#define DRAM_INTERNAL_SINE  (0x100 << 9)  /* = 0x20000 */

/*
 * Initialize a slot with fixed test values
 *
 * This bypasses the firmware's D-RAM config dispatch and directly
 * sets the D-RAM values for a simple test sound.
 *
 * @param slot      Slot number (0-15)
 * @param note      MIDI note number (0-127)
 * @param velocity  MIDI velocity (0-127)
 */
static inline void test_program_init_slot(int slot, uint8_t note, uint8_t velocity)
{
    (void)velocity;  /* Not used in fixed-amplitude version */

    /* Calculate pitch increment from note
     * At 44.1kHz, freq = 440 * 2^((note-69)/12)
     * phase_inc = freq * 4096 / 44100
     *
     * For simplicity, use a lookup or approximation.
     * Here we just use a simple formula that gives reasonable results.
     */

    /* Approximate pitch calculation (simplified) */
    /* Middle C (note 60) ≈ 261.6 Hz → phase_inc ≈ 24.3 → shifted ≈ 0xC00 */
    /* Each octave doubles the frequency */
    uint32_t base_pitch = 0x0C00;  /* ~Middle C */
    int octave_diff = (note - 60) / 12;
    int semitone = (note - 60) % 12;
    if (semitone < 0) {
        semitone += 12;
        octave_diff--;
    }

    /* Rough semitone multiplier table (Q12 format) */
    static const uint16_t semitone_mult[12] = {
        4096, 4340, 4598, 4871, 5161, 5468,   /* C, C#, D, D#, E, F */
        5793, 6137, 6502, 6889, 7298, 7732    /* F#, G, G#, A, A#, B */
    };

    uint32_t pitch = (base_pitch * semitone_mult[semitone]) >> 12;

    /* Apply octave shift */
    if (octave_diff > 0) {
        pitch <<= octave_diff;
    } else if (octave_diff < 0) {
        pitch >>= (-octave_diff);
    }

    /* Shift left 7 bits for D-RAM format */
    pitch = (pitch << 7) & 0x7FFFF;

    /* Fixed amplitude: max level with full mix */
    /* Format: amp12 in bits 18:7, mix_l in bits 6:4, mix_r in bits 2:0 */
    uint32_t amplitude = (0x7F << 7) | 0x3F;  /* Max amp, full mix */

    /* Slot base address */
    uint8_t base = slot << 4;

    /* Write D-RAM values via emulator API */
    extern void sam8905_write_dram(uint8_t addr, uint32_t param);

    /* Word 0: Phase accumulator (start at 0) */
    sam8905_write_dram(base | 0, 0);

    /* Word 1: Pitch increment */
    sam8905_write_dram(base | 1, pitch);

    /* Words 2-3: Additional parameters (zero) */
    sam8905_write_dram(base | 2, 0);
    sam8905_write_dram(base | 3, 0);

    /* Word 4: Waveform (internal sine) */
    sam8905_write_dram(base | 4, DRAM_INTERNAL_SINE);

    /* Words 5-9: More parameters (zero) */
    for (int i = 5; i < 10; i++) {
        sam8905_write_dram(base | i, 0);
    }

    /* Word 10: Waveform (internal sine) */
    sam8905_write_dram(base | 10, DRAM_INTERNAL_SINE);

    /* Words 11-13: Zero */
    for (int i = 11; i < 14; i++) {
        sam8905_write_dram(base | i, 0);
    }

    /* Word 14: Amplitude */
    sam8905_write_dram(base | 14, amplitude);

    /* Word 15: Control - algorithm 0, active (not idle) */
    /* Format: bit 17 = idle, bits 10:8 = algorithm */
    sam8905_write_dram(base | 15, 0);  /* Algorithm 0, active */
}

/*
 * Deactivate a slot (set to idle)
 */
static inline void test_program_stop_slot(int slot)
{
    extern void sam8905_write_dram(uint8_t addr, uint32_t param);

    /* Set word 15 with idle bit */
    sam8905_write_dram((slot << 4) | 15, 0x20000);  /* Idle bit = bit 17 */
}

#endif /* TEST_PROGRAM_H */
