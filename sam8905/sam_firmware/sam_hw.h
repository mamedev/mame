#pragma once

/**
 * SAM8905 Controller Firmware - Hardware Interface
 *
 * Abstraction layer for SAM8905 DSP chip access.
 * Platform implementations provide sam_write_reg/sam_read_reg.
 */

#include <stdint.h>

/*============================================================================
 * SAM8905 Register Map (offset from base address 0x8000)
 *============================================================================*/

#define SAM_REG_ADDR_DATA  0  /* 0x8000: address / data byte 0 */
#define SAM_REG_DATA1      1  /* 0x8001: data byte 1 */
#define SAM_REG_DATA2      2  /* 0x8002: data byte 2 */
#define SAM_REG_DATA3      3  /* 0x8003: data byte 3 */
#define SAM_REG_CTRL       4  /* 0x8004: control register */

/*============================================================================
 * SAM8905 Control Register Values
 *============================================================================*/

#define SAM_CTRL_RESET     0x40  /* Reset / init */
#define SAM_CTRL_DRAM_WR   0x05  /* D-RAM write mode */
#define SAM_CTRL_DRAM_RD   0x06  /* D-RAM read mode (if supported) */
#define SAM_CTRL_ARAM_WR   0x03  /* A-RAM (algorithm) write mode */

/*============================================================================
 * Platform-Specific Hardware Access (must be implemented per platform)
 *============================================================================*/

/**
 * Write to SAM8905 register
 *
 * @param reg   Register offset (0-4)
 * @param value Byte to write
 */
extern void sam_write_reg(uint8_t reg, uint8_t value);

/**
 * Read from SAM8905 register
 *
 * @param reg   Register offset (0-4)
 * @return      Byte read
 */
extern uint8_t sam_read_reg(uint8_t reg);

/*============================================================================
 * D-RAM Operations
 *
 * SAM8905 D-RAM is 19 bits wide (not 16!):
 *   REG_DATA1 = bits 7:0
 *   REG_DATA2 = bits 15:8
 *   REG_DATA3 = bits 18:16 (only low 3 bits used)
 *
 * See MAME src/devices/sound/sam8905.cpp lines 600-603 for register mapping.
 *============================================================================*/

/**
 * Write 19-bit value to D-RAM
 *
 * D-RAM has 256 addresses × 19 bits.
 * Address 0x00-0xFF, organized as 16 slots × 16 words.
 *
 * @param addr  D-RAM address (0x00-0xFF)
 * @param value 19-bit value to write (bits 18:0 used, upper bits ignored)
 */
static inline void sam_dram_write(uint8_t addr, uint32_t value)
{
    sam_write_reg(SAM_REG_ADDR_DATA, addr);
    sam_write_reg(SAM_REG_DATA1, (uint8_t)(value & 0xFF));          /* bits 7:0   */
    sam_write_reg(SAM_REG_DATA2, (uint8_t)((value >> 8) & 0xFF));   /* bits 15:8  */
    sam_write_reg(SAM_REG_DATA3, (uint8_t)((value >> 16) & 0x07));  /* bits 18:16 */
    sam_write_reg(SAM_REG_CTRL, SAM_CTRL_DRAM_WR);
}

/**
 * Write 19-bit value to D-RAM from individual bytes
 *
 * @param addr  D-RAM address (0x00-0xFF)
 * @param b0    Bits 7:0
 * @param b1    Bits 15:8
 * @param b2    Bits 18:16 (only low 3 bits used)
 */
static inline void sam_dram_write_bytes(uint8_t addr, uint8_t b0, uint8_t b1, uint8_t b2)
{
    sam_write_reg(SAM_REG_ADDR_DATA, addr);
    sam_write_reg(SAM_REG_DATA1, b0);
    sam_write_reg(SAM_REG_DATA2, b1);
    sam_write_reg(SAM_REG_DATA3, b2 & 0x07);
    sam_write_reg(SAM_REG_CTRL, SAM_CTRL_DRAM_WR);
}

/**
 * Clear all D-RAM to zero
 *
 * Clears all 256 words (16 slots × 16 words).
 */
static inline void sam_dram_clear_all(void)
{
    uint16_t addr;
    for (addr = 0; addr < 256; addr++) {
        sam_dram_write((uint8_t)addr, 0x00000);
    }
}

/**
 * Write to word index 15 (offset 0x0F) of a D-RAM slot
 *
 * Word 15 of each slot contains ALG/MIX control bits.
 * This is a simple write - NOT the read-modify-write that the
 * original firmware's sam_dram_write_word15 (CODE:A523) does.
 *
 * NOTE: The original firmware function reads the current value,
 * ORs bit 11 (ACTIVE), then writes back. Use this helper only
 * for initialization where you're setting the full value.
 *
 * @param slot  Slot number (0-15)
 * @param value 19-bit control word value
 */
static inline void sam_dram_write_slot_ctrl(uint8_t slot, uint32_t value)
{
    uint8_t addr = (slot << 4) | 0x0F;
    sam_dram_write(addr, value);
}

/*============================================================================
 * A-RAM Operations (Algorithm RAM)
 *============================================================================*/

/**
 * Write algorithm data to A-RAM slot
 *
 * Each algorithm slot is 64 bytes.
 *
 * @param slot  Algorithm slot number
 * @param data  Pointer to 64 bytes of algorithm data
 */
void sam_aram_write_slot(uint8_t slot, const uint8_t *data);

/*============================================================================
 * Higher-Level Operations
 *============================================================================*/

/**
 * Initialize SAM8905 chip
 *
 * Performs reset sequence and clears D-RAM.
 */
void sam_init(void);

/**
 * Initialize D-RAM slots with default ALG/MIX configuration
 *
 * Sets control word (offset 0x0F) of each slot with nibble-swapped slot ID.
 */
void sam_init_slots(void);
