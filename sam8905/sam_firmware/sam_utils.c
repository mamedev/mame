/**
 * SAM8905 Controller Firmware - Utility Functions
 *
 * Implementation of table search, pointer loading, and block copy.
 */

#include "sam_utils.h"
#include "sam_firmware.h"

/*============================================================================
 * table_search_match (CODE:DC1C)
 *
 * Linear search for matching byte in ROM table.
 *
 * Original disassembly:
 *   DC1C: MOV DPH,0x78          ; load search pointer
 *   DC1F: MOV DPL,0x79
 *   DC22: CLR A
 *   DC23: MOVC A,@A+DPTR        ; read table entry
 *   DC24: CJNE A,0x7A,DC2D      ; compare with search value
 *   DC27: MOV A,0x79            ; found: compute index
 *   DC29: CLR C
 *   DC2A: SUBB A,0x7C           ; A = DPL - count_lo (index = ptr - start)
 *   DC2C: RET
 *   DC2D: INC DPTR              ; not found: advance
 *   DC2E: DJNZ 0x7C,DC22        ; decrement count_lo, loop
 *   DC31: DJNZ 0x7B,DC22        ; decrement count_hi, loop
 *   DC34: MOV A,#0xFF           ; not found: return 0xFF
 *   DC36: RET
 *============================================================================*/

uint8_t table_search_match(const uint8_t *table, uint16_t table_len, uint8_t value)
{
    uint16_t i;

    for (i = 0; i < table_len; i++) {
        if (table[i] == value) {
            /* Return 8-bit index (clamped) */
            return (i > 255) ? 255 : (uint8_t)i;
        }
    }

    return 0xFF;  /* Not found */
}

/*============================================================================
 * table_search_nomatch
 *
 * Same structure as above but finds first NON-matching entry.
 *============================================================================*/

uint8_t table_search_nomatch(const uint8_t *table, uint16_t table_len, uint8_t value)
{
    uint16_t i;

    for (i = 0; i < table_len; i++) {
        if (table[i] != value) {
            return (i > 255) ? 255 : (uint8_t)i;
        }
    }

    return 0xFF;  /* All matched */
}

/*============================================================================
 * load_ptr_from_xram (CODE:DC9C)
 *
 * Load 16-bit little-endian pointer from XRAM.
 *
 * Original:
 *   DC9C: MOVX A,@DPTR          ; read low byte
 *   DC9D: MOV R7,A
 *   DC9E: INC DPTR
 *   DC9F: MOVX A,@DPTR          ; read high byte
 *   DCA0: MOV R6,A
 *   DCA1: RET
 *============================================================================*/

uint16_t load_ptr_from_xram(uint16_t addr)
{
    uint8_t *xram = (uint8_t *)&g_extmem;
    uint8_t lo, hi;

    /* Bounds check */
    if (addr >= SAM_RAM_SIZE - 1) {
        return 0;
    }

    lo = xram[addr];
    hi = xram[addr + 1];

    return ((uint16_t)hi << 8) | lo;
}

uint16_t load_ptr_from_extmem(uint16_t offset)
{
    return load_ptr_from_xram(offset);
}

/*============================================================================
 * block_copy_rom_to_xram
 *
 * Uses firmware copy variables from g_intmem:
 *   copy_count (0x2F)
 *   copy_src_lo/hi (0x30-0x31) = ROM source
 *   copy_dst_lo/hi (0x32-0x33) = XRAM dest
 *
 * Original pattern (various locations):
 *   MOV DPTR, copy_src
 *   MOV R0, copy_dst_lo
 *   MOV P2, copy_dst_hi
 *   loop:
 *     CLR A
 *     MOVC A,@A+DPTR       ; read from ROM
 *     MOVX @R0,A           ; write to XRAM
 *     INC DPTR
 *     INC R0
 *     DJNZ copy_count, loop
 *============================================================================*/

void block_copy_rom_to_xram(void)
{
    uint16_t src_addr;
    uint16_t dst_addr;
    uint8_t count;
    uint8_t *xram = (uint8_t *)&g_extmem;

    /* Build addresses from intmem variables */
    src_addr = ((uint16_t)g_intmem.copy_src_hi << 8) | g_intmem.copy_src_lo;
    dst_addr = ((uint16_t)g_intmem.copy_dst_hi << 8) | g_intmem.copy_dst_lo;
    count = g_intmem.copy_count;

    /* Copy loop */
    while (count > 0) {
        if (dst_addr < SAM_RAM_SIZE) {
            xram[dst_addr] = g_rom[src_addr & 0xFFFF];
        }
        src_addr++;
        dst_addr++;
        count--;
    }

    /* Update intmem with final positions */
    g_intmem.copy_src_lo = (uint8_t)(src_addr & 0xFF);
    g_intmem.copy_src_hi = (uint8_t)(src_addr >> 8);
    g_intmem.copy_dst_lo = (uint8_t)(dst_addr & 0xFF);
    g_intmem.copy_dst_hi = (uint8_t)(dst_addr >> 8);
    g_intmem.copy_count = 0;
}

/*============================================================================
 * block_copy_xram_to_xram
 *
 * Same pattern but both source and destination are XRAM.
 *============================================================================*/

void block_copy_xram_to_xram(void)
{
    uint16_t src_addr;
    uint16_t dst_addr;
    uint8_t count;
    uint8_t *xram = (uint8_t *)&g_extmem;

    src_addr = ((uint16_t)g_intmem.copy_src_hi << 8) | g_intmem.copy_src_lo;
    dst_addr = ((uint16_t)g_intmem.copy_dst_hi << 8) | g_intmem.copy_dst_lo;
    count = g_intmem.copy_count;

    while (count > 0) {
        if (dst_addr < SAM_RAM_SIZE && src_addr < SAM_RAM_SIZE) {
            xram[dst_addr] = xram[src_addr];
        }
        src_addr++;
        dst_addr++;
        count--;
    }

    g_intmem.copy_src_lo = (uint8_t)(src_addr & 0xFF);
    g_intmem.copy_src_hi = (uint8_t)(src_addr >> 8);
    g_intmem.copy_dst_lo = (uint8_t)(dst_addr & 0xFF);
    g_intmem.copy_dst_hi = (uint8_t)(dst_addr >> 8);
    g_intmem.copy_count = 0;
}
