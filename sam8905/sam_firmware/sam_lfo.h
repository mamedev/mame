#pragma once

/**
 * SAM8905 Controller Firmware - LFO Module
 *
 * Low Frequency Oscillator for modulation effects.
 * The firmware has a global mod LFO plus per-voice LFO/envelope blocks.
 *
 * Global LFO state is in EXTMEM:
 *   0x1180: mod_lfo_rate (0=disabled, higher=faster)
 *   0x1181: phase accumulator low byte
 *   0x1182: phase accumulator high byte
 *   0x1183: sine output (table lookup result)
 *
 * The mod wheel (CC1) sets the rate, and the output is used to modulate
 * pitch based on per-channel sensitivity (EXTMEM 0x1184-0x1193).
 */

#include <stdint.h>
#include "sam_types.h"

/* External memory contexts (defined in sam_firmware.h, declared here for inlines) */
extern XDATA extmem_t g_extmem;
extern IDATA intmem_t g_intmem;

/*============================================================================
 * Sine Table (CODE:9833)
 *
 * 64-entry signed full-wave sine table.
 * Values range from 0x00 (0) through 0x7F (+127) and back, then through
 * negative values (0x81 = -127) and back to 0.
 *
 * The firmware uses a 6-bit index: (phase_hi >> 1) & 0x3F
 *============================================================================*/

extern CODE uint8_t g_sine_table[64];

/*============================================================================
 * LFO Functions
 *============================================================================*/

/**
 * Update global modulation LFO (CODE:A314)
 *
 * Advances the phase accumulator by (rate * 32) and computes sine output.
 * Called once per timer tick when active voices exist.
 *
 * Phase increment is rate << 5 (multiply by 32).
 * At rate=255, full cycle takes ~8 ticks. At rate=1, ~2048 ticks.
 *
 * Assumes g_extmem is the active EXTMEM context.
 */
void global_mod_lfo_update(void);

/**
 * Get current LFO output value
 *
 * @return  Signed 8-bit sine value from last update
 */
static inline int8_t lfo_get_output(void)
{
    return (int8_t)g_extmem.mod_lfo_output;
}

/**
 * Set LFO rate (typically from mod wheel CC1)
 *
 * @param rate  0=off, 1-255=speed (higher=faster)
 */
static inline void lfo_set_rate(uint8_t rate)
{
    g_extmem.mod_lfo_rate = rate;
}

/**
 * Reset LFO phase to zero
 */
static inline void lfo_reset_phase(void)
{
    g_extmem.mod_lfo_phase_lo = 0;
    g_extmem.mod_lfo_phase_hi = 0;
    g_extmem.mod_lfo_output = 0;
}

/*============================================================================
 * Noise LFO (LFSR at INTMEM 0x51)
 *
 * Simple pseudo-random generator: x = x * 3 + 0x43
 * Used for noise waveform in per-voice LFO blocks (waveform type 4).
 *============================================================================*/

/**
 * Advance noise LFSR and return new value
 *
 * Uses g_intmem.lfsr_state (INTMEM 0x51).
 *
 * @return  New pseudo-random byte
 */
static inline uint8_t noise_lfsr_next(void)
{
    g_intmem.lfsr_state = g_intmem.lfsr_state * 3 + 0x43;
    return g_intmem.lfsr_state;
}

/**
 * Seed noise LFSR
 *
 * @param seed  Initial value (typically set during init)
 */
static inline void noise_lfsr_seed(uint8_t seed)
{
    g_intmem.lfsr_state = seed;
}
