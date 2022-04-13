// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    formats/apd_dsk.c

    Archimedes Protected Disk Image format

    APD file structure
    ------------------

    The APD file is a GZip compressed version of the original APD file.

    Compressed file always starts:
    1F 8B 08 00 00 00 00 00 00 0B EC BD
    ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^
    |  |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |  +- OS
    |  |  |  |  |  |  |  |  +---- xfl
    |  |  |  |  +--+--+--+------- time
    |  |  |  +------------------- gzip flags
    |  |  +---------------------- gzip compression*
    |  |
    +--+------------------------- gzip header

    *  Compression method: 8 is the only supported format

    Original APD file structure:
              0 - 7    "APDX0001" identifier
              8 - B    t0sd - Track 0 SD length in bits
              C - F    t0dd - Track 0 DD length in bits
             10 - 13   t0qd - Track 0 QD length in bits
             14 - 1F   t1sd - Track 1 SD length in bits
             20 - 23   t1dd - Track 1 DD length in bits
             24 - 27   t1qd - Track 1 QD length in bits
            ...   ...  repeated to Track 159
            77C - 787  Track 160 (blank)
            7C4 - 7CF  Track 166 (blank)

                  7D0  Track 0 SD data
    + (t0sd + 7) >> 3  Track 0 DD data
    + (t0dd + 7) >> 3  Track 0 QD data

    + (t0qd + 7) >> 3  Track 1 SD data
    + (t1sd + 7) >> 3  Track 1 DD data
    + (t1dd + 7) >> 3  Track 1 QD data

    SD    data is big-endian raw FM words
    DD/QD data is big-endian raw MFM words

   As far as I can tell, the tracks are always sequential, so
    physical tracks translate as:

    Physical         APD
    --------------   -------
    Side 0 Track 0 > Track 0
    Side 1 Track 0 > Track 1
    Side 0 Track 1 > Track 2
    etc.

*********************************************************************/

#include "formats/apd_dsk.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*, little_endianize_int32

#include <zlib.h>

#include <cstring>


static const uint8_t APD_HEADER[8] = { 'A', 'P', 'D', 'X', '0', '0', '0', '1' };
static const uint8_t GZ_HEADER[2] = { 0x1f, 0x8b };

apd_format::apd_format()
{
}

const char *apd_format::name() const
{
	return "apd";
}

const char *apd_format::description() const
{
	return "Archimedes Protected Disk Image";
}

const char *apd_format::extensions() const
{
	return "apd";
}

int apd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size) || !size)
		return 0;

	std::vector<uint8_t> img(size);
	size_t actual;
	io.read_at(0, &img[0], size, actual);

	int err;
	std::vector<uint8_t> gz_ptr(8);
	z_stream d_stream;

	if (!memcmp(&img[0], GZ_HEADER, sizeof(GZ_HEADER))) {
		d_stream.zalloc = nullptr;
		d_stream.zfree = nullptr;
		d_stream.opaque = nullptr;
		d_stream.next_in = &img[0];
		d_stream.avail_in = size;
		d_stream.next_out = &gz_ptr[0];
		d_stream.avail_out = 8;

		err = inflateInit2(&d_stream, MAX_WBITS | 16);
		if (err != Z_OK) return 0;

		err = inflate(&d_stream, Z_SYNC_FLUSH);
		if (err != Z_OK) return 0;

		err = inflateEnd(&d_stream);
		if (err != Z_OK) return 0;

		img = gz_ptr;
	}

	if (!memcmp(&img[0], APD_HEADER, sizeof(APD_HEADER))) {
		return FIFID_SIGN;
	}

	return 0;
}

bool apd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	std::vector<uint8_t> img(size);
	size_t actual;
	io.read_at(0, &img[0], size, actual);

	int err;
	std::vector<uint8_t> gz_ptr;
	z_stream d_stream;
	int inflate_size = (img[size - 1] << 24) | (img[size - 2] << 16) | (img[size - 3] << 8) | img[size - 4];
	uint8_t *in_ptr = &img[0];

	if (!memcmp(&img[0], GZ_HEADER, sizeof(GZ_HEADER))) {
		gz_ptr.resize(inflate_size);

		d_stream.zalloc = nullptr;
		d_stream.zfree = nullptr;
		d_stream.opaque = nullptr;
		d_stream.next_in = in_ptr;
		d_stream.avail_in = size;
		d_stream.next_out = &gz_ptr[0];
		d_stream.avail_out = inflate_size;

		err = inflateInit2(&d_stream, MAX_WBITS | 16);
		if (err != Z_OK) {
			osd_printf_error("inflateInit2 error: %d\n", err);
			return false;
		}
		err = inflate(&d_stream, Z_FINISH);
		if (err != Z_STREAM_END && err != Z_OK) {
			osd_printf_error("inflate error: %d\n", err);
			return false;
		}
		err = inflateEnd(&d_stream);
		if (err != Z_OK) {
			osd_printf_error("inflateEnd error: %d\n", err);
			return false;
		}
		size = inflate_size;
		img = gz_ptr;
	}

	int data = 0x7d0;
	for (int track = 0; track < 166; track++) {
		uint32_t sdlen = little_endianize_int32(*(uint32_t *)(&img[(track * 12) + 8 + 0x0]));
		uint32_t ddlen = little_endianize_int32(*(uint32_t *)(&img[(track * 12) + 8 + 0x4]));
		uint32_t qdlen = little_endianize_int32(*(uint32_t *)(&img[(track * 12) + 8 + 0x8]));

		if (sdlen > 0) {
			generate_track_from_bitstream(track / 2, track % 2, &img[data], sdlen, image);
			data += (sdlen + 7) >> 3;
		}
		if (ddlen > 0) {
			generate_track_from_bitstream(track / 2, track % 2, &img[data], ddlen, image);
			data += (ddlen + 7) >> 3;
		}
		if (qdlen > 0) {
			generate_track_from_bitstream(track / 2, track % 2, &img[data], qdlen, image);
			data += (qdlen + 7) >> 3;
		}
	}
	image->set_variant(floppy_image::DSDD);

	return true;
}

bool apd_format::supports_save() const
{
	return false;
}

const apd_format FLOPPY_APD_FORMAT;
