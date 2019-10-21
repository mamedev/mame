// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    imageutl.h

    Image related utilities

***************************************************************************/
#ifndef MAME_FORMATS_IMAGEUTL_H
#define MAME_FORMATS_IMAGEUTL_H

#pragma once

#include "osdcore.h"

/* -----------------------------------------------------------------------
 * CRC stuff
 * ----------------------------------------------------------------------- */
unsigned short ccitt_crc16(unsigned short crc, const unsigned char *buffer, size_t buffer_len);
unsigned short ccitt_crc16_one( unsigned short crc, const unsigned char data );

/* -----------------------------------------------------------------------
 * Alignment-friendly integer placement
 * ----------------------------------------------------------------------- */

inline void place_integer_be(void *ptr, size_t offset, size_t size, uint64_t value)
{
	uint8_t *byte_ptr = ((uint8_t *) ptr) + offset;
	uint16_t val16;
	uint32_t val32;

	switch (size)
	{
	case 2:
		val16 = big_endianize_int16((uint16_t) value);
		memcpy(byte_ptr, &val16, sizeof(val16));
		break;

	case 4:
		val32 = big_endianize_int32((uint32_t) value);
		memcpy(byte_ptr, &val32, sizeof(val32));
		break;

	default:
		if (size >= 1)  byte_ptr[0] = (uint8_t) (value >> ((size - 1) * 8));
		if (size >= 2)  byte_ptr[1] = (uint8_t) (value >> ((size - 2) * 8));
		if (size >= 3)  byte_ptr[2] = (uint8_t) (value >> ((size - 3) * 8));
		if (size >= 4)  byte_ptr[3] = (uint8_t) (value >> ((size - 4) * 8));
		if (size >= 5)  byte_ptr[4] = (uint8_t) (value >> ((size - 5) * 8));
		if (size >= 6)  byte_ptr[5] = (uint8_t) (value >> ((size - 6) * 8));
		if (size >= 7)  byte_ptr[6] = (uint8_t) (value >> ((size - 7) * 8));
		if (size >= 8)  byte_ptr[7] = (uint8_t) (value >> ((size - 8) * 8));
		break;
	}
}

inline uint64_t pick_integer_be(const void *ptr, size_t offset, size_t size)
{
	uint64_t result = 0;
	const uint8_t *byte_ptr = ((const uint8_t *) ptr) + offset;
	uint16_t val16;
	uint32_t val32;

	switch (size)
	{
	case 1:
		result = *byte_ptr;
		break;

	case 2:
		memcpy(&val16, byte_ptr, sizeof(val16));
		result = big_endianize_int16(val16);
		break;

	case 4:
		memcpy(&val32, byte_ptr, sizeof(val32));
		result = big_endianize_int32(val32);
		break;

	default:
		if (size >= 1)  result |= ((uint64_t) byte_ptr[0]) << ((size - 1) * 8);
		if (size >= 2)  result |= ((uint64_t) byte_ptr[1]) << ((size - 2) * 8);
		if (size >= 3)  result |= ((uint64_t) byte_ptr[2]) << ((size - 3) * 8);
		if (size >= 4)  result |= ((uint64_t) byte_ptr[3]) << ((size - 4) * 8);
		if (size >= 5)  result |= ((uint64_t) byte_ptr[4]) << ((size - 5) * 8);
		if (size >= 6)  result |= ((uint64_t) byte_ptr[5]) << ((size - 6) * 8);
		if (size >= 7)  result |= ((uint64_t) byte_ptr[6]) << ((size - 7) * 8);
		if (size >= 8)  result |= ((uint64_t) byte_ptr[7]) << ((size - 8) * 8);
		break;
	}
	return result;
}

inline void place_integer_le(void *ptr, size_t offset, size_t size, uint64_t value)
{
	uint8_t *byte_ptr = ((uint8_t *) ptr) + offset;
	uint16_t val16;
	uint32_t val32;

	switch (size)
	{
	case 2:
		val16 = little_endianize_int16((uint16_t) value);
		memcpy(byte_ptr, &val16, sizeof(val16));
		break;

	case 4:
		val32 = little_endianize_int32((uint32_t) value);
		memcpy(byte_ptr, &val32, sizeof(val32));
		break;

	default:
		if (size >= 1)  byte_ptr[0] = (uint8_t) (value >> (0 * 8));
		if (size >= 2)  byte_ptr[1] = (uint8_t) (value >> (1 * 8));
		if (size >= 3)  byte_ptr[2] = (uint8_t) (value >> (2 * 8));
		if (size >= 4)  byte_ptr[3] = (uint8_t) (value >> (3 * 8));
		if (size >= 5)  byte_ptr[4] = (uint8_t) (value >> (4 * 8));
		if (size >= 6)  byte_ptr[5] = (uint8_t) (value >> (5 * 8));
		if (size >= 7)  byte_ptr[6] = (uint8_t) (value >> (6 * 8));
		if (size >= 8)  byte_ptr[7] = (uint8_t) (value >> (7 * 8));
		break;
	}
}

static inline uint64_t pick_integer_le(const void *ptr, size_t offset, size_t size)
{
	uint64_t result = 0;
	const uint8_t *byte_ptr = ((const uint8_t *) ptr) + offset;
	uint16_t val16;
	uint32_t val32;

	switch (size)
	{
	case 1:
		result = *byte_ptr;
		break;

	case 2:
		memcpy(&val16, byte_ptr, sizeof(val16));
		result = little_endianize_int16(val16);
		break;

	case 4:
		memcpy(&val32, byte_ptr, sizeof(val32));
		result = little_endianize_int32(val32);
		break;

	default:
		if (size >= 1)  result |= ((uint64_t) byte_ptr[0]) << (0 * 8);
		if (size >= 2)  result |= ((uint64_t) byte_ptr[1]) << (1 * 8);
		if (size >= 3)  result |= ((uint64_t) byte_ptr[2]) << (2 * 8);
		if (size >= 4)  result |= ((uint64_t) byte_ptr[3]) << (3 * 8);
		if (size >= 5)  result |= ((uint64_t) byte_ptr[4]) << (4 * 8);
		if (size >= 6)  result |= ((uint64_t) byte_ptr[5]) << (5 * 8);
		if (size >= 7)  result |= ((uint64_t) byte_ptr[6]) << (6 * 8);
		if (size >= 8)  result |= ((uint64_t) byte_ptr[7]) << (7 * 8);
		break;
	}
	return result;
}

/* -----------------------------------------------------------------------
 * Miscellaneous
 * ----------------------------------------------------------------------- */

/* miscellaneous functions */
int compute_log2(int val);

/* -----------------------------------------------------------------------
 * Extension list handling
 * ----------------------------------------------------------------------- */

bool image_find_extension(const char *extensions, const char *ext);
void image_specify_extension(char *buffer, size_t buffer_len, const char *extension);

#endif // MAME_FORMATS_IMAGEUTL_H
