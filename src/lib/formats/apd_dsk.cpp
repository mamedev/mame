// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    formats/apd_dsk.c

    Archimedes Protected Disk Image format

*********************************************************************/

#include <zlib.h>
#include "formats/apd_dsk.h"

static const uint8_t APD_HEADER[8] = { 0x41, 0x50, 0x44, 0x58, 0x30, 0x30, 0x30, 0x31 };
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

int apd_format::identify(io_generic *io, uint32_t form_factor)
{
	char h[8];

	io_generic_read(io, h, 0, 8);
	if (!memcmp(h, GZ_HEADER, sizeof(GZ_HEADER)) || !memcmp(h, APD_HEADER, sizeof(APD_HEADER))) {
		return 100;
	}

	return 0;
}

bool apd_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	uint64_t size = io_generic_size(io);
	std::vector<uint8_t> img(size);
	io_generic_read(io, &img[0], 0, size);

	int err;
	std::vector<uint8_t> gz_ptr;
	z_stream d_stream;
	int inflate_size = (img[size - 1] << 24) | (img[size - 2] << 16) | (img[size - 3] << 8) | img[size - 4];
	uint8_t *in_ptr = &img[0];

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
		LOG_FORMATS("inflateInit2 error: %d\n", err);
		return false;
	}
	err = inflate(&d_stream, Z_FINISH);
	if (err != Z_STREAM_END && err != Z_OK) {
		LOG_FORMATS("inflate error: %d\n", err);
		return false;
	}
	err = inflateEnd(&d_stream);
	if (err != Z_OK) {
		LOG_FORMATS("inflateEnd error: %d\n", err);
		return false;
	}
	size = inflate_size;
	img = gz_ptr;

	if (memcmp(&img[0], APD_HEADER, sizeof(APD_HEADER))) {
		LOG_FORMATS("apd_dsk: floppy image has incorrect header\n");
		return false;
	}

	int data = 0x7d0;
	for (int track = 0; track < 83; track++) {
		for (int side = 0; side < 2; side++) {
			uint32_t sdlen = little_endianize_int32(*(uint32_t *)(&img[(track * 2 + side) * 12 + 8 + 0x0]));
			uint32_t ddlen = little_endianize_int32(*(uint32_t *)(&img[(track * 2 + side) * 12 + 8 + 0x4]));
			uint32_t qdlen = little_endianize_int32(*(uint32_t *)(&img[(track * 2 + side) * 12 + 8 + 0x8]));

			if (sdlen > 0) {
				generate_track_from_bitstream(track, side, &img[data], sdlen, image);
				data += (sdlen + 7) >> 3;
			}
			if (ddlen > 0) {
				generate_track_from_bitstream(track, side, &img[data], ddlen, image);
				data += (ddlen + 7) >> 3;
			}
			if (qdlen > 0) {
				generate_track_from_bitstream(track, side, &img[data], qdlen, image);
				data += (qdlen + 7) >> 3;
			}
		}
	}
	image->set_variant(floppy_image::DSDD);

	return true;
}

bool apd_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_APD_FORMAT = &floppy_image_format_creator<apd_format>;
