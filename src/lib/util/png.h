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
#include "utilfwd.h"

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>


namespace util {

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Error types */
enum class png_error : int
{
	UNKNOWN_FILTER = 1,
	BAD_SIGNATURE,
	DECOMPRESS_ERROR,
	FILE_TRUNCATED,
	FILE_CORRUPT,
	UNKNOWN_CHUNK,
	COMPRESS_ERROR,
	UNSUPPORTED_FORMAT
};

std::error_category const &png_category() noexcept;
inline std::error_condition make_error_condition(png_error err) noexcept { return std::error_condition(int(err), png_category()); }



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class png_info
{
public:
	using png_text = std::pair<std::string, std::string>;

	~png_info() { free_data(); }

	std::error_condition read_file(read_stream &fp);
	std::error_condition copy_to_bitmap(bitmap_argb32 &bitmap, bool &hasalpha);
	std::error_condition expand_buffer_8bit();

	std::error_condition add_text(std::string_view keyword, std::string_view text);

	void free_data();
	void reset() { free_data(); operator=(png_info()); }

	static std::error_condition verify_header(read_stream &fp);

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

std::error_condition png_read_bitmap(read_stream &fp, bitmap_argb32 &bitmap);

std::error_condition png_write_bitmap(random_write &fp, png_info *info, bitmap_t const &bitmap, int palette_length, const rgb_t *palette);

std::error_condition mng_capture_start(random_write &fp, bitmap_t const &bitmap, unsigned rate);
std::error_condition mng_capture_frame(random_write &fp, png_info &info, bitmap_t const &bitmap, int palette_length, rgb_t const *palette);
std::error_condition mng_capture_stop(random_write &fp);

} // namespace util


namespace std {

template <> struct is_error_condition_enum<util::png_error> : public std::true_type { };

} // namespace std

#endif // MAME_LIB_UTIL_PNG_H
