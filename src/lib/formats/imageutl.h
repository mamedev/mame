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

#ifndef LOG_FORMATS
#define LOG_FORMATS(...) do { if (0) osd_printf_info(__VA_ARGS__); } while (false)
#endif

/* -----------------------------------------------------------------------
 * CRC stuff
 * ----------------------------------------------------------------------- */
unsigned short ccitt_crc16(unsigned short crc, const unsigned char *buffer, size_t buffer_len);
unsigned short ccitt_crc16_one( unsigned short crc, const unsigned char data );

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
