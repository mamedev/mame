/**
 * SAM8905 Controller Firmware - Initialization
 *
 * Memory clearing, slot manager setup, pitch table computation.
 * Based on MS4 firmware analysis.
 */

#include "sam_firmware.h"
#include "sam_pitch_tables.h"

/*============================================================================
 * Global State
 *============================================================================*/

IDATA intmem_t g_intmem;
IDATA intmem_upper_t g_intmem_upper;
XDATA extmem_t g_extmem;

/*============================================================================
 * extmem_clear_all (CODE:D454)
 *
 * Clears all 8KB of external RAM to zero.
 *
 * Original:
 *   D454: MOV DPTR,#0x0000
 *   D457: CLR A
 *   D458: MOVX @DPTR,A
 *   D459: INC DPTR
 *   D45A: MOV A,DPH
 *   D45C: CJNE A,#0x20,D458   ; loop until DPTR >= 0x2000
 *   D45F: RET
 *============================================================================*/

void sam_extmem_clear_all(void)
{
    uint16_t i;
    uint8_t *ptr = (uint8_t *)&g_extmem;

    for (i = 0; i < SAM_RAM_SIZE; i++) {
        ptr[i] = 0x00;
    }
}

/*============================================================================
 * voice_pages_clear (CODE:B70B)
 *
 * Clears extended voice pages (EXTMEM 0x14D5-0x1CE4).
 *
 * Original:
 *   B70B: MOV DPTR,#0x14D5
 *   B70E: CLR A
 *   B70F: MOVX @DPTR,A
 *   B710: INC DPTR
 *   B711: MOV A,DPH
 *   B713: CJNE A,#0x1D,B70F   ; loop until DPH >= 0x1D
 *   B716: RET
 *============================================================================*/

void sam_voice_pages_clear(void)
{
    uint16_t i;

    /* Clear extended_voice_page[8][256] = 2048 bytes */
    for (i = 0; i < sizeof(g_extmem.extended_voice_page); i++) {
        ((uint8_t *)g_extmem.extended_voice_page)[i] = 0x00;
    }

    /* Clear voice_ctrl_2[16] */
    for (i = 0; i < sizeof(g_extmem.voice_ctrl_2); i++) {
        g_extmem.voice_ctrl_2[i] = 0x00;
    }
}

/*============================================================================
 * slot_manager_init (CODE:9904)
 *
 * Initializes:
 * - D-RAM free list (INTMEM 0x7E-0x8D): linked list 0→1→2→...→15→0xFF
 * - Free list head (INTMEM 0x53) = 0x00
 * - Channel-to-algorithm assignments (INTMEM 0x8E-0x9D)
 * - Algorithm slot lookup table (INTMEM 0x9E-0xA5)
 * - Voice control areas
 *
 * Original (partial):
 *   9904: MOV R0,#0x7E       ; start of free list
 *   9906: MOV R1,#0x01       ; first value (slot 0 → 1)
 *   9908: MOV @R0,R1         ; store link
 *   9909: INC R0             ; next address
 *   990A: INC R1             ; next slot
 *   990B: CJNE R0,#0x8D,9908 ; loop until 0x8D
 *   990E: MOV @R0,#0xFF      ; terminate list
 *   9910: MOV 0x53,#0x00     ; free list head = 0
 *============================================================================*/

void sam_slot_manager_init(void)
{
    uint8_t i;

    /*
     * Initialize D-RAM free list as linked list:
     * INTMEM[0x7E] = 1 (slot 0 → slot 1)
     * INTMEM[0x7F] = 2 (slot 1 → slot 2)
     * ...
     * INTMEM[0x8C] = 15 (slot 14 → slot 15)
     * INTMEM[0x8D] = 0xFF (slot 15 → end)
     */

    /* First two entries are in intmem_t */
    g_intmem.dram_free_list_0 = 0x01;
    g_intmem.dram_free_list_1 = 0x02;

    /* Remaining entries in intmem_upper_t (0x80-0x8D) */
    for (i = 0; i < 13; i++) {
        g_intmem_upper.dram_free_list[i] = i + 3;
    }
    g_intmem_upper.dram_free_list[13] = VOICE_LIST_END;  /* 0x8D = 0xFF */

    /* Free list head */
    g_intmem.dram_slot_free_list = 0x00;

    /* D-RAM slot count */
    g_intmem.dram_slot_count = SAM_DRAM_SLOTS;
    g_intmem.remaining_slots = SAM_DRAM_SLOTS;

    /*
     * Channel-to-algorithm assignments (from table at CODE:DC0B):
     * Ch 0-3: unique algorithms (1,2,3,5)
     * Ch 4-7: shared algorithm 4
     * Ch 8-15: unassigned (0)
     */
    g_intmem_upper.channel_algorithm[0] = 1;
    g_intmem_upper.channel_algorithm[1] = 2;
    g_intmem_upper.channel_algorithm[2] = 3;
    g_intmem_upper.channel_algorithm[3] = 5;
    for (i = 4; i < 8; i++) {
        g_intmem_upper.channel_algorithm[i] = 4;
    }
    for (i = 8; i < 16; i++) {
        g_intmem_upper.channel_algorithm[i] = 0;
    }

    /* Clear active/pending voice lists */
    g_intmem.active_voice_list_head = VOICE_LIST_END;
    g_intmem.pending_voice_list = VOICE_LIST_END;

    /* Clear highest note and note count per channel */
    for (i = 0; i < 4; i++) {
        g_intmem_upper.channel_highest_note[i] = 0;
        g_intmem_upper.channel_note_count[i] = 0;
    }

    /* Clear voice control areas */
    for (i = 0; i < 16; i++) {
        g_extmem.voice_ctrl_1[i] = 0x00;
        g_extmem.voice_ctrl_2[i] = 0x00;
    }

    /* Clear algorithm pool */
    for (i = 0; i < 8; i++) {
        g_extmem.algorithm_pool[i] = 0x00;
    }
}

/*============================================================================
 * pitch_table_init (CODE:9B16)
 *
 * Computes 128-note pitch frequency table with transpose offset.
 *
 * Input:  INTMEM 0x34 (current_slot_id) = transpose offset (signed)
 * Output: 128 × 3-byte frequency entries in EXTMEM pitch tables
 *
 * Source ROM tables (128 bytes each):
 *   CODE:92B1 = base frequency low byte
 *   CODE:9331 = base frequency mid byte
 *   CODE:93B1 = base frequency high byte (3-bit, 0-7)
 *
 * Algorithm:
 *   1. Read 24-bit base pitch from ROM tables
 *   2. Extract 16-bit multiplier by right-shifting base by 3 bits
 *   3. Multiply by transpose offset (signed)
 *   4. Add offset to base pitch
 *   5. Store 24-bit result (high byte masked to 3 bits)
 *
 * Original disassembly at CODE:9B16-9BA6
 *============================================================================*/

void sam_pitch_table_init(void)
{
    uint8_t note;
    int8_t transpose;
    uint8_t abs_transpose;
    uint8_t neg_flag;

    /* Get transpose offset from INTMEM 0x34 */
    transpose = (int8_t)g_intmem.current_slot_id;

    for (note = 0; note < 128; note++) {
        uint8_t base_lo, base_mid, base_hi;
        uint8_t mul_lo, mul_hi;
        uint16_t product1, product2;
        int32_t offset;
        int32_t result;

        /* Read base pitch from embedded ROM tables */
        base_lo  = g_pitch_base_lo[note];
        base_mid = g_pitch_base_mid[note];
        base_hi  = g_pitch_base_hi[note];

        /*
         * Extract 16-bit multiplier from 24-bit base by right-shifting 3 bits.
         * Original uses SWAP+RL+AND to extract bits.
         *
         * 8051 SWAP swaps nibbles, RL rotates left by 1.
         * SWAP+RL effectively does: result[7:0] = [b2 b1 b0 b7 b6 b5 b4 b3]
         * This is a left rotation by 5, or equivalently right rotation by 3.
         *
         * AND 0x1F extracts bits [7:3] of original into [4:0] of result.
         * AND 0xE0 extracts bits [2:0] of original into [7:5] of result.
         */

        /* SWAP+RL on base_lo, AND 0x1F: extracts base_lo[7:3] */
        mul_lo = (base_lo >> 3) & 0x1F;

        /* SWAP+RL on base_mid gives ((mid << 5) | (mid >> 3)) */
        /* AND 0xE0: extracts base_mid[2:0] << 5, combine with base_lo bits */
        mul_lo |= (base_mid << 5);  /* top 3 bits from mid[2:0] */

        /* SWAP+RL on base_mid, AND 0x1F: extracts base_mid[7:3] */
        mul_hi = (base_mid >> 3) & 0x1F;

        /* SWAP+RL on base_hi, OR with mul_hi (no AND in original) */
        /* For base_hi in range 0-7, this gives (base_hi << 5) */
        mul_hi |= (base_hi << 5) | (base_hi >> 3);

        /*
         * Handle sign of transpose.
         * If negative, use absolute value for multiply, then negate result.
         */
        neg_flag = 0;
        if (transpose < 0) {
            neg_flag = 1;
            abs_transpose = (uint8_t)(-transpose);
        } else {
            abs_transpose = (uint8_t)transpose;
        }

        /*
         * 16-bit × 8-bit multiply:
         * offset = abs_transpose × (mul_hi:mul_lo)
         *
         * Original assembly does two 8×8 multiplies:
         *   product1 = abs_transpose × mul_lo
         *   product2 = abs_transpose × mul_hi
         *
         * Then combines with carry-based rounding:
         *   RLC A after first MUL captures product1_lo[7] into carry
         *   ADDC adds this carry when combining products
         *
         * Result: offset_lo = product1_hi + product2_lo + round_bit
         *         offset_hi = product2_hi + carry_out
         *
         * This computes (transpose × multiplier) >> 8 with rounding.
         */
        product1 = (uint16_t)abs_transpose * mul_lo;
        product2 = (uint16_t)abs_transpose * mul_hi;

        {
            /* Round bit from product1_lo[7] */
            uint8_t round_bit = (product1 >> 7) & 1;
            uint8_t product1_hi = (uint8_t)(product1 >> 8);
            uint8_t product2_lo = (uint8_t)(product2 & 0xFF);
            uint8_t product2_hi = (uint8_t)(product2 >> 8);
            uint16_t sum;

            /* offset_lo = product1_hi + product2_lo + round_bit */
            sum = (uint16_t)product1_hi + product2_lo + round_bit;
            offset = sum & 0xFF;

            /* offset_hi = product2_hi + carry_out */
            offset |= ((uint32_t)(product2_hi + (sum >> 8)) << 8);
        }

        /* Negate if transpose was negative */
        if (neg_flag) {
            offset = -offset;
        }

        /* Add offset to 24-bit base pitch */
        result = ((int32_t)base_hi << 16) | ((int32_t)base_mid << 8) | base_lo;
        result += offset;

        /* Store result, masking high byte to 3 bits */
        g_extmem.pitch_table_lo[note] = (uint8_t)(result & 0xFF);
        g_extmem.pitch_table_mid[note] = (uint8_t)((result >> 8) & 0xFF);
        g_extmem.pitch_table_hi[note] = (uint8_t)((result >> 16) & 0x07);
    }
}

/*============================================================================
 * sam_init_all (CODE:98AD)
 *
 * Full initialization: ROM checksum (skipped), pitch table, voice/slot init.
 *============================================================================*/

void sam_init_all(void)
{
    /* Initialize SAM8905 chip */
    sam_init();

    /* Initialize slot manager */
    sam_slot_manager_init();

    /* Compute pitch tables */
    sam_pitch_table_init();

    /* Set default mod LFO rate */
    g_extmem.mod_lfo_rate = 0x38;
    g_extmem.mod_lfo_phase_lo = 0x00;
    g_extmem.mod_lfo_phase_hi = 0x00;
    g_extmem.mod_lfo_output = 0x00;

    /* Clear mod wheel sensitivity */
    for (uint8_t i = 0; i < 16; i++) {
        g_extmem.mod_wheel_sens[i] = 0x00;
    }

    /* Clear pitch bend */
    for (uint8_t i = 0; i < 32; i++) {
        g_extmem.pitch_bend[i] = 0x00;
    }

    /* Initialize LFSR for noise LFO */
    g_intmem.lfsr_state = 0x01;

    /* Clear octave shift */
    g_intmem.octave_shift = 0x00;
}
