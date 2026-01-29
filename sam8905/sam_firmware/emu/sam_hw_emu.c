/**
 * SAM8905 Hardware Interface - Emulator Implementation
 *
 * Implements sam_write_reg/sam_read_reg for the emulator platform.
 * Translates register-level access to sam8905_emu API calls.
 */

#include "sam8905_emu.h"
#include "../sam_hw.h"
#include <stdio.h>

/*============================================================================
 * Register State
 *
 * The SAM8905 uses a register interface where:
 * - Reg 0 (ADDR_DATA): Address for memory access / data byte 0
 * - Reg 1 (DATA1): Data byte 1 (high byte for D-RAM)
 * - Reg 2 (DATA2): Data byte 2 (low byte for D-RAM)
 * - Reg 3 (DATA3): Data byte 3 (for A-RAM writes)
 * - Reg 4 (CTRL): Control - triggers memory operation
 *============================================================================*/

static uint8_t g_sam_addr = 0;      /* Current address */
static uint8_t g_sam_data1 = 0;     /* Data byte 1 */
static uint8_t g_sam_data2 = 0;     /* Data byte 2 */
static uint8_t g_sam_data3 = 0;     /* Data byte 3 */

/* A-RAM write state - needs to accumulate 32 words (64 bytes) */
static uint8_t g_aram_slot = 0;     /* Current A-RAM slot being written */
static uint8_t g_aram_offset = 0;   /* Offset within slot (0-31 words) */

/* Debug output control */
#ifndef SAM_HW_DEBUG
#define SAM_HW_DEBUG 0
#endif

/*============================================================================
 * sam_write_reg - Emulator Implementation
 *============================================================================*/

void sam_write_reg(uint8_t reg, uint8_t value)
{
    switch (reg) {
        case SAM_REG_ADDR_DATA:
            g_sam_addr = value;
            break;

        case SAM_REG_DATA1:
            g_sam_data1 = value;
            break;

        case SAM_REG_DATA2:
            g_sam_data2 = value;
            break;

        case SAM_REG_DATA3:
            g_sam_data3 = value;
            break;

        case SAM_REG_CTRL:
            /* Control write triggers memory operation */
            switch (value) {
                case SAM_CTRL_DRAM_WR: {
                    /*
                     * D-RAM write: 19-bit value from DATA1:DATA2:DATA3
                     * Register mapping (matches MAME sam8905.cpp):
                     *   DATA1 = bits 7:0
                     *   DATA2 = bits 15:8
                     *   DATA3 = bits 18:16 (only low 3 bits)
                     */
                    uint32_t dram_value = ((uint32_t)(g_sam_data3 & 0x07) << 16) |
                                          ((uint32_t)g_sam_data2 << 8) |
                                          (uint32_t)g_sam_data1;
#if SAM_HW_DEBUG
                    printf("SAM D-RAM[0x%02X] = 0x%05X (d1=0x%02X d2=0x%02X d3=0x%02X)\n",
                           g_sam_addr, dram_value, g_sam_data1, g_sam_data2, g_sam_data3);
#endif
                    sam8905_write_dram(g_sam_addr, dram_value);
                    break;
                }

                case SAM_CTRL_ARAM_WR: {
                    /*
                     * A-RAM write: 15-bit instruction from DATA0:DATA1
                     * The firmware writes A-RAM in 4-byte chunks:
                     * ADDR_DATA = low byte of inst N
                     * DATA1 = high byte of inst N
                     * DATA2 = low byte of inst N+1
                     * DATA3 = high byte of inst N+1
                     *
                     * Two 15-bit instructions per write.
                     */
                    uint16_t inst0 = ((uint16_t)g_sam_data1 << 8) | g_sam_addr;
                    uint16_t inst1 = ((uint16_t)g_sam_data3 << 8) | g_sam_data2;

                    /* Calculate A-RAM addresses based on slot and offset */
                    uint8_t aram_addr0 = (g_aram_slot << 5) | g_aram_offset;
                    uint8_t aram_addr1 = aram_addr0 + 1;

#if SAM_HW_DEBUG
                    printf("SAM A-RAM[0x%02X] = 0x%04X, A-RAM[0x%02X] = 0x%04X\n",
                           aram_addr0, inst0, aram_addr1, inst1);
#endif
                    sam8905_write_aram(aram_addr0, inst0);
                    sam8905_write_aram(aram_addr1, inst1);

                    /* Advance offset (2 words per write) */
                    g_aram_offset += 2;
                    if (g_aram_offset >= 32) {
                        g_aram_offset = 0;
                        /* Could auto-advance slot, but firmware sets it explicitly */
                    }
                    break;
                }

                case SAM_CTRL_RESET:
#if SAM_HW_DEBUG
                    printf("SAM Reset\n");
#endif
                    /* Reset - could call sam8905_init() but emulator handles this */
                    break;

                default:
#if SAM_HW_DEBUG
                    printf("SAM CTRL unknown: 0x%02X\n", value);
#endif
                    break;
            }
            break;

        default:
            /* Unknown register */
            break;
    }
}

/*============================================================================
 * sam_read_reg - Emulator Implementation
 *============================================================================*/

uint8_t sam_read_reg(uint8_t reg)
{
    /* D-RAM/A-RAM read not commonly used by firmware */
    switch (reg) {
        case SAM_REG_ADDR_DATA:
            return g_sam_addr;
        case SAM_REG_DATA1:
            return g_sam_data1;
        case SAM_REG_DATA2:
            return g_sam_data2;
        case SAM_REG_DATA3:
            return g_sam_data3;
        default:
            return 0xFF;
    }
}

/*============================================================================
 * sam_aram_set_slot - Set current A-RAM slot for writing
 *
 * The firmware sets the A-RAM slot address before writing algorithm data.
 * This function is called to track which slot is being written.
 *============================================================================*/

void sam_aram_set_slot(uint8_t slot)
{
    g_aram_slot = slot & 0x07;  /* 8 slots at 44.1kHz */
    g_aram_offset = 0;
}
