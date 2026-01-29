/**
 * SAM8905 Controller Firmware - Test Main
 *
 * Simple test harness for host-based testing.
 */

#include <stdio.h>
#include <string.h>
#include "sam_firmware.h"

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

    printf("=================================\n");
    if (failures == 0) {
        printf("All tests PASSED\n");
    } else {
        printf("%d test(s) FAILED\n", failures);
    }

    return failures;
}
