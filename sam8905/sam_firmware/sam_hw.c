/**
 * SAM8905 Controller Firmware - Hardware Interface Implementation
 *
 * SAM8905 DSP chip operations: D-RAM, A-RAM, initialization.
 */

#include "sam_firmware.h"

/* Forward declarations */
static void dram_clear_all(void);

/*============================================================================
 * Platform-specific register access
 *
 * These must be provided by the platform layer (8051, emulator, etc.)
 * Default weak implementations for testing.
 *============================================================================*/

#ifndef SAM_HW_PLATFORM

/* Dummy register storage for testing without hardware */
static uint8_t sam_regs[8];

/* Write tracing for tests */
uint32_t g_sam_write_count = 0;

void sam_hw_reset_trace(void)
{
    g_sam_write_count = 0;
}

void sam_write_reg(uint8_t reg, uint8_t value)
{
    g_sam_write_count++;
    if (reg < 8) {
        sam_regs[reg] = value;
    }
}

uint8_t sam_read_reg(uint8_t reg)
{
    if (reg < 8) {
        return sam_regs[reg];
    }
    return 0xFF;
}

#endif /* !SAM_HW_PLATFORM */

/*============================================================================
 * sam_init
 *
 * Initialize SAM8905 chip.
 *
 * Original (from boot sequence):
 *   MOV DPTR,#0x8004
 *   MOV A,#0x40
 *   MOVX @DPTR,A        ; SAM reset/init
 *============================================================================*/

void sam_init(void)
{
    /* Send reset command */
    sam_write_reg(SAM_REG_CTRL, SAM_CTRL_RESET);

    /* Clear all D-RAM */
    dram_clear_all();

    /* Initialize D-RAM slot control words */
    sam_init_slots();
}

/*============================================================================
 * sam_dram_clear_all_impl (CODE:A53C)
 *
 * Clear all 256 D-RAM words to zero.
 *
 * Original:
 *   A53C: MOV R0,#0x00       ; start address
 *   A53E: CLR A
 *   A53F: MOV DPTR,#0x8000   ; SAM base
 *   A542: MOVX @DPTR,A       ; addr/data0 = 0
 *   A543: INC DPTR
 *   A544: MOVX @DPTR,A       ; data1 = 0
 *   A545: INC DPTR
 *   A546: MOVX @DPTR,A       ; data2 = 0
 *   A547: MOV DPTR,#0x8000
 *   A54A: MOV A,R0
 *   A54B: MOVX @DPTR,A       ; write address
 *   A54C: MOV DPTR,#0x8004
 *   A54F: MOV A,#0x05
 *   A551: MOVX @DPTR,A       ; trigger D-RAM write
 *   A552: INC R0
 *   A553: CJNE R0,#0x00,A53F ; loop 256 times
 *   A556: RET
 *============================================================================*/

static void dram_clear_all(void)
{
    uint16_t addr;

    for (addr = 0; addr < 256; addr++) {
        sam_write_reg(SAM_REG_ADDR_DATA, (uint8_t)addr);
        sam_write_reg(SAM_REG_DATA1, 0x00);
        sam_write_reg(SAM_REG_DATA2, 0x00);
        sam_write_reg(SAM_REG_CTRL, SAM_CTRL_DRAM_WR);
    }
}

/*============================================================================
 * sam_init_slots
 *
 * Initialize control word (offset 0x0F) of each D-RAM slot.
 *
 * For slot N:
 *   Address = (N << 4) | 0x0F
 *   Value = (nibble_swap(N) << 8) | 0x0F00
 *
 * This sets algorithm routing and full mix (MIXL=1, MIXR=7).
 *
 * NOTE: This is initialization code, not the firmware's read-modify-write
 * sam_dram_write_word15 (CODE:A523) which sets the ACTIVE bit on existing slots.
 *============================================================================*/

void sam_init_slots(void)
{
    uint8_t slot;

    for (slot = 0; slot < MAX_DRAM_SLOTS; slot++) {
        uint8_t addr = (slot << 4) | 0x0F;
        uint8_t slot_swapped = ((slot & 0x0F) << 4) | ((slot >> 4) & 0x0F);
        uint16_t value = ((uint16_t)slot_swapped << 8) | 0x0F00;

        sam_dram_write(addr, value);
    }
}

/*============================================================================
 * sam_aram_write_slot
 *
 * Write 64 bytes of algorithm data to A-RAM slot.
 *
 * A-RAM stores the DSP algorithm microcode for each voice.
 *============================================================================*/

void sam_aram_write_slot(uint8_t slot, const uint8_t *data)
{
    uint8_t i;

    /* Set A-RAM write mode with slot address */
    sam_write_reg(SAM_REG_ADDR_DATA, slot);

    /* Write 64 bytes in groups */
    for (i = 0; i < 64; i += 4) {
        sam_write_reg(SAM_REG_ADDR_DATA, data[i + 0]);
        sam_write_reg(SAM_REG_DATA1, data[i + 1]);
        sam_write_reg(SAM_REG_DATA2, data[i + 2]);
        sam_write_reg(SAM_REG_DATA3, data[i + 3]);
        sam_write_reg(SAM_REG_CTRL, SAM_CTRL_ARAM_WR);
    }
}
