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

#include "util/png.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

// need to set LOG_OUTPUT_STREAM because there's no logerror outside devices
#define LOG_OUTPUT_STREAM std::cerr

#define LOG_GENERAL (1U << 0)
#define LOG_DIB     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_DIB)

#include "logmacro.h"


namespace ui {

namespace {

// DIB compression schemes
enum : uint32_t
{
	DIB_COMP_NONE       = 0,
	DIB_COMP_RLE8       = 1,
	DIB_COMP_RLE4       = 2,
	DIB_COMP_BITFIELDS  = 3
};


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

// old-style DIB header
struct bitmap_core_header_t
{
	uint32_t    size;       // size of the header (12, 16 or 64)
	int16_t     width;      // width of bitmap in pixels
	int16_t     height;     // height of the image in pixels
	uint16_t    planes;     // number of colour planes (must be 1)
	uint16_t    bpp;        // bits per pixel
};

// new-style DIB header
struct bitmap_info_header_t
{
	uint32_t    size;       // size of the header
	int32_t     width;      // width of bitmap in pixels
	int32_t     height;     // height of bitmap in pixels
	uint16_t    planes;     // number of colour planes (must be 1)
	uint16_t    bpp;        // bits per pixel
	uint32_t    comp;       // compression method
	uint32_t    rawsize;    // size of bitmap data after decompression or 0 if uncompressed
	int32_t     hres;       // horizontal resolution in pixels/metre
	int32_t     vres;       // horizontal resolution in pixels/metre
	uint32_t    colors;     // number of colours or 0 for 1 << bpp
	uint32_t    important;  // number of important colours or 0 if all important
	uint32_t    red;        // red field mask - must be contiguous
	uint32_t    green;      // green field mask - must be contiguous
	uint32_t    blue;       // blue field mask - must be contiguous
	uint32_t    alpha;      // alpha field mask - must be contiguous
};


bool dib_parse_mask(uint32_t mask, unsigned &shift, unsigned &bits)
{
	shift = count_leading_zeros(mask);
	mask <<= shift;
	bits = count_leading_ones(mask);
	mask <<= shift;
	shift = 32 - shift - bits;
	return !mask;
}


void dib_truncate_channel(unsigned &shift, unsigned &bits)
{
	if (8U < bits)
	{
		unsigned const excess(bits - 8);
		shift += excess;
		bits -= excess;
	}
}


uint8_t dib_splat_sample(uint8_t val, unsigned bits)
{
	assert(8U >= bits);
	for (val <<= (8U - bits); bits && (8U > bits); bits <<= 1)
		val |= val >> bits;
	return val;
}


bool load_ico_png(util::core_file &fp, icon_dir_entry_t const &dir, bitmap_argb32 &bitmap)
{
	// skip out if the data isn't a reasonable size - PNG magic alone is eight bytes
	if (9U >= dir.size)
		return false;
	fp.seek(dir.offset, SEEK_SET);
	png_error const err(png_read_bitmap(fp, bitmap));
	switch (err)
	{
	case PNGERR_NONE:
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

	case PNGERR_BAD_SIGNATURE:
		// doesn't look like PNG data - just fall back to DIB without the file header
		return false;

	default:
		// invalid PNG data or I/O error
		LOG(
				"Error %u reading PNG image data from ICO file at offset %u (directory size %u)\n",
				unsigned(err),
				dir.offset,
				dir.size);
		return false;
	}
}


bool load_ico_dib(util::core_file &fp, icon_dir_entry_t const &dir, bitmap_argb32 &bitmap)
{
	// check that these things haven't been padded somehow
	static_assert(sizeof(bitmap_core_header_t) == 12U, "compiler has applied padding to bitmap_core_header_t");
	static_assert(sizeof(bitmap_info_header_t) == 56U, "compiler has applied padding to bitmap_info_header_t");

	// ensure the header fits in the space for the image data
	union { bitmap_core_header_t core; bitmap_info_header_t info; } header;
	assert(&header.core.size == &header.info.size);
	if (sizeof(header.core) > dir.size)
		return false;
	std::memset(&header, 0, sizeof(header));
	fp.seek(dir.offset, SEEK_SET);
	if (fp.read(&header.core.size, sizeof(header.core.size)) != sizeof(header.core.size))
	{
		LOG(
				"Error reading DIB header size from ICO file at offset %u (directory size %u)\n",
				dir.offset,
				dir.size);
		return false;
	}
	header.core.size = little_endianize_int32(header.core.size);
	if (dir.size < header.core.size)
	{
		LOG(
				"ICO file image data at %u (%u bytes) is too small for DIB header (%u bytes)\n",
				dir.offset,
				dir.size,
				header.core.size);
		return false;
	}

	// identify and read the header - convert OS/2 headers to Windows 3 format
	unsigned palette_bytes(4U);
	switch (header.core.size)
	{
	case 16U:
	case 64U:
		// extended OS/2 bitmap header with support for compression
		LOG(
				"ICO image data at %u (%u bytes) uses unsupported OS/2 DIB header (size %u)\n",
				dir.offset,
				dir.size,
				header.core.size);
		return false;

	case 12U:
		// introduced in OS/2 and Windows 2.0
		{
			palette_bytes = 3U;
			uint32_t const header_read(std::min<uint32_t>(header.core.size, sizeof(header.core)) - sizeof(header.core.size));
			if (fp.read(&header.core.width,  header_read) != header_read)
			{
				LOG("Error reading DIB core header from ICO file image data at %u (%u bytes)\n", dir.offset, dir.size);
				return false;
			}
			fp.seek(header.core.size - sizeof(header.core.size) - header_read, SEEK_CUR);
			header.core.width = little_endianize_int16(header.core.width);
			header.core.height = little_endianize_int16(header.core.height);
			header.core.planes = little_endianize_int16(header.core.planes);
			header.core.bpp = little_endianize_int16(header.core.bpp);
			LOGMASKED(
					LOG_DIB,
					"Read DIB core header from ICO file image data at %u: %d*%d, %u planes, %u bpp\n",
					dir.offset,
					header.core.width,
					header.core.height,
					header.core.planes,
					header.core.bpp);

			// this works because the core header only aliases the width/height of the info header
			header.info.bpp = header.core.bpp;
			header.info.planes = header.core.planes;
			header.info.height = header.core.height;
			header.info.width = header.core.width;
			header.info.size = 40U;
		}
		break;

	default:
		// the next version will be longer
		if (124U >= header.core.size)
		{
			LOG(
					"ICO image data at %u (%u bytes) uses unsupported DIB header format (size %u)\n",
					dir.offset,
					dir.size,
					header.core.size);
			return false;
		}
		// fall through
	case 40U:
	case 52U:
	case 56U:
	case 108U:
	case 124U:
		// the Windows 3 bitmap header with optional extensions
		{
			palette_bytes = 4U;
			uint32_t const header_read(std::min<uint32_t>(header.info.size, sizeof(header.info)) - sizeof(header.info.size));
			if (fp.read(&header.info.width,  header_read) != header_read)
			{
				LOG("Error reading DIB info header from ICO file image data at %u (%u bytes)\n", dir.offset, dir.size);
				return false;
			}
			fp.seek(header.info.size - sizeof(header.info.size) - header_read, SEEK_CUR);
			header.info.width = little_endianize_int32(header.info.width);
			header.info.height = little_endianize_int32(header.info.height);
			header.info.planes = little_endianize_int16(header.info.planes);
			header.info.bpp = little_endianize_int16(header.info.bpp);
			header.info.comp = little_endianize_int32(header.info.comp);
			header.info.rawsize = little_endianize_int32(header.info.rawsize);
			header.info.hres = little_endianize_int32(header.info.hres);
			header.info.vres = little_endianize_int32(header.info.vres);
			header.info.colors = little_endianize_int32(header.info.colors);
			header.info.important = little_endianize_int32(header.info.important);
			header.info.red = little_endianize_int32(header.info.red);
			header.info.green = little_endianize_int32(header.info.green);
			header.info.blue = little_endianize_int32(header.info.blue);
			header.info.alpha = little_endianize_int32(header.info.alpha);
			LOGMASKED(
					LOG_DIB,
					"Read DIB info header from ICO file image data at %u: %d*%d (%d*%d ppm), %u planes, %u bpp %u/%s%u colors\n",
					dir.offset,
					header.info.width,
					header.info.height,
					header.info.hres,
					header.info.vres,
					header.info.planes,
					header.info.bpp,
					header.info.important,
					header.info.colors ? "" : "2^",
					header.info.colors ? header.info.colors : header.info.bpp);
		}
		break;
	}

	// check for unsupported planes/bit depth
	if ((1U != header.info.planes) || !header.info.bpp || (32U < header.info.bpp) || ((8U < header.info.bpp) ? (header.info.bpp % 8) : (8 % header.info.bpp)))
	{
		LOG(
				"ICO file DIB image data at %u uses unsupported planes/bits per pixel %u*%u\n",
				dir.offset,
				header.info.planes,
				header.info.bpp);
		return false;
	}

	// check dimensions
	if ((0 >= header.info.width) || (0 == header.info.height))
	{
		LOG(
				"ICO file DIB image data at %u has invalid dimensions %u*%u\n",
				dir.offset,
				header.info.width,
				header.info.height);
		return false;
	}
	bool const top_down(0 > header.info.height);
	if (top_down)
		header.info.height = -header.info.height;
	bool have_and_mask((2 * dir.get_height()) == header.info.height);
	if (!have_and_mask && (dir.get_height() != header.info.height))
	{
		osd_printf_verbose(
				"ICO file DIB image data at %lu height %ld doesn't match directory height %u with or without AND mask\n",
				(unsigned long)dir.offset,
				(long)header.info.height,
				dir.get_height());
		return false;
	}
	if (have_and_mask)
		header.info.height >>= 1;

	// ensure compression scheme is supported
	bool indexed(true), no_palette(false);
	switch (header.info.comp)
	{
	case DIB_COMP_NONE:
		// uncompressed - direct colour with implied bitfields if more than eight bits/pixel
		indexed = 8U >= header.info.bpp;
		if (indexed)
		{
			if ((1U << header.info.bpp) < header.info.colors)
			{
				osd_printf_verbose(
						"ICO file DIB image data at %lu has oversized palette with %lu entries for %u bits per pixel\n",
						(unsigned long)dir.offset,
						(unsigned long)header.info.colors,
						(unsigned)header.info.bpp);
			}
		}
		if (!indexed)
		{
			no_palette = true;
			switch(header.info.bpp)
			{
			case 16U:
				header.info.red = 0x00007c00;
				header.info.green = 0x000003e0;
				header.info.blue = 0x0000001f;
				header.info.alpha = 0x00000000;
				break;
			case 24U:
			case 32U:
				header.info.red = 0x00ff0000;
				header.info.green = 0x0000ff00;
				header.info.blue = 0x000000ff;
				header.info.alpha = 0x00000000;
				break;
			}
		}
		break;

	case DIB_COMP_BITFIELDS:
		// uncompressed direct colour with explicitly-specified bitfields
		indexed = false;
		if (offsetof(bitmap_info_header_t, alpha) > header.info.size)
		{
			osd_printf_verbose(
					"ICO file DIB image data at %lu specifies bit masks but is too small (size %lu)\n",
					(unsigned long)dir.offset,
					(unsigned long)header.info.size);
			return false;
		}
		break;

	default:
		LOG("ICO file DIB image data at %u uses unsupported compression scheme %u\n", header.info.comp);
		return false;
	}

	// we can now calculate the size of the palette and row data
	size_t const palette_entries(
			indexed
				? ((1U == header.info.bpp) ? 2U : header.info.colors ? header.info.colors : (1U << header.info.bpp))
				: (no_palette ? 0U : header.info.colors));
	size_t const palette_size(palette_bytes * palette_entries);
	size_t const row_bytes(((31 + (header.info.width * header.info.bpp)) >> 5) << 2);
	size_t const mask_row_bytes(((31 + header.info.width) >> 5) << 2);
	size_t const required_size(
			header.info.size +
			palette_size +
			((row_bytes + (have_and_mask ? mask_row_bytes : 0U)) * header.info.height));
	if (required_size > dir.size)
	{
		LOG(
				"ICO file image data at %u (%u bytes) smaller than calculated DIB data size (%u bytes)\n",
				dir.offset,
				dir.size,
				required_size);
		return false;
	}

	// load the palette for indexed colour formats or the shifts for direct colour formats
	unsigned red_shift(0), green_shift(0), blue_shift(0), alpha_shift(0);
	unsigned red_bits(0), green_bits(0), blue_bits(0), alpha_bits(0);
	std::unique_ptr<rgb_t []> palette;
	if (indexed)
	{
		// read palette and convert
		std::unique_ptr<uint8_t []> palette_data(new uint8_t [palette_size]);
		if (fp.read(palette_data.get(), palette_size) != palette_size)
		{
			LOG("Error reading palette from ICO file DIB image data at %u (%u bytes)\n", dir.offset, dir.size);
			return false;
		}
		size_t const palette_usable(std::min<size_t>(palette_entries, size_t(1) << header.info.bpp));
		palette.reset(new rgb_t [palette_usable]);
		uint8_t const *ptr(palette_data.get());
		for (size_t i = 0; palette_usable > i; ++i, ptr += palette_bytes)
			palette[i] = rgb_t(ptr[2], ptr[1], ptr[0]);
	}
	else
	{
		// skip over the palette if necessary
		if (palette_entries)
			fp.seek(palette_bytes * palette_entries, SEEK_CUR);

		// convert masks to shifts
		bool const masks_contiguous(
				dib_parse_mask(header.info.red, red_shift, red_bits) &&
				dib_parse_mask(header.info.green, green_shift, green_bits) &&
				dib_parse_mask(header.info.blue, blue_shift, blue_bits) &&
				dib_parse_mask(header.info.alpha, alpha_shift, alpha_bits));
		if (!masks_contiguous)
		{
			osd_printf_verbose(
					"ICO file DIB image data at %lu specifies non-contiguous channel masks 0x%lx | 0x%lx | 0x%lx | 0x%lx\n",
					(unsigned long)dir.offset,
					(unsigned long)header.info.red,
					(unsigned long)header.info.green,
					(unsigned long)header.info.blue,
					(unsigned long)header.info.alpha);
		}
		if ((32U != header.info.bpp) && ((header.info.red | header.info.green | header.info.blue | header.info.alpha) >> header.info.bpp))
		{
			LOG(
					"ICO file DIB image data at %lu specifies channel masks 0x%x | 0x%x | 0x%x | 0x%x that exceed %u bits per pixel\n",
					dir.offset,
					header.info.red,
					header.info.green,
					header.info.blue,
					header.info.alpha,
					header.info.bpp);
			return false;
		}
		LOGMASKED(
				LOG_DIB,
				"DIB from ICO file image data at %1$u using channels: R((x >> %3$u) & 0x%4$0*2$x) G((x >> %5$u) & 0x%6$0*2$x) B((x >> %7$u) & 0x%8$0*2$x) A((x >> %9$u) & 0x%10$0*2$x)\n",
				dir.offset,
				(header.info.bpp + 3) >> 2,
				red_shift,
				(uint32_t(1) << red_bits) - 1,
				green_shift,
				(uint32_t(1) << green_bits) - 1,
				blue_shift,
				(uint32_t(1) << blue_bits) - 1,
				alpha_shift,
				(uint32_t(1) << alpha_bits) - 1);

		// the MAME bitmap only supports 8 bits/sample maximum
		dib_truncate_channel(red_shift, red_bits);
		dib_truncate_channel(green_shift, green_bits);
		dib_truncate_channel(blue_shift, blue_bits);
		dib_truncate_channel(alpha_shift, alpha_bits);
	}

	// allocate the bitmap and process row data
	std::unique_ptr<uint8_t []> row_data(new uint8_t [row_bytes]);
	bitmap.allocate(header.info.width, header.info.height);
	int const y_inc(top_down ? 1 : -1);
	for (int32_t i = 0, y = top_down ? 0 : (header.info.height - 1); header.info.height > i; ++i, y += y_inc)
	{
		if (fp.read(row_data.get(), row_bytes) != row_bytes)
		{
			LOG("Error reading DIB row %d data from ICO image data at %u\n", i, dir.offset);
			return false;
		}
		uint8_t *src(row_data.get());
		uint32_t *dest(&bitmap.pix(y));
		unsigned shift(0U);
		for (int32_t x = 0; header.info.width > x; ++x, ++dest)
		{
			// extract or compose a pixel
			uint32_t pix(0U);
			if (8U >= header.info.bpp)
			{
				assert(8U > shift);
				pix = *src >> (8U - header.info.bpp);
				*src <<= header.info.bpp;
				shift += header.info.bpp;
				if (8U <= shift)
				{
					shift = 0U;
					++src;
				}
			}
			else for (shift = 0; header.info.bpp > shift; shift += 8U, ++src)
			{
				pix |= uint32_t(*src) << shift;
			}

			// convert to RGB
			if (indexed)
			{
				if (palette_entries > pix)
				{
					*dest = palette[pix];
				}
				else
				{
					*dest = rgb_t::transparent();
					osd_printf_verbose(
							"ICO file DIB image data at %lu has out-of-range color %lu at (%ld, %ld) with %lu palette entries\n",
							(unsigned long)dir.offset,
							(unsigned long)pix,
							(long)x,
							(long)y,
							(unsigned long)palette_entries);
				}
			}
			else
			{
				uint8_t r(dib_splat_sample((pix >> red_shift) & ((uint32_t(1) << red_bits) - 1), red_bits));
				uint8_t g(dib_splat_sample((pix >> green_shift) & ((uint32_t(1) << green_bits) - 1), green_bits));
				uint8_t b(dib_splat_sample((pix >> blue_shift) & ((uint32_t(1) << blue_bits) - 1), blue_bits));
				uint8_t a(dib_splat_sample((pix >> alpha_shift) & ((uint32_t(1) << alpha_bits) - 1), alpha_bits));
				*dest = rgb_t(alpha_bits ? a : 255, r, g, b);
			}
		}
	}

	// process the AND mask if present
	if (have_and_mask)
	{
		for (int32_t i = 0, y = top_down ? 0 : (header.info.height - 1); header.info.height > i; ++i, y += y_inc)
		{
			if (fp.read(row_data.get(), mask_row_bytes) != mask_row_bytes)
			{
				LOG("Error reading DIB mask row %d data from ICO image data at %u\n", i, dir.offset);
				return false;
			}
			uint8_t *src(row_data.get());
			uint32_t *dest(&bitmap.pix(y));
			unsigned shift(0U);
			for (int32_t x = 0; header.info.width > x; ++x, ++dest)
			{
				assert(8U > shift);
				rgb_t pix(*dest);
				*dest = pix.set_a(BIT(*src, 7U - shift) ? 0U : pix.a());
				if (8U <= ++shift)
				{
					shift = 0U;
					++src;
				}
			}
		}
	}

	// we're done!
	return true;
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
	icon_dir_entry_t dir;
	fp.seek(sizeof(icon_dir_t) + (sizeof(icon_dir_entry_t) * index), SEEK_SET);
	if (fp.read(&dir, sizeof(dir)) != sizeof(dir))
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
	icon_dir_t header;
	fp.seek(0, SEEK_SET);
	if (fp.read(&header, sizeof(header)) != sizeof(header))
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
		std::unique_ptr<icon_dir_entry_t []> dir(new icon_dir_entry_t [count]);
		std::unique_ptr<unsigned []> index(new unsigned [count]);
		if (count && (fp.read(dir.get(), dir_bytes) != dir_bytes))
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
