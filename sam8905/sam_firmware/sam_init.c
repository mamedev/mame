/**
 * SAM8905 Controller Firmware - Initialization
 *
 * Memory clearing, slot manager setup, pitch table computation.
 * Based on MS4 firmware analysis.
 */

#include "sam_firmware.h"
#include "sam_math.h"
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

    /* Get transpose offset from INTMEM 0x34 */
    transpose = (int8_t)g_intmem.current_slot_id;

    for (note = 0; note < 128; note++) {
        /* Read base pitch from embedded ROM tables */
        uint8_t base_lo  = g_pitch_base_lo[note];
        uint8_t base_mid = g_pitch_base_mid[note];
        uint8_t base_hi  = g_pitch_base_hi[note];

        /* Scale pitch using math utility (see sam_math.h) */
        scale_pitch_24bit(
            base_lo, base_mid, base_hi,
            transpose,
            &g_extmem.pitch_table_lo[note],
            &g_extmem.pitch_table_mid[note],
            &g_extmem.pitch_table_hi[note]
        );
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
