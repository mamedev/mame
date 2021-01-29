// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/*********************************************************************

    png.cpp

    PNG reading functions.

***************************************************************************/

#include "png.h"

#include "osdcomm.h"
#include "unicode.h"

#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <new>


namespace util {

/***************************************************************************
    GENERAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    free_data - free all memory allocated in a
    pnginfo structure
-------------------------------------------------*/

void png_info::free_data()
{
	textlist.clear();
	palette.reset();
	trans.reset();
	image.reset();
}



namespace {

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

constexpr int samples[] = { 1, 0, 3, 1, 2, 0, 4 };

constexpr std::uint8_t PNG_SIGNATURE[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
#define MNG_Signature       "\x8A\x4D\x4E\x47\x0D\x0A\x1A\x0A"

// Chunk names
constexpr std::uint32_t PNG_CN_IHDR     = 0x49484452L;
constexpr std::uint32_t PNG_CN_PLTE     = 0x504C5445L;
constexpr std::uint32_t PNG_CN_IDAT     = 0x49444154L;
constexpr std::uint32_t PNG_CN_IEND     = 0x49454E44L;
constexpr std::uint32_t PNG_CN_gAMA     = 0x67414D41L;
//constexpr std::uint32_t PNG_CN_sBIT     = 0x73424954L;
//constexpr std::uint32_t PNG_CN_cHRM     = 0x6348524DL;
constexpr std::uint32_t PNG_CN_tRNS     = 0x74524E53L;
//constexpr std::uint32_t PNG_CN_bKGD     = 0x624B4744L;
//constexpr std::uint32_t PNG_CN_hIST     = 0x68495354L;
constexpr std::uint32_t PNG_CN_tEXt     = 0x74455874L;
//constexpr std::uint32_t PNG_CN_zTXt     = 0x7A545874L;
constexpr std::uint32_t PNG_CN_pHYs     = 0x70485973L;
//constexpr std::uint32_t PNG_CN_oFFs     = 0x6F464673L;
//constexpr std::uint32_t PNG_CN_tIME     = 0x74494D45L;
//constexpr std::uint32_t PNG_CN_sCAL     = 0x7343414CL;

// MNG Chunk names
constexpr std::uint32_t MNG_CN_MHDR     = 0x4D484452L;
constexpr std::uint32_t MNG_CN_MEND     = 0x4D454E44L;
//constexpr std::uint32_t MNG_CN_TERM     = 0x5445524DL;
//constexpr std::uint32_t MNG_CN_BACK     = 0x4241434BL;

// Prediction filters
constexpr std::uint8_t  PNG_PF_None     = 0;
constexpr std::uint8_t  PNG_PF_Sub      = 1;
constexpr std::uint8_t  PNG_PF_Up       = 2;
constexpr std::uint8_t  PNG_PF_Average  = 3;
constexpr std::uint8_t  PNG_PF_Paeth    = 4;


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline int compute_rowbytes(const png_info &pnginfo) { return (pnginfo.width * samples[pnginfo.color_type] * pnginfo.bit_depth + 7) / 8; }

inline uint8_t fetch_8bit(uint8_t const *v) { return *v; }
inline uint16_t fetch_16bit(uint8_t const *v) { return big_endianize_int16(*reinterpret_cast<uint16_t const *>(v)); }
inline uint32_t fetch_32bit(uint8_t const *v) { return big_endianize_int32(*reinterpret_cast<uint32_t const *>(v)); }

inline void put_8bit(uint8_t *v, uint8_t data) { *v = data; }
inline void put_16bit(uint8_t *v, uint16_t data) { *reinterpret_cast<uint16_t *>(v) = big_endianize_int16(data); }
inline void put_32bit(uint8_t *v, uint32_t data) { *reinterpret_cast<uint32_t *>(v) = big_endianize_int32(data); }


class png_private
{
private:
	static constexpr unsigned ADAM7_X_BIAS[7]   = { 7, 3, 3, 1, 1, 0, 0 };
	static constexpr unsigned ADAM7_Y_BIAS[7]   = { 7, 7, 3, 3, 1, 1, 0 };
	static constexpr unsigned ADAM7_X_SHIFT[7]  = { 3, 3, 2, 2, 1, 1, 0 };
	static constexpr unsigned ADAM7_Y_SHIFT[7]  = { 3, 3, 3, 2, 2, 1, 1 };
	static constexpr unsigned ADAM7_X_OFFS[7]   = { 0, 4, 0, 2, 0, 1, 0 };
	static constexpr unsigned ADAM7_Y_OFFS[7]   = { 0, 0, 4, 0, 2, 0, 1 };

	struct image_data_chunk
	{
		image_data_chunk(std::uint32_t l, std::unique_ptr<std::uint8_t []> &&d) : length(l), data(std::move(d)) { }

		std::uint32_t                       length;
		std::unique_ptr<std::uint8_t []>    data;
	};

	png_error process(std::list<image_data_chunk> const &idata)
	{
		// do some basic checks for unsupported images
		if (!pnginfo.bit_depth || (ARRAY_LENGTH(samples) <= pnginfo.color_type) || !samples[pnginfo.color_type])
			return png_error::UNSUPPORTED_FORMAT; // unknown colour format
		if ((0 != pnginfo.interlace_method) && (1 != pnginfo.interlace_method))
			return png_error::UNSUPPORTED_FORMAT; // unknown interlace method
		if ((3 == pnginfo.color_type) && (!pnginfo.num_palette || !pnginfo.palette))
			return png_error::FILE_CORRUPT; // indexed colour with no palette

		// calculate the offset for each pass of the interlace
		unsigned const pass_count(get_pass_count());
		std::uint32_t pass_offset[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		for (unsigned pass = 0; pass_count > pass; ++pass)
			pass_offset[pass + 1] = pass_offset[pass] + get_pass_bytes(pass);

		// allocate memory for the filtered image
		try { pnginfo.image.reset(new std::uint8_t [pass_offset[pass_count]]); }
		catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }

		// decompress image data
		png_error error = png_error::NONE;
		error = decompress(idata, pass_offset[pass_count]);
		std::uint32_t const bpp(get_bytes_per_pixel());
		for (unsigned pass = 0; (pass_count > pass) && (png_error::NONE == error); ++pass)
		{
			// compute some basic parameters
			std::pair<std::uint32_t, std::uint32_t> const dimensions(get_pass_dimensions(pass));
			std::uint32_t const rowbytes(get_row_bytes(dimensions.first));

			// we de-filter in place, stripping the filter bytes off the rows
			uint8_t *dst(&pnginfo.image[pass_offset[pass]]);
			uint8_t const *src(dst);
			for (std::uint32_t y = 0; (dimensions.second > y) && (png_error::NONE == error); ++y)
			{
				// first byte of each row is the filter type
				uint8_t const filter(*src++);
				error = unfilter_row(filter, src, dst, y ? (dst - rowbytes) : nullptr, bpp, rowbytes);
				src += rowbytes;
				dst += rowbytes;
			}
		}

		// if we errored, free the image data
		if (error != png_error::NONE)
			pnginfo.image.reset();

		return error;
	}

	png_error decompress(std::list<image_data_chunk> const &idata, std::uint32_t expected)
	{
		// only deflate is permitted
		if (0 != pnginfo.compression_method)
			return png_error::DECOMPRESS_ERROR;

		// allocate zlib stream
		z_stream stream;
		int zerr;
		std::memset(&stream, 0, sizeof(stream));
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = 0;
		stream.next_in = Z_NULL;
		zerr = inflateInit(&stream);
		if (Z_OK != zerr)
			return png_error::DECOMPRESS_ERROR;

		// decompress IDAT blocks
		stream.next_out = pnginfo.image.get();
		stream.avail_out = expected;
		stream.avail_in = 0;
		auto it = idata.begin();
		while ((idata.end() != it) && ((Z_OK == zerr) || (Z_BUF_ERROR == zerr)) && !stream.avail_in)
		{
			stream.avail_in = it->length;
			stream.next_in = it->data.get();
			do
			{
				zerr = inflate(&stream, Z_NO_FLUSH);
			}
			while (stream.avail_in && (Z_OK == zerr));
			if (!stream.avail_in)
				++it;
		}

		// it's all good if we got end-of-stream or we have with no data remaining
		if ((Z_OK == inflateEnd(&stream)) && ((Z_STREAM_END == zerr) || ((Z_OK == zerr) && (idata.end() == it) && !stream.avail_in)))
			return png_error::NONE;
		else
			return png_error::DECOMPRESS_ERROR;
	}

	png_error unfilter_row(std::uint8_t type, uint8_t const *src, uint8_t *dst, uint8_t const *dstprev, int bpp, std::uint32_t rowbytes)
	{
		if (0 != pnginfo.filter_method)
			return png_error::UNKNOWN_FILTER;

		switch (type)
		{
		case PNG_PF_None: // no filter, just copy
			std::copy_n(src, rowbytes, dst);
			return png_error::NONE;

		case PNG_PF_Sub: // SUB = previous pixel
			dst = std::copy_n(src, bpp, dst);
			src += bpp;
			for (std::uint32_t x = bpp; rowbytes > x; ++x, ++src, ++dst)
				*dst = *src + dst[-bpp];
			return png_error::NONE;

		case PNG_PF_Up: // UP = pixel above
			if (dstprev)
			{
				for (std::uint32_t x = 0; rowbytes > x; ++x, ++src, ++dst, ++dstprev)
					*dst = *src + *dstprev;
			}
			else
			{
				std::copy_n(src, rowbytes, dst);
			}
			return png_error::NONE;

		case PNG_PF_Average: // AVERAGE = average of pixel above and previous pixel
			if (dstprev)
			{
				for (std::uint32_t x = 0; bpp > x; ++x, ++src, ++dst, ++dstprev)
					*dst = *src + (*dstprev >> 1);
				for (std::uint32_t x = bpp; rowbytes > x; ++x, ++src, ++dst, ++dstprev)
					*dst = *src + ((*dstprev + dst[-bpp]) >> 1);
			}
			else
			{
				dst = std::copy_n(src, bpp, dst);
				src += bpp;
				for (std::uint32_t x = bpp; rowbytes > x; ++x, ++src, ++dst)
					*dst = *src + (dst[-bpp] >> 1);
			}
			return png_error::NONE;

		case PNG_PF_Paeth: // PAETH = special filter
			for (std::uint32_t x = 0; rowbytes > x; ++x, ++src, ++dst)
			{
				int32_t const pa((x < bpp) ? 0 : dst[-bpp]);
				int32_t const pc(((x < bpp) || !dstprev) ? 0 : dstprev[-bpp]);
				int32_t const pb(!dstprev ? 0 : *dstprev++);
				int32_t const prediction(pa + pb - pc);
				int32_t const da(std::abs(prediction - pa));
				int32_t const db(std::abs(prediction - pb));
				int32_t const dc(std::abs(prediction - pc));
				*dst = ((da <= db) && (da <= dc)) ? (*src + pa) : (db <= dc) ? (*src + pb) : (*src + pc);
			}
			return png_error::NONE;

		default: // unknown filter type
			return png_error::UNKNOWN_FILTER;
		}
	}

	png_error process_chunk(std::list<image_data_chunk> &idata, std::unique_ptr<std::uint8_t []> &&data, uint32_t type, uint32_t length)
	{
		switch (type)
		{
		case PNG_CN_IHDR: // image header
			if (13 > length)
				return png_error::FILE_CORRUPT;
			pnginfo.width = fetch_32bit(&data[0]);
			pnginfo.height = fetch_32bit(&data[4]);
			pnginfo.bit_depth = fetch_8bit(&data[8]);
			pnginfo.color_type = fetch_8bit(&data[9]);
			pnginfo.compression_method = fetch_8bit(&data[10]);
			pnginfo.filter_method = fetch_8bit(&data[11]);
			pnginfo.interlace_method = fetch_8bit(&data[12]);
			break;

		case PNG_CN_PLTE: // palette
			pnginfo.num_palette = length / 3;
			if ((length % 3) || ((3 == pnginfo.color_type) && ((1 << pnginfo.bit_depth) < pnginfo.num_palette)))
				return png_error::FILE_CORRUPT;
			pnginfo.palette = std::move(data);
			break;

		case PNG_CN_tRNS: // transparency information
			if (((0 == pnginfo.color_type) && (2 > length)) || ((2 == pnginfo.color_type) && (6 > length)))
				return png_error::FILE_CORRUPT;
			pnginfo.num_trans = length;
			pnginfo.trans = std::move(data);
			break;

		case PNG_CN_IDAT: // image data
			try { idata.emplace_back(length, std::move(data)); }
			catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }
			break;

		case PNG_CN_gAMA: // gamma
			if (4 > length)
				return png_error::FILE_CORRUPT;
			pnginfo.source_gamma = fetch_32bit(data.get()) / 100000.0;
			break;

		case PNG_CN_pHYs: // physical information
			if (9 > length)
				return png_error::FILE_CORRUPT;
			pnginfo.xres = fetch_32bit(&data[0]);
			pnginfo.yres = fetch_32bit(&data[4]);
			pnginfo.resolution_unit = fetch_8bit(&data[8]);
			break;

		case PNG_CN_tEXt: // text
			try
			{
				// split into keyword and string
				std::uint8_t const *kwbegin(&data[0]);
				std::uint8_t const *const textend(kwbegin + length);
				std::uint8_t const *const kwend(std::find(kwbegin, textend, '\0'));
				std::uint8_t const *textbegin(kwend + ((textend == kwend) ? 0 : 1));

				// text is ISO-8859-1 but MAME likes UTF-8
				std::size_t buflen(2 * std::max(kwend - kwbegin, textend - textbegin));
				std::unique_ptr<char []> utf8buf(new char [buflen]);
				char const *const bufend(utf8buf.get() + buflen);
				char *dst;
				for (dst = utf8buf.get(); kwend > kwbegin; dst += utf8_from_uchar(dst, bufend - dst, *kwbegin++)) { }
				std::string keyword(utf8buf.get(), dst);
				for (dst = utf8buf.get(); textend > textbegin; dst += utf8_from_uchar(dst, bufend - dst, *textbegin++)) { }
				std::string text(utf8buf.get(), dst);

				// allocate a new text item
				pnginfo.textlist.emplace_back(std::move(keyword), std::move(text));
			}
			catch (std::bad_alloc const &)
			{
				return png_error::OUT_OF_MEMORY;
			}
			break;

		/* anything else */
		default:
			if ((type & 0x20000000) == 0)
				return png_error::UNKNOWN_CHUNK;
			break;
		}
		return png_error::NONE;
	}

	unsigned get_pass_count() const
	{
		return (1 == pnginfo.interlace_method) ? 7 : 1;
	}

	std::pair<uint32_t, uint32_t> get_pass_dimensions(unsigned pass) const
	{
		if (0 == pnginfo.interlace_method)
			return std::make_pair(pnginfo.width, pnginfo.height);
		else
			return std::make_pair((pnginfo.width + ADAM7_X_BIAS[pass]) >> ADAM7_X_SHIFT[pass], (pnginfo.height + ADAM7_Y_BIAS[pass]) >> ADAM7_Y_SHIFT[pass]);
	}

	std::uint32_t get_pass_bytes(unsigned pass) const
	{
		return get_pass_bytes(pass, pnginfo.bit_depth);
	}

	std::uint32_t get_pass_bytes(unsigned pass, uint8_t bit_depth) const
	{
		std::pair<std::uint32_t, std::uint32_t> const dimensions(get_pass_dimensions(pass));
		return (get_row_bytes(dimensions.first, bit_depth) + 1) * dimensions.second;
	}

	std::uint32_t get_row_bytes(std::uint32_t width) const
	{
		return get_row_bytes(width, pnginfo.bit_depth);
	}

	std::uint32_t get_row_bytes(std::uint32_t width, uint8_t bit_depth) const
	{
		return ((width * samples[pnginfo.color_type] * bit_depth) + 7) >> 3;
	}

	std::uint32_t get_bytes_per_pixel() const
	{
		return ((samples[pnginfo.color_type] * pnginfo.bit_depth) + 7) >> 3;
	}

	static png_error read_chunk(core_file &fp, std::unique_ptr<std::uint8_t []> &data, std::uint32_t &type, std::uint32_t &length)
	{
		std::uint8_t tempbuff[4];

		/* fetch the length of this chunk */
		if (fp.read(tempbuff, 4) != 4)
			return png_error::FILE_TRUNCATED;
		length = fetch_32bit(tempbuff);

		/* fetch the type of this chunk */
		if (fp.read(tempbuff, 4) != 4)
			return png_error::FILE_TRUNCATED;
		type = fetch_32bit(tempbuff);

		/* stop when we hit an IEND chunk */
		if (type == PNG_CN_IEND)
			return png_error::NONE;

		/* start the CRC with the chunk type (but not the length) */
		std::uint32_t crc = crc32(0, tempbuff, 4);

		/* read the chunk itself into an allocated memory buffer */
		if (length)
		{
			/* allocate memory for this chunk */
			try { data.reset(new std::uint8_t [length]); }
			catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }

			/* read the data from the file */
			if (fp.read(data.get(), length) != length)
			{
				data.reset();
				return png_error::FILE_TRUNCATED;
			}

			/* update the CRC */
			crc = crc32(crc, data.get(), length);
		}

		/* read the CRC */
		if (fp.read(tempbuff, 4) != 4)
		{
			data.reset();
			return png_error::FILE_TRUNCATED;
		}
		std::uint32_t const chunk_crc = fetch_32bit(tempbuff);

		/* validate the CRC */
		if (crc != chunk_crc)
		{
			data.reset();
			return png_error::FILE_CORRUPT;
		}

		return png_error::NONE;
	}

	png_info &  pnginfo;

public:
	png_private(png_info &info) : pnginfo(info)
	{
	}

	png_error copy_to_bitmap(bitmap_argb32 &bitmap, bool &hasalpha) const
	{
		// do some basic checks for unsupported images
		if ((8 > pnginfo.bit_depth) || (pnginfo.bit_depth % 8))
			return png_error::UNSUPPORTED_FORMAT; // only do multiples of 8bps here - expand lower bit depth first
		if ((ARRAY_LENGTH(samples) <= pnginfo.color_type) || !samples[pnginfo.color_type])
			return png_error::UNSUPPORTED_FORMAT; // unknown colour sample format
		if ((0 != pnginfo.interlace_method) && (1 != pnginfo.interlace_method))
			return png_error::UNSUPPORTED_FORMAT; // unknown interlace method
		if ((3 == pnginfo.color_type) && (8 != pnginfo.bit_depth))
			return png_error::UNSUPPORTED_FORMAT; // indexed colour must be exactly 8bpp

		// everything looks sane, allocate the bitmap and deinterlace into it
		bitmap.allocate(pnginfo.width, pnginfo.height);
		std::uint8_t accumalpha(0xff);
		uint32_t const bps(pnginfo.bit_depth >> 3);
		uint32_t const bpp(bps * samples[pnginfo.color_type]);
		unsigned const pass_count(get_pass_count());
		std::uint32_t pass_offset[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		for (unsigned pass = 0; pass_count > pass; ++pass)
		{
			// calculate parameters for interlace pass
			pass_offset[pass + 1] = pass_offset[pass] + get_pass_bytes(pass);
			unsigned const x_shift(pnginfo.interlace_method ? ADAM7_X_SHIFT[pass] : 0);
			unsigned const y_shift(pnginfo.interlace_method ? ADAM7_Y_SHIFT[pass] : 0);
			unsigned const x_offs(pnginfo.interlace_method ? ADAM7_X_OFFS[pass] : 0);
			unsigned const y_offs(pnginfo.interlace_method ? ADAM7_Y_OFFS[pass] : 0);
			std::pair<std::uint32_t, std::uint32_t> const dimensions(get_pass_dimensions(pass));
			std::uint8_t const *src(&pnginfo.image[pass_offset[pass]]);

			if (3 == pnginfo.color_type)
			{
				// handle 8bpp palettized case
				for (std::uint32_t y = 0; dimensions.second > y; ++y)
				{
					for (std::uint32_t x = 0; dimensions.first > x; ++x, src += bpp)
					{
						// determine alpha and expand to 32bpp
						std::uint8_t const alpha((*src < pnginfo.num_trans) ? pnginfo.trans[*src] : 0xff);
						accumalpha &= alpha;
						std::uint16_t const paloffs(std::uint16_t(*src) * 3);
						rgb_t const pix(alpha, pnginfo.palette[paloffs], pnginfo.palette[paloffs + 1], pnginfo.palette[paloffs + 2]);
						bitmap.pix((y << y_shift) + y_offs, (x << x_shift) + x_offs) = pix;
					}
				}
			}
			else if (0 == pnginfo.color_type)
			{
				// handle grayscale non-alpha case
				uint32_t const bpp(pnginfo.bit_depth >> 3);
				std::uint16_t const transpen(pnginfo.trans ? fetch_16bit(pnginfo.trans.get()) : 0U);
				unsigned const samp_shift((8 < pnginfo.bit_depth) ? 8 : 0);
				for (std::uint32_t y = 0; dimensions.second > y; ++y)
				{
					for (std::uint32_t x = 0; dimensions.first > x; ++x, src += bpp)
					{
						std::uint16_t i_val((8 < pnginfo.bit_depth) ? fetch_16bit(src) : fetch_8bit(src));
						std::uint8_t const a_val((pnginfo.trans && (transpen == i_val)) ? 0x00 : 0xff);
						i_val >>= samp_shift;
						accumalpha &= a_val;
						bitmap.pix((y << y_shift) + y_offs, (x << x_shift) + x_offs) = rgb_t(a_val, i_val, i_val, i_val);
					}
				}
			}
			else if (4 == pnginfo.color_type)
			{
				// handle grayscale alpha case
				uint32_t const i(0 * bps);
				uint32_t const a(1 * bps);
				for (std::uint32_t y = 0; dimensions.second > y; ++y)
				{
					for (std::uint32_t x = 0; dimensions.first > x; ++x, src += bpp)
					{
						accumalpha &= src[a];
						rgb_t const pix(src[a], src[i], src[i], src[i]);
						bitmap.pix((y << y_shift) + y_offs, (x << x_shift) + x_offs) = pix;
					}
				}
			}
			else if (2 == pnginfo.color_type)
			{
				// handle RGB non-alpha case
				uint32_t const r(0 * bps);
				uint32_t const g(1 * bps);
				uint32_t const b(2 * bps);
				std::uint16_t const transpen_r(pnginfo.trans ? fetch_16bit(&pnginfo.trans[0]) : 0U);
				std::uint16_t const transpen_g(pnginfo.trans ? fetch_16bit(&pnginfo.trans[2]) : 0U);
				std::uint16_t const transpen_b(pnginfo.trans ? fetch_16bit(&pnginfo.trans[4]) : 0U);
				unsigned const samp_shift((8 < pnginfo.bit_depth) ? 8 : 0);
				for (std::uint32_t y = 0; dimensions.second > y; ++y)
				{
					for (std::uint32_t x = 0; dimensions.first > x; ++x, src += bpp)
					{
						uint16_t r_val((8 < pnginfo.bit_depth) ? fetch_16bit(src) : fetch_8bit(src + r));
						uint16_t g_val((8 < pnginfo.bit_depth) ? fetch_16bit(src) : fetch_8bit(src + g));
						uint16_t b_val((8 < pnginfo.bit_depth) ? fetch_16bit(src) : fetch_8bit(src + b));
						std::uint8_t const a_val((pnginfo.trans && (transpen_r == r_val) && (transpen_g == g_val) && (transpen_b == b_val)) ? 0x00 : 0xff);
						r_val >>= samp_shift;
						g_val >>= samp_shift;
						b_val >>= samp_shift;
						accumalpha &= a_val;
						bitmap.pix((y << y_shift) + y_offs, (x << x_shift) + x_offs) = rgb_t(a_val, r_val, g_val, b_val);
					}
				}
			}
			else
			{
				// handle RGB alpha case
				uint32_t const r(0 * bps);
				uint32_t const g(1 * bps);
				uint32_t const b(2 * bps);
				uint32_t const a(3 * bps);
				for (std::uint32_t y = 0; dimensions.second > y; ++y)
				{
					for (std::uint32_t x = 0; dimensions.first > x; ++x, src += bpp)
					{
						accumalpha &= src[a];
						rgb_t const pix(src[a], src[r], src[g], src[b]);
						bitmap.pix((y << y_shift) + y_offs, (x << x_shift) + x_offs) = pix;
					}
				}
			}
		}

		// set hasalpha flag and return
		hasalpha = 0xffU != accumalpha;
		return png_error::NONE;
	}

	png_error expand_buffer_8bit()
	{
		// nothing to do if we're at 8 or greater already
		if (pnginfo.bit_depth >= 8)
			return png_error::NONE;

		// do some basic checks for unsupported images
		if (!pnginfo.bit_depth || (8 % pnginfo.bit_depth))
			return png_error::UNSUPPORTED_FORMAT; // bit depth must be a factor of eight
		if ((0 != pnginfo.color_type) && (3 != pnginfo.color_type))
			return png_error::UNSUPPORTED_FORMAT; // only upsample monochrome and indexed colour
		if ((0 != pnginfo.interlace_method) && (1 != pnginfo.interlace_method))
			return png_error::UNSUPPORTED_FORMAT; // unknown interlace method

		// calculate the offset for each pass of the interlace on the input and output
		unsigned const pass_count(get_pass_count());
		std::uint32_t inp_offset[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		std::uint32_t outp_offset[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		for (unsigned pass = 0; pass_count > pass; ++pass)
		{
			inp_offset[pass + 1] = inp_offset[pass] + get_pass_bytes(pass);
			outp_offset[pass + 1] = outp_offset[pass] + get_pass_bytes(pass, 8);
		}

		// allocate a new buffer at 8-bit
		std::unique_ptr<std::uint8_t []> outbuf;
		try { outbuf.reset(new std::uint8_t [outp_offset[pass_count]]); }
		catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }

		// upsample bitmap
		std::uint8_t const bytesamples(8 / pnginfo.bit_depth);
		for (unsigned pass = 0; pass_count > pass; ++pass)
		{
			std::pair<std::uint32_t, std::uint32_t> const dimensions(get_pass_dimensions(pass));
			std::uint32_t const rowsamples(samples[pnginfo.color_type] * dimensions.first);
			std::uint32_t const wholebytes(rowsamples / bytesamples);
			std::uint32_t const leftover(rowsamples % bytesamples);
			std::uint8_t const *inp(&pnginfo.image[inp_offset[pass]]);
			std::uint8_t *outp(&outbuf[outp_offset[pass]]);

			for (std::uint32_t y = 0; dimensions.second > y; ++y)
			{
				for (std::uint32_t i = 0; wholebytes > i; ++i, ++inp)
				{
					for (std::int8_t j = bytesamples - 1; 0 <= j; --j, ++outp)
					{
						*outp = (*inp >> (j * pnginfo.bit_depth)) & (0xffU >> (8 - pnginfo.bit_depth));
						if (!pnginfo.color_type)
						{
							for (unsigned k = 4; pnginfo.bit_depth <= k; k >>= 1)
								*outp |= *outp << k;
						}
					}
				}
				if (leftover)
				{
					for (std::int8_t j = leftover - 1; 0 <= j; --j,++outp)
					{
						*outp = (*inp >> (j * pnginfo.bit_depth)) & (0xffU >> (8 - pnginfo.bit_depth));
						if (!pnginfo.color_type)
						{
							for (unsigned k = 4; pnginfo.bit_depth <= k; k >>= 1)
								*outp |= *outp << k;
						}
					}
					inp++;
				}
			}
		}

		// upsample transparent pen as well
		if ((0 == pnginfo.color_type) && pnginfo.trans)
		{
			std::uint16_t pen(fetch_16bit(&pnginfo.trans[0]));
			for (unsigned k = 4; pnginfo.bit_depth <= k; k >>= 1)
				pen |= pen << k;
			put_16bit(&pnginfo.trans[0], pen);
		}

		pnginfo.image = std::move(outbuf);
		pnginfo.bit_depth = 8;
		return png_error::NONE;
	}

	png_error read_file(core_file &fp)
	{
		// initialize the data structures
		png_error error = png_error::NONE;
		pnginfo.reset();
		std::list<image_data_chunk> idata;

		// verify the signature at the start of the file
		error = verify_header(fp);

		// loop until we hit an IEND chunk
		while (png_error::NONE == error)
		{
			// read a chunk
			std::unique_ptr<std::uint8_t []> chunk_data;
			std::uint32_t chunk_type = 0, chunk_length;
			error = read_chunk(fp, chunk_data, chunk_type, chunk_length);
			if (png_error::NONE == error)
			{
				if (chunk_type == PNG_CN_IEND)
					break; // stop when we hit an IEND chunk
				else
					error = process_chunk(idata, std::move(chunk_data), chunk_type, chunk_length);
			}
		}

		// finish processing the image
		if (png_error::NONE == error)
			error = process(idata);

		// if we have an error, free all the output data
		if (error != png_error::NONE)
			pnginfo.reset();

		return error;
	}

	static png_error verify_header(core_file &fp)
	{
		EQUIVALENT_ARRAY(PNG_SIGNATURE, std::uint8_t) signature;

		// read 8 bytes
		if (fp.read(signature, sizeof(signature)) != sizeof(signature))
			return png_error::FILE_TRUNCATED;

		// return an error if we don't match
		if (std::memcmp(signature, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)))
			return png_error::BAD_SIGNATURE;

		return png_error::NONE;
	}
};

constexpr unsigned png_private::ADAM7_X_BIAS[7];
constexpr unsigned png_private::ADAM7_Y_BIAS[7];
constexpr unsigned png_private::ADAM7_X_SHIFT[7];
constexpr unsigned png_private::ADAM7_Y_SHIFT[7];
constexpr unsigned png_private::ADAM7_X_OFFS[7];
constexpr unsigned png_private::ADAM7_Y_OFFS[7];

} // anonymous namespace




/*-------------------------------------------------
    verify_header - verify PNG file header from a
    core stream
-------------------------------------------------*/

png_error png_info::verify_header(core_file &fp)
{
	return png_private::verify_header(fp);
}


/*-------------------------------------------------
    read_file - read a PNG from a core stream
-------------------------------------------------*/

png_error png_info::read_file(core_file &fp)
{
	return png_private(*this).read_file(fp);
}


/*-------------------------------------------------
    png_read_bitmap - load a PNG file into a
    bitmap
-------------------------------------------------*/

png_error png_read_bitmap(core_file &fp, bitmap_argb32 &bitmap)
{
	png_error result;
	png_info pnginfo;
	png_private png(pnginfo);

	// read the PNG data
	result = png.read_file(fp);
	if (png_error::NONE != result)
		return result;

	// resample to 8bpp if necessary
	result = png.expand_buffer_8bit();
	if (png_error::NONE != result)
	{
		pnginfo.free_data();
		return result;
	}

	// allocate a bitmap of the appropriate size and copy it
	bool hasalpha;
	return png.copy_to_bitmap(bitmap, hasalpha);
}


/*-------------------------------------------------
    expand_buffer_8bit - copy PNG data into a
    bitmap
-------------------------------------------------*/

png_error png_info::copy_to_bitmap(bitmap_argb32 &bitmap, bool &hasalpha)
{
	return png_private(*this).copy_to_bitmap(bitmap, hasalpha);
}


/*-------------------------------------------------
    expand_buffer_8bit - expand a buffer from
    sub 8-bit to 8-bit
-------------------------------------------------*/

png_error png_info::expand_buffer_8bit()
{
	return png_private(*this).expand_buffer_8bit();
}



/***************************************************************************
    PNG WRITING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    add_text - add a text entry to the png_info
-------------------------------------------------*/

png_error png_info::add_text(std::string_view keyword, std::string_view text)
{
	// apply rules to keyword
	char32_t prev(0);
	std::size_t cnt(0);
	for (std::string_view kw = keyword; !kw.empty(); )
	{
		char32_t ch;
		int const len(uchar_from_utf8(&ch, kw));
		if ((0 >= len) || (32 > ch) || (255 < ch) || ((126 < ch) && (161 > ch)) || (((32 == prev) || (0 == cnt)) && (32 == ch)))
			return png_error::UNSUPPORTED_FORMAT;
		prev = ch;
		++cnt;
		kw.remove_prefix(len);
	}
	if ((32 == prev) || (1 > cnt) || (79 < cnt))
		return png_error::UNSUPPORTED_FORMAT;

	// apply rules to text
	for (std::string_view tx = text; !tx.empty(); )
	{
		char32_t ch;
		int const len(uchar_from_utf8(&ch, tx));
		if ((0 >= len) || (1 > ch) || (255 < ch))
			return png_error::UNSUPPORTED_FORMAT;
		tx.remove_prefix(len);
	}

	// allocate a new text element
	try { textlist.emplace_back(std::piecewise_construct, std::forward_as_tuple(keyword), std::forward_as_tuple(text)); }
	catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }
	return png_error::NONE;
}


/*-------------------------------------------------
    write_chunk - write an in-memory chunk to
    the given file
-------------------------------------------------*/

static png_error write_chunk(core_file &fp, const uint8_t *data, uint32_t type, uint32_t length)
{
	uint8_t tempbuff[8];
	uint32_t crc;

	/* stuff the length/type into the buffer */
	put_32bit(tempbuff + 0, length);
	put_32bit(tempbuff + 4, type);
	crc = crc32(0, tempbuff + 4, 4);

	/* write that data */
	if (fp.write(tempbuff, 8) != 8)
		return png_error::FILE_ERROR;

	/* append the actual data */
	if (length > 0)
	{
		if (fp.write(data, length) != length)
			return png_error::FILE_ERROR;
		crc = crc32(crc, data, length);
	}

	/* write the CRC */
	put_32bit(tempbuff, crc);
	if (fp.write(tempbuff, 4) != 4)
		return png_error::FILE_ERROR;

	return png_error::NONE;
}


/*-------------------------------------------------
    write_deflated_chunk - write an in-memory
    chunk to the given file by deflating it
-------------------------------------------------*/

static png_error write_deflated_chunk(core_file &fp, uint8_t *data, uint32_t type, uint32_t length)
{
	uint64_t lengthpos = fp.tell();
	uint8_t tempbuff[8192];
	uint32_t zlength = 0;
	z_stream stream;
	uint32_t crc;
	int zerr;

	/* stuff the length/type into the buffer */
	put_32bit(tempbuff + 0, length);
	put_32bit(tempbuff + 4, type);
	crc = crc32(0, tempbuff + 4, 4);

	/* write that data */
	if (fp.write(tempbuff, 8) != 8)
		return png_error::FILE_ERROR;

	/* initialize the stream */
	memset(&stream, 0, sizeof(stream));
	stream.next_in = data;
	stream.avail_in = length;
	zerr = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	if (zerr != Z_OK)
		return png_error::COMPRESS_ERROR;

	/* now loop until we run out of data */
	for ( ; ; )
	{
		/* compress this chunk */
		stream.next_out = tempbuff;
		stream.avail_out = sizeof(tempbuff);
		zerr = deflate(&stream, Z_FINISH);

		/* if there's data to write, do it */
		if (stream.avail_out < sizeof(tempbuff))
		{
			int bytes = sizeof(tempbuff) - stream.avail_out;
			if (fp.write(tempbuff, bytes) != bytes)
			{
				deflateEnd(&stream);
				return png_error::FILE_ERROR;
			}
			crc = crc32(crc, tempbuff, bytes);
			zlength += bytes;
		}

		/* stop at the end of the stream */
		if (zerr == Z_STREAM_END)
			break;

		/* other errors are fatal */
		if (zerr != Z_OK)
		{
			deflateEnd(&stream);
			return png_error::COMPRESS_ERROR;
		}
	}

	/* clean up deflater(maus) */
	zerr = deflateEnd(&stream);
	if (zerr != Z_OK)
		return png_error::COMPRESS_ERROR;

	/* write the CRC */
	put_32bit(tempbuff, crc);
	if (fp.write(tempbuff, 4) != 4)
		return png_error::FILE_ERROR;

	/* seek back and update the length */
	fp.seek(lengthpos, SEEK_SET);
	put_32bit(tempbuff + 0, zlength);
	if (fp.write(tempbuff, 4) != 4)
		return png_error::FILE_ERROR;

	/* return to the end */
	fp.seek(lengthpos + 8 + zlength + 4, SEEK_SET);
	return png_error::NONE;
}


/*-------------------------------------------------
    convert_bitmap_to_image_palette - convert a
    bitmap to a palettized image
-------------------------------------------------*/

static png_error convert_bitmap_to_image_palette(png_info &pnginfo, const bitmap_t &bitmap, int palette_length, const rgb_t *palette)
{
	int rowbytes;
	int x, y;

	/* set the common info */
	pnginfo.width = bitmap.width();
	pnginfo.height = bitmap.height();
	pnginfo.bit_depth = 8;
	pnginfo.color_type = 3;
	pnginfo.num_palette = 256;
	rowbytes = pnginfo.width;

	/* allocate memory for the palette */
	try { pnginfo.palette.reset(new std::uint8_t [3 * 256]); }
	catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }

	/* build the palette */
	std::fill_n(pnginfo.palette.get(), 3 * 256, 0);
	for (x = 0; x < palette_length; x++)
	{
		rgb_t color = palette[x];
		pnginfo.palette[3 * x + 0] = color.r();
		pnginfo.palette[3 * x + 1] = color.g();
		pnginfo.palette[3 * x + 2] = color.b();
	}

	/* allocate memory for the image */
	try
	{
		pnginfo.image.reset(new std::uint8_t [pnginfo.height * (rowbytes + 1)]);
	}
	catch (std::bad_alloc const &)
	{
		pnginfo.palette.reset();
		return png_error::OUT_OF_MEMORY;
	}

	/* copy in the pixels, specifying a nullptr filter */
	for (y = 0; y < pnginfo.height; y++)
	{
		uint16_t const *src = reinterpret_cast<uint16_t const *>(bitmap.raw_pixptr(y));
		uint8_t *dst = &pnginfo.image[y * (rowbytes + 1)];

		/* store the filter byte, then copy the data */
		*dst++ = 0;
		for (x = 0; x < pnginfo.width; x++)
			*dst++ = *src++;
	}

	return png_error::NONE;
}


/*-------------------------------------------------
    convert_bitmap_to_image_rgb - convert a
    bitmap to an RGB image
-------------------------------------------------*/

static png_error convert_bitmap_to_image_rgb(png_info &pnginfo, const bitmap_t &bitmap, int palette_length, const rgb_t *palette)
{
	int alpha = (bitmap.format() == BITMAP_FORMAT_ARGB32);
	int rowbytes;
	int x, y;

	/* set the common info */
	pnginfo.width = bitmap.width();
	pnginfo.height = bitmap.height();
	pnginfo.bit_depth = 8;
	pnginfo.color_type = alpha ? 6 : 2;
	rowbytes = pnginfo.width * (alpha ? 4 : 3);

	/* allocate memory for the image */
	try { pnginfo.image.reset(new std::uint8_t [pnginfo.height * (rowbytes + 1)]); }
	catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }

	/* copy in the pixels, specifying a nullptr filter */
	for (y = 0; y < pnginfo.height; y++)
	{
		uint8_t *dst = &pnginfo.image[y * (rowbytes + 1)];

		/* store the filter byte, then copy the data */
		*dst++ = 0;

		/* 16bpp palettized format */
		if (bitmap.format() == BITMAP_FORMAT_IND16)
		{
			uint16_t const *src16 = reinterpret_cast<uint16_t const *>(bitmap.raw_pixptr(y));
			for (x = 0; x < pnginfo.width; x++)
			{
				rgb_t color = palette[*src16++];
				*dst++ = color.r();
				*dst++ = color.g();
				*dst++ = color.b();
			}
		}

		/* 32-bit RGB direct */
		else if (bitmap.format() == BITMAP_FORMAT_RGB32)
		{
			uint32_t const *src32 = reinterpret_cast<uint32_t const *>(bitmap.raw_pixptr(y));
			for (x = 0; x < pnginfo.width; x++)
			{
				rgb_t raw = *src32++;
				*dst++ = raw.r();
				*dst++ = raw.g();
				*dst++ = raw.b();
			}
		}

		/* 32-bit ARGB direct */
		else if (bitmap.format() == BITMAP_FORMAT_ARGB32)
		{
			uint32_t const *src32 = reinterpret_cast<uint32_t const *>(bitmap.raw_pixptr(y));
			for (x = 0; x < pnginfo.width; x++)
			{
				rgb_t raw = *src32++;
				*dst++ = raw.r();
				*dst++ = raw.g();
				*dst++ = raw.b();
				*dst++ = raw.a();
			}
		}

		/* unsupported format */
		else
			return png_error::UNSUPPORTED_FORMAT;
	}

	return png_error::NONE;
}


/*-------------------------------------------------
    write_png_stream - stream a series of PNG
    chunks to the given file
-------------------------------------------------*/

static png_error write_png_stream(core_file &fp, png_info &pnginfo, const bitmap_t &bitmap, int palette_length, const rgb_t *palette)
{
	uint8_t tempbuff[16];
	png_error error;

	// create an unfiltered image in either palette or RGB form
	if (bitmap.format() == BITMAP_FORMAT_IND16 && palette_length <= 256)
		error = convert_bitmap_to_image_palette(pnginfo, bitmap, palette_length, palette);
	else
		error = convert_bitmap_to_image_rgb(pnginfo, bitmap, palette_length, palette);
	if (error != png_error::NONE)
		return error;

	// if we wanted to get clever and do filtering, we would do it here

	// write the IHDR chunk
	put_32bit(tempbuff + 0, pnginfo.width);
	put_32bit(tempbuff + 4, pnginfo.height);
	put_8bit(tempbuff + 8, pnginfo.bit_depth);
	put_8bit(tempbuff + 9, pnginfo.color_type);
	put_8bit(tempbuff + 10, pnginfo.compression_method);
	put_8bit(tempbuff + 11, pnginfo.filter_method);
	put_8bit(tempbuff + 12, pnginfo.interlace_method);
	error = write_chunk(fp, tempbuff, PNG_CN_IHDR, 13);
	if (error != png_error::NONE)
		return error;

	// write the PLTE chunk
	if (pnginfo.num_palette > 0)
		error = write_chunk(fp, pnginfo.palette.get(), PNG_CN_PLTE, pnginfo.num_palette * 3);
	if (error != png_error::NONE)
		return error;

	// write a single IDAT chunk
	error = write_deflated_chunk(fp, pnginfo.image.get(), PNG_CN_IDAT, pnginfo.height * (compute_rowbytes(pnginfo) + 1));
	if (error != png_error::NONE)
		return error;

	// write TEXT chunks
	std::vector<std::uint8_t> textbuf;
	for (png_info::png_text const &text : pnginfo.textlist)
	{
		try { textbuf.resize(text.first.length() + 1 + text.second.length()); }
		catch (std::bad_alloc const &) { return png_error::OUT_OF_MEMORY; }
		std::uint8_t *dst(&textbuf[0]);

		// convert keyword to ISO-8859-1
		for (std::string_view src = text.first; !src.empty(); ++dst)
		{
			char32_t ch;
			int const len(uchar_from_utf8(&ch, src));
			if (0 >= len)
				break;
			*dst = std::uint8_t(ch);
			src.remove_prefix(len);
		}

		// NUL separator between keword and text
		*dst++ = 0;

		// convert text to ISO-8859-1
		for (std::string_view src = text.second; !src.empty(); ++dst)
		{
			char32_t ch;
			int const len(uchar_from_utf8(&ch, src));
			if (0 >= len)
				break;
			*dst = std::uint8_t(ch);
			src.remove_prefix(len);
		}

		error = write_chunk(fp, &textbuf[0], PNG_CN_tEXt, dst - &textbuf[0]);
		if (error != png_error::NONE)
			return error;
	}

	// write an IEND chunk
	return write_chunk(fp, nullptr, PNG_CN_IEND, 0);
}


png_error png_write_bitmap(core_file &fp, png_info *info, bitmap_t const &bitmap, int palette_length, const rgb_t *palette)
{
	// use a dummy pnginfo if none passed to us
	png_info pnginfo;
	if (info == nullptr)
		info = &pnginfo;

	// write the PNG signature
	if (fp.write(PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != sizeof(PNG_SIGNATURE))
		return png_error::FILE_ERROR;

	/* write the rest of the PNG data */
	return write_png_stream(fp, *info, bitmap, palette_length, palette);
}



/********************************************************************************

  MNG write functions

********************************************************************************/

/**
 * @fn  png_error mng_capture_start(core_file &fp, bitmap_t &bitmap, unsigned rate)
 *
 * @brief   Mng capture start.
 *
 * @param [in,out]  fp      If non-null, the fp.
 * @param [in,out]  bitmap  The bitmap.
 * @param   rate            The framerate.
 *
 * @return  A png_error.
 */

png_error mng_capture_start(core_file &fp, bitmap_t &bitmap, unsigned rate)
{
	uint8_t mhdr[28];

	if (fp.write(MNG_Signature, 8) != 8)
		return png_error::FILE_ERROR;

	memset(mhdr, 0, 28);
	put_32bit(mhdr + 0, bitmap.width());
	put_32bit(mhdr + 4, bitmap.height());
	put_32bit(mhdr + 8, rate);
	put_32bit(mhdr + 24, 0x0041); // Simplicity profile - frame count and play time unspecified because we don't know at this stage
	return write_chunk(fp, mhdr, MNG_CN_MHDR, 28);
}

/**
 * @fn  png_error mng_capture_frame(core_file &fp, png_info *info, bitmap_t &bitmap, int palette_length, const rgb_t *palette)
 *
 * @brief   Mng capture frame.
 *
 * @param [in,out]  fp      If non-null, the fp.
 * @param [in,out]  info    If non-null, the information.
 * @param [in,out]  bitmap  The bitmap.
 * @param   palette_length  Length of the palette.
 * @param   palette         The palette.
 *
 * @return  A png_error.
 */

png_error mng_capture_frame(core_file &fp, png_info &info, bitmap_t const &bitmap, int palette_length, const rgb_t *palette)
{
	return write_png_stream(fp, info, bitmap, palette_length, palette);
}

/**
 * @fn  png_error mng_capture_stop(core_file &fp)
 *
 * @brief   Mng capture stop.
 *
 * @param [in,out]  fp  If non-null, the fp.
 *
 * @return  A png_error.
 */

png_error mng_capture_stop(core_file &fp)
{
	return write_chunk(fp, nullptr, MNG_CN_MEND, 0);
}

} // namespace util
