// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII PROM loading and decoding
 *
 *****************************************************************************/
#include "emu.h"
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
static uint32_t ones_u32(uint32_t val)
{
	val -= ((val >> 1) & 0x55555555);
	val = (((val >> 2) & 0x33333333) + (val & 0x33333333));
	val = (((val >> 4) + val) & 0x0f0f0f0f);
	val += (val >> 8);
	val += (val >> 16);
	return val & 0x3f;
}

/**
 * @brief return the log2 of an integer value
 */
static uint32_t log2_u32(uint32_t val)
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
 * @param map pointer to an array of values, or nullptr for default
 * @param lines number of data or address lines
 * @param val value to map
 * @result returns the remapped value, or just val, if map was nullptr
 */
static uint32_t map_lines(const uint8_t *map, int lines, uint32_t val)
{
	if (nullptr == map)
		return val;

	uint32_t res = 0;
	for (int i = 0; i < lines; i++)
		if (val & (1 << i))
			res |= 1 << map[i];
	return res;
}

/**
 * @brief write to a ROM base + address of type 'type', ANDing with and, ORing with or
 *
 * @param base ROM base address in memory
 * @param type one of 1 for uint8_t, 2 for uint16_t, 4 for uint32_t
 * @param addr address offset into base
 * @param dand value to AND to contents before XORing
 * @param dxor value to XOR before writing back
 */
static void write_type_and_xor(void *base, int type, uint32_t addr, uint32_t dand, uint32_t dxor)
{
	switch (type) {
	case sizeof(uint8_t):
		{
			uint8_t *base8 = reinterpret_cast<uint8_t *>(base);
			base8[addr] = (base8[addr] & dand) ^ dxor;
		}
		break;
	case sizeof(uint16_t):
		{
			uint16_t *base16 = reinterpret_cast<uint16_t *>(base);
			base16[addr] = (base16[addr] & dand) ^ dxor;
		}
		break;
	case sizeof(uint32_t):
		{
			uint32_t *base32 = reinterpret_cast<uint32_t *>(base);
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
template <typename T>
std::unique_ptr<T []> prom_load(running_machine& machine, const prom_load_t* prom, const uint8_t* src, int pages, int segments)
{
	assert(sizeof(T) == prom->type);

	size_t const size = prom->size;
#if DEBUG_PROM_LOAD
	size_t const type = prom->type;
	uint8_t const width = prom->width;
#endif

	std::unique_ptr<T []> array = std::make_unique<T []>(pages * size);

	uint8_t* base = reinterpret_cast<uint8_t *>(array.get());
	for (int page = 0; page < pages; page++)
	{
		uint8_t* dst = base + (prom->type * prom->size * page);
		for (int segment = 0; segment < segments; segment++, prom++)
		{
			for (uint32_t src_addr = 0; src_addr < prom->size; src_addr++)
			{
				// map destination address lines
				uint32_t dst_addr = map_lines(prom->amap, log2_u32(prom->size) + 1, src_addr);
				// fetch data bits
				uint32_t data = src[src_addr ^ prom->axor] ^ prom->dxor;
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
	for (int addr = 0; addr < pages*size; addr++) {
		switch (type) {
		case sizeof(uint8_t):
			if (0 == (addr % 16))
				printf("%04x:", addr);
			if (width <= 4)
				printf(" %x", array[addr]);
			else
				printf(" %02x", array[addr]);
			if (15 == (addr % 16))
				printf("\n");
			break;
		case sizeof(uint16_t):
			if (0 == (addr % 8))
				printf("%04x:", addr);
			printf(" %04x", array[addr]);
			if (7 == (addr % 8))
				printf("\n");
			break;
		case sizeof(uint32_t):
			if (0 == (addr % 4))
				printf("%04x:", addr);
			printf(" %08x", array[addr]);
			if (3 == (addr % 4))
				printf("\n");
			break;
		}
	}
#endif
	return array;
}

template std::unique_ptr<uint8_t []> prom_load<uint8_t>(running_machine& machine, const prom_load_t* prom, const uint8_t* src, int pages, int segments);
template std::unique_ptr<uint16_t []> prom_load<uint16_t>(running_machine& machine, const prom_load_t* prom, const uint8_t* src, int pages, int segments);
template std::unique_ptr<uint32_t []> prom_load<uint32_t>(running_machine& machine, const prom_load_t* prom, const uint8_t* src, int pages, int segments);
