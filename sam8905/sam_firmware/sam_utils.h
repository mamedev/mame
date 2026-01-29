#pragma once

/**
 * SAM8905 Controller Firmware - Utility Functions
 *
 * 8051-derived utilities for table search, pointer loading, etc.
 * In C, pointer arithmetic is native, but we preserve the firmware's
 * data structures and calling patterns for structural fidelity.
 */

#include <stdint.h>
#include "sam_types.h"

/*============================================================================
 * Table Search (CODE:DC1C)
 *
 * Linear search through CODE space table for matching byte.
 *
 * Original uses INTMEM:78-7C for parameters:
 *   search_ptr_hi (0x78), search_ptr_lo (0x79) = table address
 *   search_value (0x7A) = byte to find
 *   search_count_hi (0x7B), search_count_lo (0x7C) = table length
 *
 * Original disassembly:
 *   DC1C: MOV DPH,0x78
 *   DC1F: MOV DPL,0x79
 *   DC22: CLR A
 *   DC23: MOVC A,@A+DPTR
 *   DC24: CJNE A,0x7A,DC2D     ; compare with search value
 *   DC27: ... (found: compute index)
 *   DC2D: INC DPTR
 *   DC2E: ... (loop decrement, check count)
 *============================================================================*/

/**
 * Search ROM table for matching byte
 *
 * @param table      Pointer to table in ROM/CODE space
 * @param table_len  Number of entries to search
 * @param value      Byte value to find
 * @return           Index (0-based) if found, 0xFF if not found
 */
uint8_t table_search_match(const uint8_t *table, uint16_t table_len, uint8_t value);

/**
 * Search ROM table for non-matching byte (inverse of above)
 *
 * Used by some firmware routines to find first entry that doesn't match.
 *
 * @param table      Pointer to table in ROM/CODE space
 * @param table_len  Number of entries to search
 * @param value      Byte value to NOT match
 * @return           Index (0-based) of first non-match, 0xFF if all match
 */
uint8_t table_search_nomatch(const uint8_t *table, uint16_t table_len, uint8_t value);

/*============================================================================
 * Pointer Loading (CODE:DC9C)
 *
 * Load 16-bit pointer from XRAM (little-endian).
 *
 * Original:
 *   DC9C: MOVX A,@DPTR         ; read low byte
 *   DC9D: MOV R7,A
 *   DC9E: INC DPTR
 *   DC9F: MOVX A,@DPTR         ; read high byte
 *   DCA0: MOV R6,A
 *   ... (return R6:R7 as 16-bit value)
 *============================================================================*/

/**
 * Load 16-bit little-endian pointer from XRAM
 *
 * @param addr  XRAM address containing pointer (little-endian)
 * @return      16-bit value (lo at addr, hi at addr+1)
 */
uint16_t load_ptr_from_xram(uint16_t addr);

/**
 * Load 16-bit little-endian pointer from extmem by offset
 *
 * @param offset  Offset into g_extmem
 * @return        16-bit value
 */
uint16_t load_ptr_from_extmem(uint16_t offset);

/*============================================================================
 * Block Copy Utilities
 *
 * The firmware uses INTMEM 0x2F-0x33 for block copy operations:
 *   copy_count (0x2F) = byte count
 *   copy_src_lo/hi (0x30-0x31) = source pointer
 *   copy_dst_lo/hi (0x32-0x33) = destination pointer
 *
 * In C, we can use memcpy, but these wrappers preserve the interface.
 *============================================================================*/

/**
 * Copy bytes from ROM to XRAM using firmware's copy variables
 *
 * Uses g_intmem.copy_src, g_intmem.copy_dst, g_intmem.copy_count
 */
void block_copy_rom_to_xram(void);

/**
 * Copy bytes from XRAM to XRAM using firmware's copy variables
 */
void block_copy_xram_to_xram(void);

/*============================================================================
 * Byte Manipulation
 *
 * Common 8051 byte manipulation patterns used throughout the firmware.
 *============================================================================*/

/**
 * Swap nibbles (8051 SWAP A instruction)
 *
 * @param val  Input byte
 * @return     High nibble ↔ low nibble swapped
 */
static inline uint8_t swap_nibbles(uint8_t val)
{
    return (val >> 4) | (val << 4);
}

/**
 * Rotate left through carry (8051 RLC A)
 *
 * @param val  Input byte
 * @param c    Carry bit input
 * @param cout Pointer to receive carry output (may be NULL)
 * @return     Rotated value
 */
static inline uint8_t rotate_left_carry(uint8_t val, uint8_t c, uint8_t *cout)
{
    uint8_t result = (val << 1) | (c & 1);
    if (cout) *cout = (val >> 7) & 1;
    return result;
}

/**
 * Rotate right through carry (8051 RRC A)
 *
 * @param val  Input byte
 * @param c    Carry bit input
 * @param cout Pointer to receive carry output (may be NULL)
 * @return     Rotated value
 */
static inline uint8_t rotate_right_carry(uint8_t val, uint8_t c, uint8_t *cout)
{
    uint8_t result = (val >> 1) | ((c & 1) << 7);
    if (cout) *cout = val & 1;
    return result;
}

/**
 * Clamp signed value to 8-bit unsigned range
 *
 * @param val  Signed 16-bit value
 * @return     Clamped to 0-255
 */
static inline uint8_t clamp_u8(int16_t val)
{
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (uint8_t)val;
}

/**
 * Clamp signed value to 7-bit unsigned range (0-127)
 *
 * @param val  Signed 16-bit value
 * @return     Clamped to 0-127
 */
static inline uint8_t clamp_u7(int16_t val)
{
    if (val < 0) return 0;
    if (val > 127) return 127;
    return (uint8_t)val;
}

