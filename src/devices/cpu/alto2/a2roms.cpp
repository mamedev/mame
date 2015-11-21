// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII PROM loading and decoding
 *
 *****************************************************************************/
#include "alto2cpu.h"
#include "a2roms.h"

#define DEBUG_PROM_LOAD     0   //!< define to 1 to dump PROMs after loading

/**
 * @brief return number of 1 bits in a 32 bit value
 *
 * 32-bit recursive reduction using SWAR,
 * but first step is mapping 2-bit values
 * into sum of 2 1-bit values in sneaky way.
 */
static UINT32 ones_u32(UINT32 val)
{
		val -= ((val >> 1) & 0x55555555);
		val = (((val >> 2) & 0x33333333) + (val & 0x33333333));
		val = (((val >> 4) + val) & 0x0f0f0f0f);
		val += (val >> 8);
		val += (val >> 16);
		return (val & 0x0000003f);
}

/**
 * @brief return the log2 of an integer value
 */
static UINT32 log2_u32(UINT32 val)
{
	val |= (val >> 1);
	val |= (val >> 2);
	val |= (val >> 4);
	val |= (val >> 8);
	val |= (val >> 16);
	return ones_u32(val >> 1);
}

/**
 * @brief map a number of data or address lines using a lookup table
 *
 * @param map pointer to an array of values, or NULL for default
 * @param lines number of data or address lines
 * @param val value to map
 * @result returns the remapped value, or just val, if map was NULL
 */
static UINT32 map_lines(const UINT8 *map, int lines, UINT32 val)
{
	if (NULL == map)
		return val;

	UINT32 res = 0;
	for (int i = 0; i < lines; i++)
		if (val & (1 << i))
			res |= 1 << map[i];
	return res;
}

/**
 * @brief write to a ROM base + address of type 'type', ANDing with and, ORing with or
 *
 * @param base ROM base address in memory
 * @param type one of 1 for UINT8, 2 for UINT16, 4 for UINT32
 * @param addr address offset into base
 * @param dand value to AND to contents before XORing
 * @param dxor value to XOR before writing back
 */
static void write_type_and_xor(void *base, int type, UINT32 addr, UINT32 dand, UINT32 dxor)
{
	switch (type) {
	case sizeof(UINT8):
		{
			UINT8 *base8 = reinterpret_cast<UINT8 *>(base);
			base8[addr] = (base8[addr] & dand) ^ dxor;
		}
		break;
	case sizeof(UINT16):
		{
			UINT16 *base16 = reinterpret_cast<UINT16 *>(base);
			base16[addr] = (base16[addr] & dand) ^ dxor;
		}
		break;
	case sizeof(UINT32):
		{
			UINT32 *base32 = reinterpret_cast<UINT32 *>(base);
			base32[addr] = (base32[addr] & dand) ^ dxor;
		}
		break;
	default:
		fatalerror("write_type_and_xor() invalid type size (%d) in ROM definitions\n", type);
	}
}

/**
 * @brief load a PROM from a (list of) source region(s) shifting, swapping and inverting address and data bits
 * @param prom PROM loading definition
 * @param src source ROM region where to load data from
 * @param pages number of pages of definitions
 * @param segments number of segments in one page of the result
 * @return pointer to the newly allocated memory filled with source bits
 */
UINT8* prom_load(running_machine& machine, const prom_load_t* prom, const UINT8* src, int pages, int segments)
{
	void* array = 0;
	size_t type = prom->type;
	size_t size = prom->size;
#if DEBUG_PROM_LOAD
	UINT8 width = prom->width;
#endif

	switch (type) {
	case sizeof(UINT8):
		array = auto_alloc_array(machine, UINT8, pages * size);
		break;
	case sizeof(UINT16):
		array = auto_alloc_array(machine, UINT16, pages * size);
		break;
	case sizeof(UINT32):
		array = auto_alloc_array(machine, UINT32, pages * size);
		break;
	}

	UINT8* base = reinterpret_cast<UINT8*>(array);
	for (int page = 0; page < pages; page++)
	{
		UINT8* dst = base + (prom->type * prom->size * page);
		for (int segment = 0; segment < segments; segment++, prom++)
		{
			for (UINT32 src_addr = 0; src_addr < prom->size; src_addr++)
			{
				// map destination address lines
				UINT32 dst_addr = map_lines(prom->amap, log2_u32(prom->size) + 1, src_addr);
				// fetch data bits
				UINT32 data = src[src_addr ^ prom->axor] ^ prom->dxor;
				// mask width bits
				data = data & ((1 << prom->width) - 1);
				// map destination data lines
				data = map_lines(prom->dmap, prom->width, data);
				// shift to destination position
				data = data << prom->shift;
				// and destination width dand then xor data
				write_type_and_xor(dst, prom->type, dst_addr, prom->dand, data);
			}
			src += prom->size;
		}
	}

#if DEBUG_PROM_LOAD
	switch (type) {
	case sizeof(UINT8):
		{
			UINT8* data = reinterpret_cast<UINT8*>(array);
			for (int addr = 0; addr < pages*size; addr++) {
				if (0 == (addr % 16))
					printf("%04x:", addr);
				if (width <= 4)
					printf(" %x", data[addr]);
				else
					printf(" %02x", data[addr]);
				if (15 == (addr % 16))
					printf("\n");
			}
		}
		break;
	case sizeof(UINT16):
		{
			UINT16* data = reinterpret_cast<UINT16*>(array);
			for (int addr = 0; addr < pages*size; addr++) {
				if (0 == (addr % 8))
					printf("%04x:", addr);
				printf(" %04x", data[addr]);
				if (7 == (addr % 8))
					printf("\n");
			}
		}
		break;
	case sizeof(UINT32):
		{
			UINT32* data = reinterpret_cast<UINT32*>(array);
			for (int addr = 0; addr < pages*size; addr++) {
				if (0 == (addr % 4))
					printf("%04x:", addr);
				printf(" %08x", data[addr]);
				if (3 == (addr % 4))
					printf("\n");
			}
		}
		break;
	}
#endif
	return reinterpret_cast<UINT8 *>(array);
}
