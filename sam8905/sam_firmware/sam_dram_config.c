/**
 * SAM8905 Controller Firmware - D-RAM Configuration Handlers
 *
 * Implementation of voice init data stream decoder.
 */

#include "sam_dram_config.h"
#include "sam_firmware.h"
#include <stddef.h>  /* For NULL */

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
 * Dispatch Loop (CODE:AB4C)
 *
 * Main dispatch loop for D-RAM configuration.
 * Processes 16 D-RAM words (remaining_slots countdown).
 *============================================================================*/

void dram_config_dispatch(void)
{
    uint8_t dispatch;

    while (g_intmem.remaining_slots > 0) {
        /* Read dispatch byte from stream */
        dispatch = dram_config_peek_stream_byte();

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
