// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    msdib.h

    Microsoft Device-Independent Bitmap file loading.

***************************************************************************/

#include "msdib.h"

#include "eminline.h"

#include <cassert>
#include <cstdlib>
#include <cstring>


#define LOG_GENERAL (1U << 0)
#define LOG_DIB     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_DIB)

#define LOG_OUTPUT_FUNC osd_printf_verbose

#ifndef VERBOSE
#define VERBOSE 0
#endif

#define LOGMASKED(mask, ...) do { if (VERBOSE & (mask)) (LOG_OUTPUT_FUNC)(__VA_ARGS__); } while (false)

#define LOG(...) LOGMASKED(LOG_GENERAL, __VA_ARGS__)


namespace util {

namespace {

// DIB compression schemes
enum : std::uint32_t
{
	DIB_COMP_NONE       = 0,
	DIB_COMP_RLE8       = 1,
	DIB_COMP_RLE4       = 2,
	DIB_COMP_BITFIELDS  = 3
};


// file header doesn't use natural alignment
using bitmap_file_header = std::uint8_t [14];


// old-style DIB header
struct bitmap_core_header
{
	std::uint32_t       size;       // size of the header (12, 16 or 64)
	std::int16_t        width;      // width of bitmap in pixels
	std::int16_t        height;     // height of the image in pixels
	std::uint16_t       planes;     // number of colour planes (must be 1)
	std::uint16_t       bpp;        // bits per pixel
};

// new-style DIB header
struct bitmap_info_header
{
	std::uint32_t       size;       // size of the header
	std::int32_t        width;      // width of bitmap in pixels
	std::int32_t        height;     // height of bitmap in pixels
	std::uint16_t       planes;     // number of colour planes (must be 1)
	std::uint16_t       bpp;        // bits per pixel
	std::uint32_t       comp;       // compression method
	std::uint32_t       rawsize;    // size of bitmap data after decompression or 0 if uncompressed
	std::int32_t        hres;       // horizontal resolution in pixels/metre
	std::int32_t        vres;       // horizontal resolution in pixels/metre
	std::uint32_t       colors;     // number of colours or 0 for 1 << bpp
	std::uint32_t       important;  // number of important colours or 0 if all important
	std::uint32_t       red;        // red field mask - must be contiguous
	std::uint32_t       green;      // green field mask - must be contiguous
	std::uint32_t       blue;       // blue field mask - must be contiguous
	std::uint32_t       alpha;      // alpha field mask - must be contiguous
};


union bitmap_headers
{
	bitmap_core_header  core;
	bitmap_info_header  info;
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


msdib_error dib_read_file_header(core_file &fp, std::uint32_t &filelen)
{
	// the bitmap file header doesn't use natural alignment
	bitmap_file_header file_header;
	if (fp.read(file_header, sizeof(file_header)) != sizeof(file_header))
	{
		LOG("Error reading DIB file header\n");
		return msdib_error::FILE_TRUNCATED;
	}

	// only support Windows bitmaps for now
	if ((0x42 != file_header[0]) || (0x4d != file_header[1]))
		return msdib_error::BAD_SIGNATURE;

	// do a very basic check on the file length
	std::uint32_t const file_length(
			(std::uint32_t(file_header[2]) << 0) |
			(std::uint32_t(file_header[3]) << 8) |
			(std::uint32_t(file_header[4]) << 16) |
			(std::uint32_t(file_header[5]) << 24));
	if ((sizeof(file_header) + sizeof(bitmap_core_header)) > file_length)
		return msdib_error::FILE_CORRUPT;

	// check that the offset to the pixel data looks half sane
	std::uint32_t const pixel_offset(
			(std::uint32_t(file_header[10]) << 0) |
			(std::uint32_t(file_header[11]) << 8) |
			(std::uint32_t(file_header[12]) << 16) |
			(std::uint32_t(file_header[13]) << 24));
	if (((sizeof(file_header) + sizeof(bitmap_core_header)) > pixel_offset) || (file_length < pixel_offset))
		return msdib_error::FILE_CORRUPT;

	// looks OK enough
	filelen = file_length;
	return msdib_error::NONE;
}


msdib_error dib_read_bitmap_header(
		core_file &fp,
		bitmap_headers &header,
		unsigned &palette_bytes,
		bool &indexed,
		std::size_t &palette_entries,
		std::size_t &palette_size,
		std::size_t &row_bytes,
		std::uint32_t length)
{
	// check that these things haven't been padded somehow
	static_assert(sizeof(bitmap_core_header) == 12U, "compiler has applied padding to bitmap_core_header");
	static_assert(sizeof(bitmap_info_header) == 56U, "compiler has applied padding to bitmap_info_header");

	// ensure the header fits in the space for the image data
	assert(&header.core.size == &header.info.size);
	if (sizeof(header.core) > length)
		return msdib_error::FILE_TRUNCATED;
	std::memset(&header, 0, sizeof(header));
	if (fp.read(&header.core.size, sizeof(header.core.size)) != sizeof(header.core.size))
	{
		LOG("Error reading DIB header size (length %u)\n", length);
		return msdib_error::FILE_TRUNCATED;
	}
	header.core.size = little_endianize_int32(header.core.size);
	if (length < header.core.size)
	{
		LOG("DIB image data (%u bytes) is too small for DIB header (%u bytes)\n", length, header.core.size);
		return msdib_error::FILE_CORRUPT;
	}

	// identify and read the header - convert OS/2 headers to Windows 3 format
	palette_bytes = 4U;
	switch (header.core.size)
	{
	case 16U:
	case 64U:
		// extended OS/2 bitmap header with support for compression
		LOG(
				"DIB image data (%u bytes) uses unsupported OS/2 DIB header (size %u)\n",
				length,
				header.core.size);
		return msdib_error::UNSUPPORTED_FORMAT;

	case 12U:
		// introduced in OS/2 and Windows 2.0
		{
			palette_bytes = 3U;
			std::uint32_t const header_read(std::min<std::uint32_t>(header.core.size, sizeof(header.core)) - sizeof(header.core.size));
			if (fp.read(&header.core.width, header_read) != header_read)
			{
				LOG("Error reading DIB core header from image data (%u bytes)\n", length);
				return msdib_error::FILE_TRUNCATED;
			}
			fp.seek(header.core.size - sizeof(header.core.size) - header_read, SEEK_CUR);
			header.core.width = little_endianize_int16(header.core.width);
			header.core.height = little_endianize_int16(header.core.height);
			header.core.planes = little_endianize_int16(header.core.planes);
			header.core.bpp = little_endianize_int16(header.core.bpp);
			LOGMASKED(
					LOG_DIB,
					"Read DIB core header from image data : %d*%d, %u planes, %u bpp\n",
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
					"DIB image data (%u bytes) uses unsupported DIB header format (size %u)\n",
					length,
					header.core.size);
			return msdib_error::UNSUPPORTED_FORMAT;
		}
		[[fallthrough]];
	case 40U:
	case 52U:
	case 56U:
	case 108U:
	case 124U:
		// the Windows 3 bitmap header with optional extensions
		{
			palette_bytes = 4U;
			std::uint32_t const header_read(std::min<std::uint32_t>(header.info.size, sizeof(header.info)) - sizeof(header.info.size));
			if (fp.read(&header.info.width,  header_read) != header_read)
			{
				LOG("Error reading DIB info header from image data (%u bytes)\n", length);
				return msdib_error::FILE_TRUNCATED;
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
					"Read DIB info header from image data: %d*%d (%d*%d ppm), %u planes, %u bpp %u/%s%u colors\n",
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
		LOG("DIB image data uses unsupported planes/bits per pixel %u*%u\n", header.info.planes, header.info.bpp);
		return msdib_error::UNSUPPORTED_FORMAT;
	}

	// check dimensions
	if ((0 >= header.info.width) || (0 == header.info.height))
	{
		LOG("DIB image data has invalid dimensions %u*%u\n", header.info.width, header.info.height);
		return msdib_error::FILE_CORRUPT;
	}

	// ensure compression scheme is supported
	bool no_palette(false);
	indexed = true;
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
						"DIB image data has oversized palette with %u entries for %u bits per pixel\n",
						header.info.colors,
						header.info.bpp);
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
		if (offsetof(bitmap_info_header, alpha) > header.info.size)
		{
			osd_printf_verbose(
					"DIB image data specifies bit masks but is too small (size %u)\n",
					header.info.size);
			return msdib_error::FILE_CORRUPT;
		}
		break;

	default:
		LOG("DIB image data uses unsupported compression scheme %u\n", header.info.comp);
		return msdib_error::UNSUPPORTED_FORMAT;
	}

	// we can now calculate the size of the palette and row data
	palette_entries =
			indexed
				? ((1U == header.info.bpp) ? 2U : header.info.colors ? header.info.colors : (1U << header.info.bpp))
				: (no_palette ? 0U : header.info.colors);
	palette_size = palette_bytes * palette_entries;
	row_bytes = ((31 + (header.info.width * header.info.bpp)) >> 5) << 2;

	// header looks OK
	return msdib_error::NONE;
}

} // anonymous namespace



msdib_error msdib_verify_header(core_file &fp)
{
	msdib_error err;

	// file header
	std::uint32_t file_length;
	err = dib_read_file_header(fp, file_length);
	if (msdib_error::NONE != err)
		return err;

	// bitmap header
	bitmap_headers header;
	unsigned palette_bytes;
	bool indexed;
	std::size_t palette_entries, palette_size, row_bytes;
	err = dib_read_bitmap_header(
			fp,
			header,
			palette_bytes,
			indexed,
			palette_entries,
			palette_size,
			row_bytes,
			file_length - sizeof(bitmap_file_header));
	if (msdib_error::NONE != err)
		return err;

	// check length
	std::size_t const required_size(
			sizeof(bitmap_file_header) +
			header.info.size +
			palette_size +
			(row_bytes * std::abs(header.info.height)));
	if (required_size > file_length)
		return msdib_error::FILE_TRUNCATED;

	// good chance this file is supported
	return msdib_error::NONE;
}


msdib_error msdib_read_bitmap(core_file &fp, bitmap_argb32 &bitmap)
{
	std::uint32_t file_length;
	msdib_error const headerr(dib_read_file_header(fp, file_length));
	if (msdib_error::NONE != headerr)
		return headerr;

	return msdib_read_bitmap_data(fp, bitmap, file_length - sizeof(bitmap_file_header), 0U);
}


msdib_error msdib_read_bitmap_data(core_file &fp, bitmap_argb32 &bitmap, std::uint32_t length, std::uint32_t dirheight)
{
	// read the bitmap header
	bitmap_headers header;
	unsigned palette_bytes;
	bool indexed;
	std::size_t palette_entries, palette_size, row_bytes;
	msdib_error const head_error(dib_read_bitmap_header(fp, header, palette_bytes, indexed, palette_entries, palette_size, row_bytes, length));
	if (msdib_error::NONE != head_error)
		return head_error;

	// check dimensions
	bool const top_down(0 > header.info.height);
	if (top_down)
		header.info.height = -header.info.height;
	bool have_and_mask((2 * dirheight) == header.info.height);
	if (!have_and_mask && (0 < dirheight) && (dirheight != header.info.height))
	{
		osd_printf_verbose(
				"DIB image data height %d doesn't match directory height %u with or without AND mask\n",
				header.info.height,
				dirheight);
		return msdib_error::UNSUPPORTED_FORMAT;
	}
	if (have_and_mask)
		header.info.height >>= 1;

	// we can now calculate the size of the image data
	std::size_t const mask_row_bytes(((31 + header.info.width) >> 5) << 2);
	std::size_t const required_size(
			header.info.size +
			palette_size +
			((row_bytes + (have_and_mask ? mask_row_bytes : 0U)) * header.info.height));
	if (required_size > length)
	{
		LOG("DIB image data (%u bytes) smaller than calculated DIB data size (%u bytes)\n", length, required_size);
		return msdib_error::FILE_TRUNCATED;
	}

	// load the palette for indexed colour formats or the shifts for direct colour formats
	unsigned red_shift(0), green_shift(0), blue_shift(0), alpha_shift(0);
	unsigned red_bits(0), green_bits(0), blue_bits(0), alpha_bits(0);
	std::unique_ptr<rgb_t []> palette;
	if (indexed)
	{
		// read palette and convert
		std::unique_ptr<std::uint8_t []> palette_data;
		try { palette_data.reset(new std::uint8_t [palette_size]); }
		catch (std::bad_alloc const &) { return msdib_error::OUT_OF_MEMORY; }
		if (fp.read(palette_data.get(), palette_size) != palette_size)
		{
			LOG("Error reading palette from DIB image data (%u bytes)\n", length);
			return msdib_error::FILE_TRUNCATED;
		}
		std::size_t const palette_usable(std::min<std::size_t>(palette_entries, std::size_t(1) << header.info.bpp));
		try { palette.reset(new rgb_t [palette_usable]); }
		catch (std::bad_alloc const &) { return msdib_error::OUT_OF_MEMORY; }
		std::uint8_t const *ptr(palette_data.get());
		for (std::size_t i = 0; palette_usable > i; ++i, ptr += palette_bytes)
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
					"DIB image data specifies non-contiguous channel masks 0x%x | 0x%x | 0x%x | 0x%x\n",
					header.info.red,
					header.info.green,
					header.info.blue,
					header.info.alpha);
		}
		if ((32U != header.info.bpp) && ((header.info.red | header.info.green | header.info.blue | header.info.alpha) >> header.info.bpp))
		{
			LOG(
					"DIB image data specifies channel masks 0x%x | 0x%x | 0x%x | 0x%x that exceed %u bits per pixel\n",
					header.info.red,
					header.info.green,
					header.info.blue,
					header.info.alpha,
					header.info.bpp);
			return msdib_error::FILE_CORRUPT;
		}
		LOGMASKED(
				LOG_DIB,
				"DIB image data using channels: R((x >> %2$u) & 0x%3$0*1$x) G((x >> %4$u) & 0x%5$0*1$x) B((x >> %6$u) & 0x%7$0*1$x) A((x >> %8$u) & 0x%9$0*1$x)\n",
				(header.info.bpp + 3) >> 2,
				red_shift,
				(std::uint32_t(1) << red_bits) - 1,
				green_shift,
				(std::uint32_t(1) << green_bits) - 1,
				blue_shift,
				(std::uint32_t(1) << blue_bits) - 1,
				alpha_shift,
				(std::uint32_t(1) << alpha_bits) - 1);

		// the MAME bitmap only supports 8 bits/sample maximum
		dib_truncate_channel(red_shift, red_bits);
		dib_truncate_channel(green_shift, green_bits);
		dib_truncate_channel(blue_shift, blue_bits);
		dib_truncate_channel(alpha_shift, alpha_bits);
	}

	// allocate the bitmap and process row data
	std::unique_ptr<std::uint8_t []> row_data;
	try {row_data.reset(new std::uint8_t [row_bytes]); }
	catch (std::bad_alloc const &) { return msdib_error::OUT_OF_MEMORY; }
	bitmap.allocate(header.info.width, header.info.height);
	int const y_inc(top_down ? 1 : -1);
	for (std::int32_t i = 0, y = top_down ? 0 : (header.info.height - 1); header.info.height > i; ++i, y += y_inc)
	{
		if (fp.read(row_data.get(), row_bytes) != row_bytes)
		{
			LOG("Error reading DIB row %d data from image data\n", i);
			return msdib_error::FILE_TRUNCATED;
		}
		std::uint8_t *src(row_data.get());
		std::uint32_t *dest(&bitmap.pix(y));
		unsigned shift(0U);
		for (std::int32_t x = 0; header.info.width > x; ++x, ++dest)
		{
			// extract or compose a pixel
			std::uint32_t pix(0U);
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
				pix |= std::uint32_t(*src) << shift;
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
							"DIB image data has out-of-range color %u at (%d, %d) with %u palette entries\n",
							pix,
							x,
							y,
							palette_entries);
				}
			}
			else
			{
				std::uint8_t r(dib_splat_sample((pix >> red_shift) & ((std::uint32_t(1) << red_bits) - 1), red_bits));
				std::uint8_t g(dib_splat_sample((pix >> green_shift) & ((std::uint32_t(1) << green_bits) - 1), green_bits));
				std::uint8_t b(dib_splat_sample((pix >> blue_shift) & ((std::uint32_t(1) << blue_bits) - 1), blue_bits));
				std::uint8_t a(dib_splat_sample((pix >> alpha_shift) & ((std::uint32_t(1) << alpha_bits) - 1), alpha_bits));
				*dest = rgb_t(alpha_bits ? a : 255, r, g, b);
			}
		}
	}

	// process the AND mask if present
	if (have_and_mask)
	{
		for (std::int32_t i = 0, y = top_down ? 0 : (header.info.height - 1); header.info.height > i; ++i, y += y_inc)
		{
			if (fp.read(row_data.get(), mask_row_bytes) != mask_row_bytes)
			{
				LOG("Error reading DIB mask row %d data from image data\n", i);
				return msdib_error::FILE_TRUNCATED;
			}
			std::uint8_t *src(row_data.get());
			std::uint32_t *dest(&bitmap.pix(y));
			unsigned shift(0U);
			for (std::int32_t x = 0; header.info.width > x; ++x, ++dest)
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
	return msdib_error::NONE;
}

} // namespace util
