/**
 * SAM8905 Controller Firmware - Test Main
 *
 * Simple test harness for host-based testing.
 */

#include <stdio.h>
#include <string.h>
#include "sam_firmware.h"
#include "sam_pitch_tables.h"

/* ROM data placeholder */
CODE uint8_t g_rom[0x10000];

/*============================================================================
 * Test Helpers
 *============================================================================*/

static void print_intmem_summary(void)
{
    printf("INTMEM Summary:\n");
    printf("  dram_slot_free_list: 0x%02X\n", g_intmem.dram_slot_free_list);
    printf("  active_voice_list_head: 0x%02X\n", g_intmem.active_voice_list_head);
    printf("  pending_voice_list: 0x%02X\n", g_intmem.pending_voice_list);
    printf("  dram_slot_count: %d\n", g_intmem.dram_slot_count);
    printf("  remaining_slots: %d\n", g_intmem.remaining_slots);
    printf("  lfsr_state: 0x%02X\n", g_intmem.lfsr_state);
    printf("\n");
}

static void print_free_list(void)
{
    printf("D-RAM Free List:\n");
    printf("  0x7E (slot 0): -> %d\n", g_intmem.dram_free_list_0);
    printf("  0x7F (slot 1): -> %d\n", g_intmem.dram_free_list_1);
    for (int i = 0; i < 14; i++) {
        printf("  0x%02X (slot %d): -> %d\n",
               0x80 + i, i + 2, g_intmem_upper.dram_free_list[i]);
    }
    printf("\n");
}

static void print_channel_algo(void)
{
    printf("Channel → Algorithm:\n");
    for (int i = 0; i < 16; i++) {
        printf("  Ch %2d: Algo %d\n", i, g_intmem_upper.channel_algorithm[i]);
    }
    printf("\n");
}

/*============================================================================
 * Tests
 *============================================================================*/

static int test_extmem_clear(void)
{
    printf("=== Test: extmem_clear_all ===\n");

    /* Dirty the memory */
    memset(&g_extmem, 0xAA, sizeof(g_extmem));

    /* Clear it */
    sam_extmem_clear_all();

    /* Verify */
    uint8_t *ptr = (uint8_t *)&g_extmem;
    for (size_t i = 0; i < SAM_RAM_SIZE; i++) {
        if (ptr[i] != 0x00) {
            printf("FAIL: extmem[0x%04zX] = 0x%02X (expected 0x00)\n", i, ptr[i]);
            return 1;
        }
    }

    printf("PASS\n\n");
    return 0;
}

static int test_slot_manager_init(void)
{
    printf("=== Test: slot_manager_init ===\n");

    /* Clear everything first */
    memset(&g_intmem, 0xFF, sizeof(g_intmem));
    memset(&g_intmem_upper, 0xFF, sizeof(g_intmem_upper));

    /* Initialize */
    sam_slot_manager_init();

    /* Verify free list head */
    if (g_intmem.dram_slot_free_list != 0x00) {
        printf("FAIL: free list head = 0x%02X (expected 0x00)\n",
               g_intmem.dram_slot_free_list);
        return 1;
    }

    /* Verify free list chain */
    if (g_intmem.dram_free_list_0 != 0x01) {
        printf("FAIL: slot 0 -> %d (expected 1)\n", g_intmem.dram_free_list_0);
        return 1;
    }
    if (g_intmem.dram_free_list_1 != 0x02) {
        printf("FAIL: slot 1 -> %d (expected 2)\n", g_intmem.dram_free_list_1);
        return 1;
    }
    for (int i = 0; i < 13; i++) {
        uint8_t expected = i + 3;
        if (g_intmem_upper.dram_free_list[i] != expected) {
            printf("FAIL: slot %d -> %d (expected %d)\n",
                   i + 2, g_intmem_upper.dram_free_list[i], expected);
            return 1;
        }
    }
    if (g_intmem_upper.dram_free_list[13] != 0xFF) {
        printf("FAIL: slot 15 -> 0x%02X (expected 0xFF)\n",
               g_intmem_upper.dram_free_list[13]);
        return 1;
    }

    /* Verify channel algorithm assignments */
    uint8_t expected_algo[] = {1, 2, 3, 5, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 16; i++) {
        if (g_intmem_upper.channel_algorithm[i] != expected_algo[i]) {
            printf("FAIL: channel %d algo = %d (expected %d)\n",
                   i, g_intmem_upper.channel_algorithm[i], expected_algo[i]);
            return 1;
        }
    }

    /* Verify voice lists cleared */
    if (g_intmem.active_voice_list_head != 0xFF) {
        printf("FAIL: active_voice_list_head = 0x%02X (expected 0xFF)\n",
               g_intmem.active_voice_list_head);
        return 1;
    }
    if (g_intmem.pending_voice_list != 0xFF) {
        printf("FAIL: pending_voice_list = 0x%02X (expected 0xFF)\n",
               g_intmem.pending_voice_list);
        return 1;
    }

    print_intmem_summary();
    print_free_list();
    print_channel_algo();

    printf("PASS\n\n");
    return 0;
}

static int test_init_all(void)
{
    printf("=== Test: sam_init_all ===\n");

    /* Clear everything */
    memset(&g_intmem, 0xFF, sizeof(g_intmem));
    memset(&g_intmem_upper, 0xFF, sizeof(g_intmem_upper));
    memset(&g_extmem, 0xFF, sizeof(g_extmem));

    /* Initialize */
    sam_init_all();

    /* Verify mod LFO rate */
    if (g_extmem.mod_lfo_rate != 0x38) {
        printf("FAIL: mod_lfo_rate = 0x%02X (expected 0x38)\n",
               g_extmem.mod_lfo_rate);
        return 1;
    }

    /* Verify LFSR state */
    if (g_intmem.lfsr_state != 0x01) {
        printf("FAIL: lfsr_state = 0x%02X (expected 0x01)\n",
               g_intmem.lfsr_state);
        return 1;
    }

    printf("PASS\n\n");
    return 0;
}

static int test_pitch_table_init(void)
{
    printf("=== Test: sam_pitch_table_init (transpose=0) ===\n");

    /* Clear pitch tables */
    memset(g_extmem.pitch_table_lo, 0xFF, 128);
    memset(g_extmem.pitch_table_mid, 0xFF, 128);
    memset(g_extmem.pitch_table_hi, 0xFF, 128);

    /* Set transpose to 0 */
    g_intmem.current_slot_id = 0;

    /* Initialize pitch tables */
    sam_pitch_table_init();

    /* With transpose=0, output should match input exactly */
    int errors = 0;
    for (int note = 0; note < 128; note++) {
        if (g_extmem.pitch_table_lo[note] != g_pitch_base_lo[note]) {
            if (errors < 5) {
                printf("FAIL: pitch_lo[%d] = 0x%02X (expected 0x%02X)\n",
                       note, g_extmem.pitch_table_lo[note], g_pitch_base_lo[note]);
            }
            errors++;
        }
        if (g_extmem.pitch_table_mid[note] != g_pitch_base_mid[note]) {
            if (errors < 5) {
                printf("FAIL: pitch_mid[%d] = 0x%02X (expected 0x%02X)\n",
                       note, g_extmem.pitch_table_mid[note], g_pitch_base_mid[note]);
            }
            errors++;
        }
        if (g_extmem.pitch_table_hi[note] != (g_pitch_base_hi[note] & 0x07)) {
            if (errors < 5) {
                printf("FAIL: pitch_hi[%d] = 0x%02X (expected 0x%02X)\n",
                       note, g_extmem.pitch_table_hi[note], g_pitch_base_hi[note] & 0x07);
            }
            errors++;
        }
    }

    if (errors > 0) {
        printf("Total errors: %d\n", errors);
        return 1;
    }

    /* Print a few sample values */
    printf("Sample pitch values (note: lo/mid/hi):\n");
    for (int note = 0; note < 128; note += 16) {
        printf("  %3d: %02X/%02X/%02X\n", note,
               g_extmem.pitch_table_lo[note],
               g_extmem.pitch_table_mid[note],
               g_extmem.pitch_table_hi[note]);
    }

    printf("PASS\n\n");
    return 0;
}

static int test_pitch_table_transpose(void)
{
    printf("=== Test: sam_pitch_table_init (transpose=64) ===\n");

    /*
     * The pitch table algorithm computes:
     *   multiplier = base >> 3 (16-bit extract from 24-bit base)
     *   offset = (transpose × multiplier) >> 8
     *   result = base + offset
     *
     * This gives: result ≈ base × (1 + transpose/2048)
     *
     * For transpose=64:
     *   result ≈ base × (1 + 64/2048) = base × 1.03125 (~3.1% increase)
     *
     * Maximum range (transpose=±127/128) is about ±6.2% (≈±1 semitone).
     * This is a fine-tuning/pitch-bend parameter.
     */

    /* Set transpose to 64 */
    g_intmem.current_slot_id = 64;

    /* Initialize pitch tables */
    sam_pitch_table_init();

    /* Check a few notes */
    int errors = 0;
    double expected_ratio = 1.0 + 64.0/2048.0;  /* 1.03125 */

    printf("Comparing base vs transposed (+64), expected ratio ~%.4f:\n", expected_ratio);
    for (int note = 64; note < 128; note += 8) {  /* Use higher notes for better resolution */
        uint32_t base = ((uint32_t)g_pitch_base_hi[note] << 16) |
                        ((uint32_t)g_pitch_base_mid[note] << 8) |
                        g_pitch_base_lo[note];
        uint32_t result = ((uint32_t)g_extmem.pitch_table_hi[note] << 16) |
                          ((uint32_t)g_extmem.pitch_table_mid[note] << 8) |
                          g_extmem.pitch_table_lo[note];

        double ratio = (double)result / base;
        printf("  %3d: base=0x%06X result=0x%06X ratio=%.4f\n",
               note, base, result, ratio);

        /* Allow 1% tolerance for integer rounding */
        if (ratio < expected_ratio * 0.99 || ratio > expected_ratio * 1.01) {
            errors++;
        }
    }

    if (errors > 0) {
        printf("FAIL: %d notes outside expected range\n", errors);
        return 1;
    }

    printf("PASS\n\n");
    return 0;
}

/*============================================================================
 * Main
 *============================================================================*/

int main(void)
{
    int failures = 0;

    printf("SAM8905 Controller Firmware Tests\n");
    printf("Device: %s\n", SAM_DEVICE_NAME);
    printf("RAM size: 0x%X (%d KB)\n", SAM_RAM_SIZE, SAM_RAM_SIZE / 1024);
    printf("\n");

    failures += test_extmem_clear();
    failures += test_slot_manager_init();
    failures += test_init_all();
    failures += test_pitch_table_init();
    failures += test_pitch_table_transpose();

    printf("=================================\n");
    if (failures == 0) {
        printf("All tests PASSED\n");
    } else {
        printf("%d test(s) FAILED\n", failures);
    }

    return failures;
}
