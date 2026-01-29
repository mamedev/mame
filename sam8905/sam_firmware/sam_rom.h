#pragma once

/**
 * SAM8905 Controller Firmware - ROM Access
 *
 * Macros and functions for accessing program ROM data.
 * Handles both CODE space (MOVC) and DATA space (MOVX) access patterns.
 */

#include <stdint.h>
#include "sam_types.h"
#include "sam_config.h"

/*============================================================================
 * ROM Data (provided by platform)
 *============================================================================*/

/**
 * ROM data array
 *
 * For 8051 targets, this is in CODE space accessed via MOVC.
 * For emulation/testing, this is loaded from the ROM file.
 */
extern CODE uint8_t g_rom[];

/*============================================================================
 * Basic ROM Access Macros
 *============================================================================*/

/**
 * Read single byte from ROM
 */
#define ROM_BYTE(addr) (g_rom[(addr)])

/**
 * Read 16-bit little-endian word from ROM
 */
#define ROM_WORD_LE(addr) \
    ((uint16_t)g_rom[(addr)] | ((uint16_t)g_rom[(addr)+1] << 8))

/**
 * Read 16-bit big-endian word from ROM
 */
#define ROM_WORD_BE(addr) \
    (((uint16_t)g_rom[(addr)] << 8) | (uint16_t)g_rom[(addr)+1])

/*============================================================================
 * Program Table Access
 *============================================================================*/

/**
 * Get program data pointer from program table
 *
 * Program table contains big-endian 16-bit pointers.
 *
 * @param program_num  Program number (0 to SAM_NUM_PROGRAMS-1)
 * @return             ROM address of program data
 */
static inline uint16_t rom_get_program_ptr(uint8_t program_num)
{
    uint16_t table_addr = SAM_PROGRAM_TABLE + (uint16_t)program_num * 2;
    return ROM_WORD_BE(table_addr);
}

/**
 * Read byte from program data stream
 *
 * Uses intmem rom_data_ptr_lo/hi as current position, auto-increments.
 *
 * @param im  Pointer to intmem structure
 * @return    Byte at current position
 */
static inline uint8_t rom_read_stream_byte(intmem_t *im)
{
    uint16_t addr = ((uint16_t)im->rom_data_ptr_hi << 8) | im->rom_data_ptr_lo;
    uint8_t value = ROM_BYTE(addr);

    /* Increment pointer */
    addr++;
    im->rom_data_ptr_lo = (uint8_t)addr;
    im->rom_data_ptr_hi = (uint8_t)(addr >> 8);

    return value;
}

/**
 * Read 16-bit BE word from program data stream
 *
 * @param im  Pointer to intmem structure
 * @return    16-bit big-endian word
 */
static inline uint16_t rom_read_stream_word_be(intmem_t *im)
{
    uint8_t hi = rom_read_stream_byte(im);
    uint8_t lo = rom_read_stream_byte(im);
    return ((uint16_t)hi << 8) | lo;
}

/**
 * Read 16-bit LE word from program data stream
 *
 * @param im  Pointer to intmem structure
 * @return    16-bit little-endian word
 */
static inline uint16_t rom_read_stream_word_le(intmem_t *im)
{
    uint8_t lo = rom_read_stream_byte(im);
    uint8_t hi = rom_read_stream_byte(im);
    return ((uint16_t)hi << 8) | lo;
}

/**
 * Set ROM stream position
 *
 * @param im    Pointer to intmem structure
 * @param addr  ROM address to set
 */
static inline void rom_set_stream_pos(intmem_t *im, uint16_t addr)
{
    im->rom_data_ptr_lo = (uint8_t)addr;
    im->rom_data_ptr_hi = (uint8_t)(addr >> 8);
}

/**
 * Get current ROM stream position
 *
 * @param im  Pointer to intmem structure
 * @return    Current ROM address
 */
static inline uint16_t rom_get_stream_pos(intmem_t *im)
{
    return ((uint16_t)im->rom_data_ptr_hi << 8) | im->rom_data_ptr_lo;
}

/*============================================================================
 * Table Lookups
 *============================================================================*/

#ifdef SAM_SINE_TABLE
/**
 * Read from sine LFO table
 *
 * @param index  Table index (0-63)
 * @return       Sine value (signed)
 */
static inline int8_t rom_sine_lookup(uint8_t index)
{
    return (int8_t)ROM_BYTE(SAM_SINE_TABLE + (index & 0x3F));
}
#endif

#ifdef SAM_VELOCITY_CURVES
/**
 * Read from velocity curve table
 *
 * @param curve    Curve number
 * @param velocity Input velocity (0-127)
 * @return         Scaled velocity
 */
static inline uint8_t rom_velocity_lookup(uint8_t curve, uint8_t velocity)
{
    /* Each curve is 128 bytes */
    uint16_t addr = SAM_VELOCITY_CURVES + (uint16_t)curve * 128 + velocity;
    return ROM_BYTE(addr);
}
#endif
