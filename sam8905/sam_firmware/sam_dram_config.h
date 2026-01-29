#pragma once

/**
 * SAM8905 Controller Firmware - D-RAM Configuration Handlers
 *
 * These handlers decode the voice init data stream and configure
 * SAM D-RAM words for voice initialization.
 *
 * Based on MS4 firmware analysis (CODE:AB4C-B2E5).
 *
 * Stream format:
 *   Each D-RAM word is configured by a dispatch byte followed by
 *   handler-specific data. The dispatch byte encodes:
 *     - bits 7: terminator flag (if set, exit after this word)
 *     - bits 5:3: handler selection (0-7)
 *     - bits 2:0: handler-specific flags
 *
 * Handler table:
 *   0x00 (AD8F): Short D-RAM write (3-4 bytes)
 *   0x08 (ADBD): Pitch/frequency setup (10 bytes)
 *   0x10 (B030): Amplitude/level + envelope (9 bytes)
 *   0x18 (B222): D-RAM write + velocity mod (4 bytes)
 *   0x20 (B278): Output routing (4 bytes, TERMINATES)
 *   0x28 (B2D2): Write constant 0x28 (1 byte)
 *   0x30 (B2CF): Skip/re-dispatch (1 byte)
 *   0x38 (inline): Write constant 0x38 (1 byte)
 */

#include <stdint.h>
#include "sam_types.h"

/*============================================================================
 * Dispatch System
 *============================================================================*/

/**
 * D-RAM config dispatch loop (CODE:AB4C)
 *
 * Reads dispatch bytes from voice data stream and calls handlers.
 * Entry: rom_data_ptr_lo/hi = stream pointer
 *        remaining_slots = D-RAM word counter (16)
 *        voice_slot_base = voice page offset
 *
 * Exits when: remaining_slots reaches 0, or terminator (bit 7 set)
 */
void dram_config_dispatch(void);

/**
 * Advance to next D-RAM word and dispatch (CODE:AB40)
 *
 * Called at end of each handler to continue to next word.
 * Updates voice_slot_base, dram_address_counter, remaining_slots.
 */
void dram_config_advance_and_dispatch(void);

/*============================================================================
 * Handler Functions
 *============================================================================*/

/**
 * Handler 0x00: Short D-RAM write (CODE:AD8F)
 *
 * Stream bytes: dispatch + value_lo + value_hi [+ extra]
 * 3 or 4 bytes depending on next byte's high bits.
 * Writes 24-bit value to D-RAM word.
 */
void dram_config_handler_00(void);

/**
 * Handler 0x08: Pitch/frequency setup (CODE:ADBD)
 *
 * Stream bytes (10 total):
 *   [0] dispatch
 *   [1] velocity_sensitivity
 *   [2] note_offset
 *   [3] control_flags
 *   [4] bend_range
 *   [5] fine_tune_lo
 *   [6] fine_tune_hi
 *   [7] skip
 *   [8] portamento_rate
 *   [9] portamento_depth
 *
 * Calculates pitch from MIDI note + offset, applies pitch bend and fine tune.
 */
void dram_config_handler_08(void);

/**
 * Handler 0x10: Amplitude/level + envelope (CODE:B030)
 *
 * Stream bytes (9 total):
 *   [0] dispatch
 *   [1] base_level
 *   [2] amplitude
 *   [3] envelope_control
 *   [4] attack_rate
 *   [5] unused
 *   [6] sustain_level
 *   [7] velocity_sensitivity
 *   [8] modulation_amount
 *
 * Sets up amplitude with velocity scaling and envelope enable.
 */
void dram_config_handler_10(void);

/**
 * Handler 0x18: D-RAM write + velocity mod (CODE:B222)
 *
 * Stream bytes (4 total):
 *   [0] dispatch
 *   [1] value_lo
 *   [2] value_hi
 *   [3] velocity_sensitivity
 *
 * Writes D-RAM value, optionally modulated by velocity.
 */
void dram_config_handler_18(void);

/**
 * Handler 0x20: Output routing (CODE:B278)
 *
 * Stream bytes (4 total):
 *   [0] dispatch
 *   [1] route_byte_1
 *   [2] route_byte_2
 *   [3] route_byte_3
 *
 * Sets voice output routing. TERMINATES dispatch loop.
 */
void dram_config_handler_20(void);

/**
 * Handler 0x28: Write constant 0x28 (CODE:B2D2)
 *
 * Stream bytes (1 total):
 *   [0] dispatch (0x28-0x2F)
 *
 * Writes constant 0x28 to voice page, continues dispatch.
 */
void dram_config_handler_28(void);

/**
 * Handler 0x30: Skip remaining words (CODE:B2CF)
 *
 * Stream bytes (1 total):
 *   [0] dispatch (0x30-0x37)
 *
 * Skips all remaining D-RAM words. TERMINATES when counter exhausted.
 */
void dram_config_handler_30(void);

/*============================================================================
 * Helper Functions
 *============================================================================*/

/**
 * Read byte from ROM data stream and advance pointer
 *
 * @return  Byte from rom_data_ptr_lo/hi, pointer incremented
 */
uint8_t dram_config_read_stream_byte(void);

/**
 * Read byte from ROM data stream without advancing (peek)
 *
 * @return  Byte at current rom_data_ptr_lo/hi
 */
uint8_t dram_config_peek_stream_byte(void);

/**
 * Write byte to voice page at current slot offset
 *
 * @param value  Byte to write at voice_page[voice_page_num][voice_slot_base]
 */
void dram_config_write_slot_byte(uint8_t value);

/**
 * Apply velocity scaling to a value (CODE:B1EC)
 *
 * @param value       Base value (0-255)
 * @param sensitivity Velocity sensitivity (0-127)
 * @return            Value scaled by velocity
 */
uint8_t dram_config_apply_velocity(uint8_t value, uint8_t sensitivity);

/*============================================================================
 * Runtime Modulation Functions (called from periodic_voice_update)
 *============================================================================*/

/**
 * Write pitch modulation to D-RAM (CODE:9FCD)
 *
 * Called during periodic voice update to apply LFO/pitch bend modulation
 * to a D-RAM pitch word. Computes: base_pitch + (modulation * base_pitch)
 *
 * The modulation state block (8 bytes at voice_slot_base) contains:
 *   [0]: dispatch byte (identifies D-RAM word type)
 *   [1]: base pitch low byte
 *   [2]: base pitch mid byte
 *   [3]: base pitch high byte (bits 2:0) + flags
 *   [4-5]: modulation parameters
 *   [6-7]: last written modulation (used to detect changes)
 *   [0xE]: last mod_lo (relative to voice_slot_base start)
 *   [0xF]: last mod_hi
 *
 * @param mod_lo   Modulation value low byte
 * @param mod_hi   Modulation value high byte (signed)
 * @param dispatch Dispatch byte from slot (for D-RAM address calculation)
 */
void modulation_write_dram(int8_t mod_lo, int8_t mod_hi, uint8_t dispatch);

/*============================================================================
 * Test Support
 *============================================================================*/

/**
 * Set test ROM buffer (for unit testing)
 *
 * When set to non-NULL, ROM access functions will read from this
 * buffer instead of g_rom. Pass NULL to revert to normal operation.
 *
 * @param rom  Pointer to test ROM buffer, or NULL
 */
void dram_config_set_test_rom(uint8_t *rom);
