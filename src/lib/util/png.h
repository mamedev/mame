// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    png.h

    PNG file management.

***************************************************************************/

#ifndef MAME_LIB_UTIL_PNG_H
#define MAME_LIB_UTIL_PNG_H

#pragma once

#include "bitmap.h"
#include "corefile.h"
#include "osdcore.h"

#include <list>
#include <memory>
#include <string>
#include <utility>



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Error types */
enum png_error
{
	PNGERR_NONE,
	PNGERR_OUT_OF_MEMORY,
	PNGERR_UNKNOWN_FILTER,
	PNGERR_FILE_ERROR,
	PNGERR_BAD_SIGNATURE,
	PNGERR_DECOMPRESS_ERROR,
	PNGERR_FILE_TRUNCATED,
	PNGERR_FILE_CORRUPT,
	PNGERR_UNKNOWN_CHUNK,
	PNGERR_COMPRESS_ERROR,
	PNGERR_UNSUPPORTED_FORMAT
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class png_info
{
public:
	using png_text = std::pair<std::string, std::string>;

	~png_info() { free_data(); }

	png_error read_file(util::core_file &fp);
	png_error copy_to_bitmap(bitmap_argb32 &bitmap, bool &hasalpha);
	png_error expand_buffer_8bit();

	png_error add_text(char const *keyword, char const *text);

	void free_data();
	void reset() { free_data(); operator=(png_info()); }

	static png_error verify_header(util::core_file &fp);

	std::unique_ptr<std::uint8_t []>    image;
	std::uint32_t                       width, height;
	std::uint32_t                       xres = 0, yres = 0;
	rectangle                           screen;
	double                              xscale = 0, yscale = 0;
	double                              source_gamma = 0;
	std::uint32_t                       resolution_unit = 0;
	std::uint8_t                        bit_depth = 0;
	std::uint8_t                        color_type = 0;
	std::uint8_t                        compression_method = 0;
	std::uint8_t                        filter_method = 0;
	std::uint8_t                        interlace_method = 0;

	std::unique_ptr<std::uint8_t []>    palette = 0;
	std::uint32_t                       num_palette = 0;

	std::unique_ptr<std::uint8_t []>    trans = 0;
	std::uint32_t                       num_trans = 0;

	std::list<png_text>                 textlist;

private:
	png_info &operator=(png_info &&) = default;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

png_error png_read_bitmap(util::core_file &fp, bitmap_argb32 &bitmap);

png_error png_write_bitmap(util::core_file &fp, png_info *info, bitmap_t const &bitmap, int palette_length, const rgb_t *palette);

png_error mng_capture_start(util::core_file &fp, bitmap_t &bitmap, double rate);
png_error mng_capture_frame(util::core_file &fp, png_info &info, bitmap_t const &bitmap, int palette_length, const rgb_t *palette);
png_error mng_capture_stop(util::core_file &fp);

#endif // MAME_LIB_UTIL_PNG_H
