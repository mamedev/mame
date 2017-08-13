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

#include <memory>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PNG_Signature       "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
#define MNG_Signature       "\x8A\x4D\x4E\x47\x0D\x0A\x1A\x0A"

/* Chunk names */
#define PNG_CN_IHDR         0x49484452L
#define PNG_CN_PLTE         0x504C5445L
#define PNG_CN_IDAT         0x49444154L
#define PNG_CN_IEND         0x49454E44L
#define PNG_CN_gAMA         0x67414D41L
#define PNG_CN_sBIT         0x73424954L
#define PNG_CN_cHRM         0x6348524DL
#define PNG_CN_tRNS         0x74524E53L
#define PNG_CN_bKGD         0x624B4744L
#define PNG_CN_hIST         0x68495354L
#define PNG_CN_tEXt         0x74455874L
#define PNG_CN_zTXt         0x7A545874L
#define PNG_CN_pHYs         0x70485973L
#define PNG_CN_oFFs         0x6F464673L
#define PNG_CN_tIME         0x74494D45L
#define PNG_CN_sCAL         0x7343414CL

/* MNG Chunk names */
#define MNG_CN_MHDR         0x4D484452L
#define MNG_CN_MEND         0x4D454E44L
#define MNG_CN_TERM         0x5445524DL
#define MNG_CN_BACK         0x4241434BL

/* Prediction filters */
#define PNG_PF_None         0
#define PNG_PF_Sub          1
#define PNG_PF_Up           2
#define PNG_PF_Average      3
#define PNG_PF_Paeth        4

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

struct png_text
{
	png_text *      next;
	const char *    keyword;        /* this is allocated */
	const char *    text;           /* this points to a part of keyword */
};


class png_info
{
public:
	~png_info() { free_data(); }

	png_error read_file(util::core_file &fp);
	png_error copy_to_bitmap(bitmap_argb32 &bitmap, bool &hasalpha);
	png_error expand_buffer_8bit();

	png_error add_text(char const *keyword, char const *text);

	void free_data();
	void reset() { free_data(); operator=(png_info()); }

	std::unique_ptr<std::uint8_t []>	image;
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

	png_text *                          textlist = nullptr;

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
