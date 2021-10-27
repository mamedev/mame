// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Victor Laskin
/***************************************************************************

    ui/icorender.h

    Windows icon file parser.

    Previously based on code by Victor Laskin (victor.laskin@gmail.com)
    http://vitiy.info/Code/ico.cpp

    TODO:
    * Add variant that loads all images from the file
    * Allow size hint for choosing best candidate
    * Allow selecting amongst candidates based on colour depth

***************************************************************************/

#include "emu.h"
#include "icorender.h"

#include "util/msdib.h"
#include "util/png.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

// need to set LOG_OUTPUT_FUNC or LOG_OUTPUT_STREAM because there's no logerror outside devices
#define LOG_OUTPUT_FUNC osd_printf_verbose

#define LOG_GENERAL (1U << 0)

//#define VERBOSE (LOG_GENERAL | LOG_DIB)

#include "logmacro.h"


namespace ui {

namespace {

// ICO file header
struct icon_dir_t
{
	uint16_t    reserved;   // must be 0
	uint16_t    type;       // 1 for icon or 2 for cursor
	uint16_t    count;      // number of images in the file
};

// ICO file directory entry
struct icon_dir_entry_t
{
	constexpr unsigned get_width() const { return width ? width : 256U; }
	constexpr unsigned get_height() const { return height ? height : 256U; }

	void byteswap()
	{
		planes = little_endianize_int16(planes);
		bpp = little_endianize_int16(bpp);
		size = little_endianize_int32(size);
		offset = little_endianize_int32(offset);
	}

	uint8_t     width;      // 0 means 256
	uint8_t     height;     // 0 means 256
	uint8_t     colors;     // for indexed colour, or 0 for direct colour
	uint8_t     reserved;   // documentation says this should be 0 but .NET writes 255
	uint16_t    planes;     // or hotspot X for cursor
	uint16_t    bpp;        // 0 to infer from image data, or hotspot Y for cursor
	uint32_t    size;       // image data size in bytes
	uint32_t    offset;     // offset to image data from start of file
};


bool load_ico_png(util::core_file &fp, icon_dir_entry_t const &dir, bitmap_argb32 &bitmap)
{
	// skip out if the data isn't a reasonable size - PNG magic alone is eight bytes
	if (9U >= dir.size)
		return false;
	fp.seek(dir.offset, SEEK_SET);
	std::error_condition const err(util::png_read_bitmap(fp, bitmap));
	if (!err)
	{
		// found valid PNG image
		assert(bitmap.valid());
		if ((dir.get_width() == bitmap.width()) && ((dir.get_height() == bitmap.height())))
		{
			LOG("Loaded %d*%d pixel PNG image from ICO file\n", bitmap.width(), bitmap.height());
		}
		else
		{
			LOG(
					"Loaded %d*%d pixel PNG image from ICO file (directory indicated %u*%u)\n",
					bitmap.width(),
					bitmap.height(),
					dir.get_width(),
					dir.get_height());
		}
		return true;
	}
	else if (util::png_error::BAD_SIGNATURE == err)
	{
		// doesn't look like PNG data - just fall back to DIB without the file header
		return false;
	}
	else
	{
		// invalid PNG data or I/O error
		LOG(
				"Error %s:%d %s reading PNG image data from ICO file at offset %u (directory size %u)\n",
				err.category().name(),
				err.value(),
				err.message(),
				dir.offset,
				dir.size);
		return false;
	}
}


bool load_ico_dib(util::core_file &fp, icon_dir_entry_t const &dir, bitmap_argb32 &bitmap)
{
	fp.seek(dir.offset, SEEK_SET);
	util::msdib_error const err(util::msdib_read_bitmap_data(fp, bitmap, dir.size, dir.get_height()));
	switch (err)
	{
	case util::msdib_error::NONE:
		// found valid DIB image
		assert(bitmap.valid());
		if ((dir.get_width() == bitmap.width()) && ((dir.get_height() == bitmap.height())))
		{
			LOG("Loaded %d*%d pixel DIB image from ICO file\n", bitmap.width(), bitmap.height());
		}
		else
		{
			LOG(
					"Loaded %d*%d pixel DIB image from ICO file (directory indicated %u*%u)\n",
					bitmap.width(),
					bitmap.height(),
					dir.get_width(),
					dir.get_height());
		}
		return true;

	default:
		// invalid DIB data or I/O error
		LOG(
				"Error %u reading DIB image data from ICO file at offset %u (directory size %u)\n",
				unsigned(err),
				dir.offset,
				dir.size);
		return false;
	}
}


bool load_ico_image(util::core_file &fp, unsigned index, icon_dir_entry_t const &dir, bitmap_argb32 &bitmap)
{
	// try loading PNG image data (contains PNG file magic if used), and then fall back
	if (load_ico_png(fp, dir, bitmap))
	{
		LOG("Successfully loaded PNG image from ICO file entry %u\n", index);
		return true;
	}
	else if (load_ico_dib(fp, dir, bitmap))
	{
		LOG("Successfully loaded DIB image from ICO file entry %u\n", index);
		return true;
	}

	// no luck
	return false;
}


bool load_ico_image(util::core_file &fp, unsigned count, unsigned index, bitmap_argb32 &bitmap)
{
	// read the directory entry
	std::error_condition err;
	size_t actual;
	icon_dir_entry_t dir;
	err = fp.seek(sizeof(icon_dir_t) + (sizeof(icon_dir_entry_t) * index), SEEK_SET);
	if (!err)
		err = fp.read(&dir, sizeof(dir), actual);
	if (err || (sizeof(dir) != actual))
	{
		LOG("Failed to read ICO file directory entry %u\n", index);
		return false;
	}
	dir.byteswap();
	if ((sizeof(icon_dir_t) + (sizeof(icon_dir_entry_t) * count)) > dir.offset)
	{
		LOG(
				"ICO file image %u data starting at %u overlaps %u bytes of file header and directory\n",
				index,
				dir.offset,
				sizeof(icon_dir_t) + (sizeof(icon_dir_entry_t) * count));
		return false;
	}
	else
	{
		return load_ico_image(fp, index, dir, bitmap);
	}
}

} // anonymous namespace


int images_in_ico(util::core_file &fp)
{
	// read and check the icon file header
	std::error_condition err;
	size_t actual;
	icon_dir_t header;
	err = fp.seek(0, SEEK_SET);
	if (!err)
		err = fp.read(&header, sizeof(header), actual);
	if (err || (sizeof(header) != actual))
	{
		LOG("Failed to read ICO file header\n");
		return -1;
	}
	header.reserved = little_endianize_int16(header.reserved);
	header.type = little_endianize_int16(header.type);
	header.count = little_endianize_int16(header.count);
	if (0U != header.reserved)
	{
		LOG("Invalid ICO file header reserved field %u (expected 0)\n", header.reserved);
		return -1;
	}
	if ((1U != header.type) && (2U != header.type))
	{
		LOG("Invalid ICO file header type field %u (expected 1 or 2)\n", header.type);
		return -1;
	}
	return int(unsigned(little_endianize_int16(header.count)));
}


void render_load_ico(util::core_file &fp, unsigned index, bitmap_argb32 &bitmap)
{
	// check that these things haven't been padded somehow
	static_assert(sizeof(icon_dir_t) == 6U, "compiler has applied padding to icon_dir_t");
	static_assert(sizeof(icon_dir_entry_t) == 16U, "compiler has applied padding to icon_dir_entry_t");

	// read and check the icon file header, then try to load the specified image
	int const count(images_in_ico(fp));
	if (0 > count)
	{
		// images_in_ico already logged an error
	}
	else if (index >= count)
	{
		osd_printf_verbose("Requested image %u from ICO file containing %d images\n", index, count);
	}
	else if (load_ico_image(fp, count, index, bitmap))
	{
		return;
	}
	bitmap.reset();
}


void render_load_ico_first(util::core_file &fp, bitmap_argb32 &bitmap)
{
	int const count(images_in_ico(fp));
	for (int i = 0; count > i; ++i)
	{
		if (load_ico_image(fp, count, i, bitmap))
			return;
	}
	bitmap.reset();
}


void render_load_ico_highest_detail(util::core_file &fp, bitmap_argb32 &bitmap)
{
	// read and check the icon file header - logs a message on error
	int const count(images_in_ico(fp));
	if (0 <= count)
	{
		// now load all the directory entries
		size_t const dir_bytes(sizeof(icon_dir_entry_t) * count);
		std::unique_ptr<icon_dir_entry_t []> dir(new (std::nothrow) icon_dir_entry_t [count]);
		std::unique_ptr<unsigned []> index(new (std::nothrow) unsigned [count]);
		size_t actual;
		if (count && (!dir || !index || fp.read(dir.get(), dir_bytes, actual) || (dir_bytes != actual)))
		{
			LOG("Failed to read ICO file directory entries\n");
		}
		else
		{
			// byteswap and sort by (pixels, depth)
			for (int i = 0; count > i; ++i)
			{
				dir[i].byteswap();
				index[i] = i;
			}
			std::stable_sort(
					index.get(),
					index.get() + count,
					[&dir] (unsigned x, unsigned y)
					{
						unsigned const x_pixels(dir[x].get_width() * dir[x].get_height());
						unsigned const y_pixels(dir[y].get_width() * dir[y].get_height());
						if (x_pixels > y_pixels)
							return true;
						else if (x_pixels < y_pixels)
							return false;
						else
							return dir[x].bpp > dir[y].bpp;
					});

			// walk down until something works
			for (int i = 0; count > i; ++i)
			{
				LOG(
						"Try loading ICO file entry %u: %u*%u, %u bits per pixel\n",
						index[i],
						dir[index[i]].get_width(),
						dir[index[i]].get_height(),
						dir[index[i]].bpp);
				if (load_ico_image(fp, index[i], dir[index[i]], bitmap))
					return;
			}
		}
	}
	bitmap.reset();
}

} // namespace ui
