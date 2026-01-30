/**
 * SAM8905 Controller Firmware - D-RAM Configuration Handlers
 *
 * Implementation of voice init data stream decoder.
 */

#include "sam_dram_config.h"
#include "sam_firmware.h"
#include <stddef.h>  /* For NULL */

/*============================================================================
 * Debug Output
 *============================================================================*/

#ifdef SAM_HW_PLATFORM
#include <stdio.h>
#define DEBUG_DRAM(fmt, ...) do { printf("DRAM: " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#else
#define DEBUG_DRAM(fmt, ...) do { } while(0)
#endif

/* Forward declarations (defined later in this file) */
void dram_param_processor_finish(void);

/*============================================================================
 * ROM Access Helpers
 *
 * The firmware can access ROM via MOVC (code space) or MOVX (data space).
 * FLAG20_ROM_ACCESS_MODE bit 1 selects the mode.
 * For host testing, we use a configurable ROM read function.
 *============================================================================*/

/* ROM data (extern from test harness or actual ROM) */
extern CODE uint8_t g_rom[];

/* Test hook: if non-NULL, use this instead of g_rom for testing */
static uint8_t *s_test_rom = NULL;

/**
 * Set test ROM buffer (for unit testing)
 * Pass NULL to revert to using g_rom.
 */
void dram_config_set_test_rom(uint8_t *rom)
{
    s_test_rom = rom;
}

/**
 * Read byte from ROM at address
 */
static inline uint8_t rom_read_byte(uint16_t addr)
{
    if (s_test_rom != NULL) {
        return s_test_rom[addr];
    }
    return g_rom[addr];
}

/*============================================================================
 * Stream Access Functions
 *============================================================================*/

uint8_t dram_config_read_stream_byte(void)
{
    uint16_t ptr = ((uint16_t)g_intmem.rom_data_ptr_hi << 8) | g_intmem.rom_data_ptr_lo;
    uint8_t byte = rom_read_byte(ptr);

    /* Increment pointer with carry */
    g_intmem.rom_data_ptr_lo++;
    if (g_intmem.rom_data_ptr_lo == 0) {
        g_intmem.rom_data_ptr_hi++;
    }

    return byte;
}

uint8_t dram_config_peek_stream_byte(void)
{
    uint16_t ptr = ((uint16_t)g_intmem.rom_data_ptr_hi << 8) | g_intmem.rom_data_ptr_lo;
    return rom_read_byte(ptr);
}

/*============================================================================
 * Voice Page Access
 *============================================================================*/

void dram_config_write_slot_byte(uint8_t value)
{
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base, value);
}

/*============================================================================
 * Slot Mapping Update (verified against original at CODE:AE49-AE4E)
 *
 * Original assembly:
 *   MOV R1,0x4f          ; R1 = dram_slot_index
 *   MOV A,0x38           ; A = dram_address_counter
 *   MOVX @R1,A           ; Write to XDATA at (P2:R1) = voice_page[dram_slot_index]
 *   INC 0x4f             ; dram_slot_index++
 *
 * Called from dram_config_handler_08 when _0_3 flag is not set.
 * dram_slot_index is initialized to 0x70 in voice_note_init_dram before
 * calling dram_param_processor.
 *
 * Updates the slot mapping at page[dram_slot_index] with the current
 * D-RAM address counter. This tells periodic_voice_update() which D-RAM
 * words need modulation processing.
 *============================================================================*/

void dram_config_update_slot_mapping(void)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t dram_addr = g_intmem.dram_address_counter;

    DEBUG_DRAM("slot_map[0x%02X] = 0x%02X", g_intmem.dram_slot_index, dram_addr);

    /* Write D-RAM address to slot mapping (MOVX @R1,A with P2=page) */
    voice_page_write(page, g_intmem.dram_slot_index, dram_addr);

    /* Increment slot index for next configured word */
    g_intmem.dram_slot_index++;
}

/*============================================================================
 * Velocity Scaling (CODE:B1EC)
 *
 * Applies velocity-based attenuation to a value.
 * sensitivity=0: no velocity effect (return value)
 * sensitivity=127: full velocity effect
 *============================================================================*/

uint8_t dram_config_apply_velocity(uint8_t value, uint8_t sensitivity)
{
    uint8_t velocity = g_intmem.midi_velocity;
    uint16_t scaled;

    if (sensitivity == 0) {
        return value;
    }

    /* Scale: result = value - ((127 - velocity) * sensitivity) / 128 */
    /* This reduces value when velocity is below 127 */
    scaled = (uint16_t)(127 - velocity) * sensitivity;
    scaled >>= 7;  /* Divide by 128 */

    if (scaled >= value) {
        return 0;
    }
    return value - (uint8_t)scaled;
}

/*============================================================================
 * Parameter Processor (CODE:AA9A)
 *
 * Main entry point for D-RAM parameter initialization.
 *
 * MS4 Program format (from parse_programs.py analysis):
 *   Offset 12-14: dram_entry0 (3 bytes, one-time)
 *   Offset 15-16: first voice_data_ptr (2 bytes)
 *   Offset 17+: Additional 2-byte slot pointers until 0x0000
 *   After 0x0000: D-RAM stream
 *
 * Process flow:
 *   1. Read initial 5 bytes (entry0 + first voice_ptr) at offset 12
 *   2. If voice_ptr != 0: process envelope at that pointer, advance slot_base
 *   3. Read 2-byte slot pointers until 0x0000
 *   4. When 0x0000 found: switch to D-RAM dispatch mode
 *
 * Called with rom_data_ptr pointing to program_base + 12
 *============================================================================*/

void dram_param_processor(void)
{
    uint8_t entry_byte0, entry_lo, entry_hi;
    uint8_t voice_ptr_lo, voice_ptr_hi;
    uint16_t voice_ptr;

    DEBUG_DRAM("param_processor: stream=0x%04X slot_base=%d",
               ((uint16_t)g_intmem.rom_data_ptr_hi << 8) | g_intmem.rom_data_ptr_lo,
               g_intmem.voice_slot_base);

    /* Read initial 5 bytes: 3-byte D-RAM entry + 2-byte voice_data_ptr */
    entry_byte0 = dram_config_read_stream_byte();
    entry_lo = dram_config_read_stream_byte();
    entry_hi = dram_config_read_stream_byte();
    voice_ptr_lo = dram_config_read_stream_byte();
    voice_ptr_hi = dram_config_read_stream_byte();

    voice_ptr = ((uint16_t)voice_ptr_hi << 8) | voice_ptr_lo;

    DEBUG_DRAM("  entry0: 0x%02X%02X%02X, first_voice_ptr=0x%04X",
               entry_byte0, entry_lo, entry_hi, voice_ptr);

    /* Store entry bytes in intmem */
    g_intmem.velocity_curve_ptr = entry_byte0;
    g_intmem.dram_entry_lo = entry_lo;
    g_intmem.dram_entry_hi = entry_hi;
    g_intmem.voice_data_ptr_lo = voice_ptr_lo;
    g_intmem.voice_data_ptr_hi = voice_ptr_hi;

    /* Process first envelope block if voice_ptr != 0 */
    if (voice_ptr != 0) {
        DEBUG_DRAM("  processing envelope at 0x%04X, slot_base=%d", voice_ptr, g_intmem.voice_slot_base);
        voice_init_copy_and_envelope();
        /* voice_init_copy_and_envelope advances slot_base and calls back to dram_param_processor_continue */
        return;
    }

    /* voice_ptr == 0: no envelope blocks, go straight to D-RAM dispatch */
    dram_param_processor_finish();
}

/*============================================================================
 * Parameter Processor Continue (called after envelope processing)
 *
 * Reads additional 2-byte slot pointers until 0x0000, then switches to
 * D-RAM dispatch mode.
 *============================================================================*/

void dram_param_processor_continue(void)
{
    uint8_t voice_ptr_lo, voice_ptr_hi;
    uint16_t voice_ptr;
    int iteration_limit = 20;

    DEBUG_DRAM("param_processor_continue: slot_base=%d", g_intmem.voice_slot_base);

    while (iteration_limit-- > 0) {
        /* Read 2-byte slot pointer */
        voice_ptr_lo = dram_config_read_stream_byte();
        voice_ptr_hi = dram_config_read_stream_byte();
        voice_ptr = ((uint16_t)voice_ptr_hi << 8) | voice_ptr_lo;

        DEBUG_DRAM("  slot_ptr=0x%04X", voice_ptr);

        if (voice_ptr == 0) {
            /* Terminator found, switch to D-RAM dispatch */
            dram_param_processor_finish();
            return;
        }

        /* Store and process envelope block */
        g_intmem.voice_data_ptr_lo = voice_ptr_lo;
        g_intmem.voice_data_ptr_hi = voice_ptr_hi;

        DEBUG_DRAM("  processing envelope at 0x%04X, slot_base=%d", voice_ptr, g_intmem.voice_slot_base);
        voice_init_copy_and_envelope();
        /* voice_init_copy_and_envelope will call back to continue */
        return;
    }
}

/*============================================================================
 * Parameter Processor Finish (switches to D-RAM dispatch mode)
 *============================================================================*/

void dram_param_processor_finish(void)
{
    DEBUG_DRAM("param_processor_finish: switching to D-RAM dispatch");

    /* Set voice_slot_base for D-RAM dispatch phase */
    /* Original: voice_slot_base = (dram_address_counter << 4) >> 1 | 0x80 */
    g_intmem.voice_slot_base = ((g_intmem.dram_address_counter << 4) >> 1) | 0x80;
    g_intmem.remaining_slots = 16;

    /* Start D-RAM dispatch */
    dram_config_dispatch();
}

/*============================================================================
 * Dispatch Loop (CODE:AB4C)
 *
 * Main dispatch loop for D-RAM configuration.
 * Processes 16 D-RAM words (remaining_slots countdown).
 *============================================================================*/

void dram_config_dispatch(void)
{
    uint8_t dispatch;
    int iteration_limit = 100;  /* Safety limit */

    DEBUG_DRAM("dispatch: stream=0x%04X remaining=%d slot_base=%d",
               ((uint16_t)g_intmem.rom_data_ptr_hi << 8) | g_intmem.rom_data_ptr_lo,
               g_intmem.remaining_slots, g_intmem.voice_slot_base);

    while (g_intmem.remaining_slots > 0 && iteration_limit-- > 0) {
        /* Read dispatch byte from stream */
        dispatch = dram_config_peek_stream_byte();

        DEBUG_DRAM("  byte=0x%02X handler=0x%02X", dispatch, dispatch & 0x38);

        /* Check terminator bit (bit 7) */
        if (dispatch & 0x80) {
            /* Terminator: write dispatch byte value to slot and exit */
            dram_config_write_slot_byte(dispatch);
            /* Advance pointer past dispatch byte */
            dram_config_read_stream_byte();
            /* Mark as complete */
            g_intmem.remaining_slots = 0;
            return;
        }

        /* Dispatch based on bits 5:3 */
        switch (dispatch & 0x38) {
        case 0x00:
            dram_config_handler_00();
            return;  /* Handler continues dispatch */
        case 0x08:
            dram_config_handler_08();
            return;
        case 0x10:
            dram_config_handler_10();
            return;
        case 0x18:
            dram_config_handler_18();
            return;
        case 0x20:
            dram_config_handler_20();
            return;  /* Terminates */
        case 0x28:
            dram_config_handler_28();
            return;
        case 0x30:
            dram_config_handler_30();
            return;  /* May terminate */
        case 0x38:
            /* Inline handler: write constant 0x38 */
            dram_config_write_slot_byte(0x38);
            /* Consume dispatch byte */
            dram_config_read_stream_byte();
            /* Advance to next word */
            dram_config_advance_and_dispatch();
            return;
        }
    }
}

/*============================================================================
 * Advance and Continue (CODE:AB40)
 *
 * Called at end of each handler to move to next D-RAM word.
 *============================================================================*/

void dram_config_advance_and_dispatch(void)
{
    /* Advance voice slot base by 8 (D-RAM word size in voice page) */
    g_intmem.voice_slot_base += 8;

    /* Increment D-RAM address counter */
    g_intmem.dram_address_counter++;

    /* Decrement remaining slots */
    g_intmem.remaining_slots--;

    /* Continue dispatch if more slots */
    if (g_intmem.remaining_slots > 0) {
        dram_config_dispatch();
    }
}

/*============================================================================
 * Handler 0x28: Write Constant (CODE:B2D2)
 *
 * Simplest handler - writes constant 0x28 to voice slot.
 *============================================================================*/

void dram_config_handler_28(void)
{
    /* Consume dispatch byte */
    dram_config_read_stream_byte();

    /* Write constant 0x28 to voice page slot */
    dram_config_write_slot_byte(0x28);

    /* Continue to next word */
    dram_config_advance_and_dispatch();
}

/*============================================================================
 * Handler 0x30: Skip Remaining (CODE:B2CF)
 *
 * Skips all remaining D-RAM words, terminates when counter exhausted.
 *============================================================================*/

void dram_config_handler_30(void)
{
    /* Consume dispatch byte */
    dram_config_read_stream_byte();

    /* Loop through remaining slots */
    while (g_intmem.remaining_slots > 0) {
        /* Advance counters */
        g_intmem.voice_slot_base += 8;
        g_intmem.dram_address_counter++;
        g_intmem.remaining_slots--;

        if (g_intmem.remaining_slots == 0) {
            break;
        }

        /* Check next dispatch byte - if it's a different handler, call it */
        uint8_t next = dram_config_peek_stream_byte();

        if (next & 0x80) {
            /* Terminator - write and exit */
            dram_config_write_slot_byte(next);
            dram_config_read_stream_byte();
            g_intmem.remaining_slots = 0;
            return;
        }

        /* Re-dispatch based on handler type */
        switch (next & 0x38) {
        case 0x00:
            dram_config_handler_00();
            return;
        case 0x08:
            dram_config_handler_08();
            return;
        case 0x10:
            dram_config_handler_10();
            return;
        case 0x18:
            dram_config_handler_18();
            return;
        case 0x20:
            dram_config_handler_20();
            return;
        case 0x28:
            dram_config_handler_28();
            return;
        case 0x30:
            /* Continue skipping */
            dram_config_read_stream_byte();
            break;
        case 0x38:
            /* Write constant 0x38, continue loop */
            dram_config_write_slot_byte(0x38);
            dram_config_read_stream_byte();
            break;
        }
    }
}

/*============================================================================
 * Handler 0x00: Short D-RAM Write (CODE:AD8F)
 *
 * Writes 3-byte D-RAM value to voice page.
 * Format varies based on byte[3] high bits.
 *============================================================================*/

void dram_config_handler_00(void)
{
    uint8_t dispatch;
    uint8_t value_lo;
    uint8_t value_hi;
    uint8_t byte3;

    /* Read dispatch byte (already peeked, now consume) */
    dispatch = dram_config_read_stream_byte();

    /* Read value bytes */
    value_lo = dram_config_read_stream_byte();
    value_hi = dram_config_read_stream_byte();

    /* Write to voice page slot (3 bytes at offset) */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base, dispatch);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 1, value_lo);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 2, value_hi);

    /* Check byte[3] to determine 3 vs 4 byte mode */
    byte3 = dram_config_peek_stream_byte();

    if ((byte3 & 0xE0) == 0) {
        /* 4-byte mode: consume extra byte */
        voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 3,
                        dram_config_read_stream_byte());
    }
    /* else: 3-byte mode, byte3 is next dispatch byte */

    /* Continue dispatch */
    dram_config_advance_and_dispatch();
}

/*============================================================================
 * Handler 0x18: D-RAM Write with Velocity (CODE:B222)
 *
 * 4-byte write with optional velocity modulation.
 *============================================================================*/

void dram_config_handler_18(void)
{
    uint8_t dispatch;
    uint8_t value_lo;
    uint8_t value_hi;
    uint8_t vel_sens;

    /* Read bytes */
    dispatch = dram_config_read_stream_byte();
    value_lo = dram_config_read_stream_byte();
    value_hi = dram_config_read_stream_byte();
    vel_sens = dram_config_read_stream_byte();

    /* Apply velocity scaling if sensitivity non-zero */
    if (vel_sens != 0) {
        value_hi = dram_config_apply_velocity(value_hi, vel_sens);
    }

    /* Write to voice page slot */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base, dispatch);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 1, value_lo);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 2, value_hi);

    /* Continue dispatch */
    dram_config_advance_and_dispatch();
}

/*============================================================================
 * Handler 0x20: Output Routing (CODE:B278)
 *
 * 4-byte output routing setup. TERMINATES dispatch loop.
 *============================================================================*/

void dram_config_handler_20(void)
{
    uint8_t dispatch;
    uint8_t route1;
    uint8_t route2;
    uint8_t route3;

    /* Read bytes */
    dispatch = dram_config_read_stream_byte();
    route1 = dram_config_read_stream_byte();
    route2 = dram_config_read_stream_byte();
    route3 = dram_config_read_stream_byte();

    /* Write to voice page output routing offsets (0xF9-0xFB) */
    voice_page_write(g_intmem.voice_page_num, VOICE_PAGE_ROUTE1, route1);
    voice_page_write(g_intmem.voice_page_num, VOICE_PAGE_ROUTE2, route2);
    voice_page_write(g_intmem.voice_page_num, VOICE_PAGE_ROUTE3, route3);

    /* Also write to voice slot base */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base, dispatch);

    /* Handler 0x20 TERMINATES the dispatch loop - don't call advance */
    /* Set remaining_slots to 0 to indicate completion */
    g_intmem.remaining_slots = 0;
}

/*============================================================================
 * Handler 0x08: Pitch Setup (CODE:ADBD)
 *
 * 10-byte pitch/frequency configuration.
 * This is the most complex handler - calculates pitch from note + offset,
 * applies pitch bend, fine tuning, and portamento.
 *
 * TODO: Full implementation requires pitch table lookup and modulation.
 * For now, implement basic structure.
 *============================================================================*/

void dram_config_handler_08(void)
{
    uint8_t dispatch;
    uint8_t vel_sens;
    uint8_t note_offset;
    uint8_t ctrl_flags;
    uint8_t bend_range;
    uint8_t fine_lo;
    uint8_t fine_hi;
    uint8_t skip;
    uint8_t port_rate;
    uint8_t port_depth;
    uint8_t note;

    /* Read all 10 bytes */
    dispatch = dram_config_read_stream_byte();
    vel_sens = dram_config_read_stream_byte();
    note_offset = dram_config_read_stream_byte();
    ctrl_flags = dram_config_read_stream_byte();
    bend_range = dram_config_read_stream_byte();
    fine_lo = dram_config_read_stream_byte();
    fine_hi = dram_config_read_stream_byte();
    skip = dram_config_read_stream_byte();
    port_rate = dram_config_read_stream_byte();
    port_depth = dram_config_read_stream_byte();

    /* Suppress unused variable warnings - these are used for modulation */
    (void)vel_sens;
    (void)skip;

    /* Calculate note with offset */
    note = g_intmem.midi_note + note_offset;
    if (note > 127) {
        note = 127;  /* Clamp to valid range */
    }

    /* Look up pitch from table */
    uint8_t pitch_lo = g_extmem.pitch_table_lo[note];
    uint8_t pitch_mid = g_extmem.pitch_table_mid[note];
    uint8_t pitch_hi = g_extmem.pitch_table_hi[note];

    /* Apply fine tune offset */
    uint16_t fine = ((uint16_t)fine_hi << 8) | fine_lo;
    uint32_t pitch = ((uint32_t)pitch_hi << 16) | ((uint32_t)pitch_mid << 8) | pitch_lo;
    pitch += fine;

    /* Store pitch in voice page slot */
    /* D-RAM pitch format: word contains 24-bit pitch value */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base, dispatch);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 1, (uint8_t)pitch);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 2, (uint8_t)(pitch >> 8));
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 3, (uint8_t)(pitch >> 16));

    /* Store portamento parameters (if enabled) */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 4, port_rate);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 5, port_depth);

    /* Store control flags for modulation processing */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 6, ctrl_flags);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 7, bend_range);

    /* Update slot mapping for periodic modulation (from CODE:AE49-AE4E) */
    /* Original: if (_0_3 != '\x01') { write dram_address to slot_map; dram_slot_index++; } */
    dram_config_update_slot_mapping();

    /* Continue dispatch */
    dram_config_advance_and_dispatch();
}

/*============================================================================
 * Handler 0x10: Amplitude/Envelope (CODE:B030)
 *
 * 9-byte amplitude and envelope configuration.
 * Sets up base level, velocity scaling, and envelope parameters.
 *
 * TODO: Full envelope setup requires envelope state initialization.
 * For now, implement basic amplitude calculation.
 *============================================================================*/

void dram_config_handler_10(void)
{
    uint8_t dispatch;
    uint8_t base_level;
    uint8_t amplitude;
    uint8_t env_ctrl;
    uint8_t attack_rate;
    uint8_t unused;
    uint8_t sustain;
    uint8_t vel_sens;
    uint8_t mod_amt;
    uint8_t final_level;

    /* Read all 9 bytes */
    dispatch = dram_config_read_stream_byte();
    base_level = dram_config_read_stream_byte();
    amplitude = dram_config_read_stream_byte();
    env_ctrl = dram_config_read_stream_byte();
    attack_rate = dram_config_read_stream_byte();
    unused = dram_config_read_stream_byte();
    sustain = dram_config_read_stream_byte();
    vel_sens = dram_config_read_stream_byte();
    mod_amt = dram_config_read_stream_byte();

    (void)unused;  /* Suppress unused warning */

    /* Apply velocity scaling to amplitude */
    final_level = dram_config_apply_velocity(amplitude, vel_sens);

    /* Store amplitude data in voice page slot */
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base, dispatch);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 1, base_level);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 2, final_level);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 3, env_ctrl);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 4, attack_rate);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 5, sustain);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 6, vel_sens);
    voice_page_write(g_intmem.voice_page_num, g_intmem.voice_slot_base + 7, mod_amt);

    /* Update slot mapping for periodic amplitude modulation
     * Original has complex conditions based on env_ctrl bits, simplified here */
    if (env_ctrl & 0x80) {  /* Envelope enabled */
        dram_config_update_slot_mapping();
    }

    /* Continue dispatch */
    dram_config_advance_and_dispatch();
}

/*============================================================================
 * Modulation Write to D-RAM (CODE:9FCD)
 *
 * Called during periodic voice update to write pitch modulation.
 * Computes: final_pitch = base_pitch + (modulation * base_pitch / scale)
 *
 * The modulation value is a signed 16-bit number (mod_hi:mod_lo).
 * The base pitch is a 19-bit value stored in the voice slot.
 *
 * Algorithm:
 *   1. Early exit if modulation unchanged
 *   2. Store new modulation to slot[0xE:0xF]
 *   3. Handle sign: if negative, negate and track sign
 *   4. If slot[0] bit 0 set: scale by 16 (fine pitch mode)
 *   5. Multiply: offset = modulation * base_pitch >> scale
 *   6. Re-apply sign
 *   7. Add offset to base pitch
 *   8. Write result to SAM D-RAM
 *============================================================================*/

void modulation_write_dram(int8_t mod_lo, int8_t mod_hi, uint8_t dispatch)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t slot_base = g_intmem.voice_slot_base;

    /* Read last modulation value */
    uint8_t last_lo = voice_page_read(page, slot_base + 0x0E);
    uint8_t last_hi = voice_page_read(page, slot_base + 0x0F);

    /* Check if modulation unchanged - early exit */
    if (last_lo == (uint8_t)mod_lo && last_hi == (uint8_t)mod_hi) {
        return;
    }

    /* Store new modulation value */
    voice_page_write(page, slot_base + 0x0E, (uint8_t)mod_lo);
    voice_page_write(page, slot_base + 0x0F, (uint8_t)mod_hi);

    /* Read base pitch from slot */
    uint8_t base_lo = voice_page_read(page, slot_base + 1);
    uint8_t base_mid = voice_page_read(page, slot_base + 2);
    uint8_t base_hi = voice_page_read(page, slot_base + 3) & 0x07;  /* Only bits 2:0 */

    /* Handle sign */
    int16_t mod_value = ((int16_t)mod_hi << 8) | ((uint8_t)mod_lo);
    uint8_t negative = (mod_hi < 0) ? 1 : 0;

    uint16_t abs_mod;
    if (negative) {
        abs_mod = (uint16_t)(-mod_value);
    } else {
        /* Positive: shift left by 1 (original firmware does RLC) */
        abs_mod = (uint16_t)mod_value << 1;
    }

    /* Check fine pitch mode (slot[0] bit 0) */
    uint8_t slot_flags = voice_page_read(page, slot_base);
    if (slot_flags & 0x01) {
        /* Fine pitch mode: scale by 16 (shift right by 4) */
        if ((abs_mod >> 8) < 0x10) {
            /* No overflow, do the scale */
            abs_mod <<= 4;  /* Original does nibble swap which is *16 */
        } else {
            /* Saturate to max */
            abs_mod = 0x7FFF;
        }
    }

    /*
     * Multiply modulation by base pitch
     *
     * Uses multiply_16x24 from sam_math.h which implements the exact
     * 8051 MUL AB sequence with proper carry propagation.
     */
    uint32_t base_pitch = ((uint32_t)base_hi << 16) |
                          ((uint32_t)base_mid << 8) |
                          base_lo;

    /* Compute offset = (abs_mod * base_pitch) >> 16 using 8x8 multiplies */
    uint32_t offset = multiply_16x24(abs_mod, base_pitch);

    /* Apply sign */
    int32_t signed_offset = negative ? -(int32_t)offset : (int32_t)offset;

    /* Add to base pitch */
    int32_t final_pitch = (int32_t)base_pitch + signed_offset;

    /* Clamp to valid 19-bit range */
    if (final_pitch < 0) {
        final_pitch = 0;
    } else if (final_pitch > 0x7FFFF) {
        final_pitch = 0x7FFFF;
    }

    /* Extract bytes */
    uint8_t final_lo = (uint8_t)(final_pitch & 0xFF);
    uint8_t final_mid = (uint8_t)((final_pitch >> 8) & 0xFF);
    uint8_t final_hi = (uint8_t)((final_pitch >> 16) & 0x07);

    /* Check for valid range before writing (original checks bit 2 of high byte) */
    if (final_hi & 0x04) {
        /* Out of range, don't write */
        return;
    }

    /* Write to SAM D-RAM */
    sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
    sam_write_reg(SAM_REG_DATA1, final_lo);
    sam_write_reg(SAM_REG_DATA2, final_mid);
    sam_write_reg(SAM_REG_DATA3, final_hi);
    sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);

    /* Suppress unused parameter warning */
    (void)dispatch;
}

/*============================================================================
 * Portamento (Pitch Glide) Update (CODE:A33E)
 *
 * Smoothly glides current pitch toward target pitch.
 * Called from periodic_voice_update when portamento is active.
 *
 * The algorithm computes a proportional step size:
 *   step = |delta| * rate
 * This means larger pitch distances result in larger steps (faster glide).
 * A minimum step of 'rate' is enforced when delta×rate rounds to zero.
 *============================================================================*/

void dram_slot_portamento_update(void)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t slot_base = g_intmem.voice_slot_base;

    /* Read target pitch (where we're gliding TO) from slot+1,2,3 */
    uint8_t target_lo = voice_page_read(page, slot_base + 1);
    uint8_t target_mid = voice_page_read(page, slot_base + 2);
    uint8_t target_hi = voice_page_read(page, slot_base + 3) & 0x07;

    /* Read current pitch (where we ARE) from slot+9,10,11 */
    uint8_t current_lo = voice_page_read(page, slot_base + 9);
    uint8_t current_mid = voice_page_read(page, slot_base + 10);
    uint8_t current_hi = voice_page_read(page, slot_base + 11) & 0x07;

    /* Read rate multiplier from slot+12 */
    uint8_t rate = voice_page_read(page, slot_base + 12);

    /* Compute delta = current - target (signed 24-bit) */
    int32_t current_pitch = ((int32_t)current_hi << 16) |
                            ((int32_t)current_mid << 8) |
                            current_lo;
    int32_t target_pitch = ((int32_t)target_hi << 16) |
                           ((int32_t)target_mid << 8) |
                           target_lo;

    int32_t delta = current_pitch - target_pitch;
    uint8_t negative = (delta < 0) ? 1 : 0;

    /* Take absolute value of delta */
    uint32_t abs_delta;
    if (negative) {
        abs_delta = (uint32_t)(-delta);
    } else {
        abs_delta = (uint32_t)delta;
    }

    /*
     * Compute step = (abs_delta * rate) >> 16
     * Using 8×8 multiplies to match firmware behavior.
     *
     * abs_delta is 19-bit max, rate is 8-bit
     * We multiply mid:hi portion by rate (ignoring low byte for speed)
     */
    uint8_t delta_mid = (uint8_t)((abs_delta >> 8) & 0xFF);
    uint8_t delta_hi = (uint8_t)((abs_delta >> 16) & 0x07);

    uint16_t p1 = (uint16_t)rate * delta_mid;
    uint16_t p2 = (uint16_t)rate * delta_hi;

    /* Combine: step = (p1 >> 8) + p2 (approximately) */
    uint32_t step = (p1 >> 8) + p2;

    /* Low byte contribution for more precision */
    uint8_t delta_lo = (uint8_t)(abs_delta & 0xFF);
    uint16_t p0 = (uint16_t)rate * delta_lo;
    step = ((uint32_t)(p0 >> 8) + p1 + ((uint32_t)p2 << 8)) >> 8;

    /* If step is zero, use rate as minimum step */
    if (step == 0) {
        step = rate;
    }

    /* Re-apply sign: if current > target, step should be negative */
    int32_t signed_step;
    if (negative) {
        /* Current < target, need to increase (positive step) */
        signed_step = (int32_t)step;
    } else {
        /* Current > target, need to decrease (negative step) */
        signed_step = -(int32_t)step;
    }

    /* Compute new pitch = current + signed_step (moving toward target) */
    int32_t new_pitch = current_pitch + signed_step;

    /* Check for overshoot:
     * If we were below target (negative delta) and now above, clamp.
     * If we were above target (positive delta) and now below, clamp.
     */
    int32_t new_delta = new_pitch - target_pitch;
    uint8_t overshot = 0;

    if (negative && new_delta >= 0) {
        /* Was below target, now at or above - overshot */
        overshot = 1;
    } else if (!negative && new_delta <= 0) {
        /* Was above target, now at or below - overshot */
        overshot = 1;
    }

    if (overshot) {
        /* Clamp to target and clear portamento flag */
        new_pitch = target_pitch;

        /* Clear portamento active flag (bit 7 of slot+4) */
        uint8_t flags = voice_page_read(page, slot_base + 4);
        voice_page_write(page, slot_base + 4, flags & 0x7F);
    }

    /* Clamp to valid 19-bit range */
    if (new_pitch < 0) {
        new_pitch = 0;
    } else if (new_pitch > 0x7FFFF) {
        new_pitch = 0x7FFFF;
    }

    /* Extract bytes */
    uint8_t new_lo = (uint8_t)(new_pitch & 0xFF);
    uint8_t new_mid = (uint8_t)((new_pitch >> 8) & 0xFF);
    uint8_t new_hi = (uint8_t)((new_pitch >> 16) & 0x07);

    /* Store new current pitch back to slot+9,10,11 */
    voice_page_write(page, slot_base + 9, new_lo);
    voice_page_write(page, slot_base + 10, new_mid);
    voice_page_write(page, slot_base + 11, new_hi);

    /* Update slot+3 high bits (preserve flags in bits 7:3) */
    uint8_t slot3 = voice_page_read(page, slot_base + 3);
    voice_page_write(page, slot_base + 3, (slot3 & 0xF8) | new_hi);

    /* Write to SAM D-RAM */
    sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
    sam_write_reg(SAM_REG_DATA1, new_lo);
    sam_write_reg(SAM_REG_DATA2, new_mid);
    sam_write_reg(SAM_REG_DATA3, new_hi);
    sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);

    /* Update direction indicator at slot+7 (for change detection) */
    uint8_t direction = (new_hi == 0 && new_mid == 0) ? 0x01 : 0xFF;
    voice_page_write(page, slot_base + 7, direction);
}

/*============================================================================
 * Apply Modulation Depth (CODE:A2E3)
 *
 * Applies velocity-based scaling to modulation input.
 * Called from amplitude update to scale envelope/LFO output.
 *
 * ⚠️ PORTING SHORTCUTS - TODO for full fidelity:
 * --------------------------------------------
 * 1. Velocity curve table: Original uses curve table at (0x11, midi_velocity-0x3C)
 *    for non-linear velocity response. We use linear (midi_velocity - 0x3C).
 *    Affects: Velocity feel/response curve.
 *
 * 2. Bank register preservation: Original saves/restores R1 (BANK0_R1).
 *    Not needed in C but affects register allocation on 8051.
 *============================================================================*/

uint8_t dram_slot_apply_mod_depth(uint8_t mod_input)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t slot_base = g_intmem.voice_slot_base;

    /* Read mod sensitivity from slot+6 */
    uint8_t mod_sensitivity = voice_page_read(page, slot_base + 6);

    /* If sensitivity is zero, no modulation - return input unchanged */
    if (mod_sensitivity == 0) {
        return mod_input;
    }

    /* Store masked input to slot[0] as current mod level */
    voice_page_write(page, slot_base, mod_input & 0x7F);

    /* Clear bit 5 of slot+4 (reset mod state flag) */
    uint8_t flags = voice_page_read(page, slot_base + 4);
    voice_page_write(page, slot_base + 4, flags & 0xDF);

    /*
     * Apply velocity-based scaling using signed_multiply_chain
     *
     * The original computes:
     *   result = signed_multiply_chain(velocity_curve[midi_velocity - 0x3C], mod_sensitivity)
     *
     * We use a simplified version: scale mod_input by sensitivity
     * Full implementation would need the velocity curve table lookup.
     */
    int8_t velocity_factor = (int8_t)(g_intmem.midi_velocity - 0x3C);
    int8_t scaled = signed_multiply_chain(velocity_factor, (int8_t)mod_sensitivity, (int8_t)mod_input);

    return (uint8_t)scaled;
}

/*============================================================================
 * Amplitude/Envelope Update (CODE:A18F)
 *
 * Updates amplitude for a D-RAM slot based on envelope state.
 * This is a complex function in the original firmware with many code paths.
 *
 * Simplified implementation focuses on:
 * 1. Reading envelope output
 * 2. Applying velocity scaling
 * 3. Writing amplitude to D-RAM
 *
 * ⚠️ PORTING SHORTCUTS - TODO for full fidelity:
 * --------------------------------------------
 * 1. Read-modify-write D-RAM: Original reads current D-RAM value and
 *    accumulates for envelope release curves. We just write new value.
 *    Affects: Smooth envelope release transitions.
 *
 * 2. Envelope block cross-reference: Original checks envelope state from
 *    one of 7 LFO/envelope blocks (indexed by slot+4 bits 2:0). We don't
 *    read the envelope block output.
 *    Affects: Dynamic envelope following.
 *
 * 3. Negative amplitude handling: Original has special path when amplitude
 *    goes negative (envelope inversion). Not implemented.
 *    Affects: Inverted envelope effects.
 *
 * 4. Done flag propagation: Original sets _0_4, _0_5 flags for voice
 *    management. We only set dram_slot_index.
 *    Affects: Voice release timing.
 *
 * 5. Velocity curve table lookup: Original uses a 128-byte curve table
 *    for non-linear velocity response. We use linear.
 *    Affects: Velocity feel/response.
 *============================================================================*/

void dram_slot_amplitude_update(uint8_t env_output, uint8_t gate_flags)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t slot_base = g_intmem.voice_slot_base;

    /* Apply modulation depth scaling */
    uint8_t mod_output = dram_slot_apply_mod_depth(env_output);

    /* Read control flags from slot+3 */
    uint8_t ctrl_flags = voice_page_read(page, slot_base + 3);

    /* Check bit 4 - if set, skip this update */
    if (ctrl_flags & 0x10) {
        /* Check bit 3 for alternate path */
        if (!(ctrl_flags & 0x08)) {
            return;
        }
        /* Set done flag and return */
        uint8_t slot0 = voice_page_read(page, slot_base);
        voice_page_write(page, slot_base, (slot0 & 0x7F));
        return;
    }

    /* Read envelope control from slot+4 */
    uint8_t env_ctrl = voice_page_read(page, slot_base + 4);

    /* Check envelope enable bits (0x18) */
    if ((env_ctrl & 0x18) == 0) {
        /* No envelope - simple direct amplitude write */
        uint8_t base_level = voice_page_read(page, slot_base + 1) & 0x3F;

        sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
        sam_write_reg(SAM_REG_DATA1, base_level);
        sam_write_reg(SAM_REG_DATA2, mod_output);
        sam_write_reg(SAM_REG_DATA3, 0);
        sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);
        goto simple_write;
    }

    /* Check envelope gate bit (bit 3) */
    if (!(env_ctrl & 0x08)) {
        /* Direct write path */
        uint8_t base_level = voice_page_read(page, slot_base + 1) & 0x3F;

        /* Write amplitude to D-RAM */
        sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
        sam_write_reg(SAM_REG_DATA1, base_level);
        sam_write_reg(SAM_REG_DATA2, (env_output << 4) >> 1);
        sam_write_reg(SAM_REG_DATA3, env_output >> 5);
        sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);
        goto check_done;
    }

    /* Envelope-gated amplitude path */
    {
        /* Read velocity/mix attenuation from slot+8,9 */
        uint8_t atten_lo = voice_page_read(page, slot_base + 8);
        uint8_t atten_hi = voice_page_read(page, slot_base + 9);

        uint8_t atten_hi_masked = atten_hi & 0x7F;

        /* Check for zero attenuation */
        if (atten_hi_masked == 0 && (atten_lo & 0xF0) == 0) {
            /* Zero attenuation - minimal amplitude */
            uint8_t base_level = voice_page_read(page, slot_base + 1) & 0x3F;

            sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
            sam_write_reg(SAM_REG_DATA1, base_level);
            sam_write_reg(SAM_REG_DATA2, 0);
            sam_write_reg(SAM_REG_DATA3, 0);
            sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);
        } else {
            /*
             * Scale mod_output by attenuation
             * attenuation is 12-bit: (atten_lo >> 4) | (atten_hi << 4)
             */
            uint16_t attenuation = ((uint16_t)(atten_lo >> 4)) | ((uint16_t)atten_hi << 4);

            /* Multiply: result = mod_output * attenuation */
            uint16_t p1 = (uint16_t)mod_output * (uint8_t)(attenuation & 0xFF);
            uint16_t p2 = (uint16_t)mod_output * (uint8_t)(attenuation >> 8);

            uint8_t result_lo = (uint8_t)(p1 >> 8);
            uint8_t result_mid = (uint8_t)p2 + result_lo;
            uint8_t result_hi = (uint8_t)(p2 >> 8);
            if (result_mid < result_lo) result_hi--;  /* Handle borrow */

            /* Check slot[0] bit 2 for read-modify-write mode */
            uint8_t slot0 = voice_page_read(page, slot_base);
            if (slot0 & 0x04) {
                /* Read-modify-write: read current D-RAM value and accumulate */
                /* For simplicity, just write the new value */
            }

            /* Write to D-RAM */
            uint8_t base_level = voice_page_read(page, slot_base + 1) & 0x3F;

            sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
            sam_write_reg(SAM_REG_DATA1, base_level | (result_lo & 0xC0));
            sam_write_reg(SAM_REG_DATA2, result_mid);
            sam_write_reg(SAM_REG_DATA3, result_hi & 0x07);
            sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);
        }
    }

check_done:
    /* Check if we should mark slot as done */
    ctrl_flags = voice_page_read(page, slot_base + 3);
    if (ctrl_flags & 0x10) {
        /* Already handled above */
        return;
    }

simple_write:
    /* Update done flag based on gate_flags */
    (void)gate_flags;  /* Used in full implementation for envelope state */

    /* Mark update complete in slot index */
    g_intmem.dram_slot_index = 0x0F;
}
