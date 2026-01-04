// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*
  CSW format
  ----------
  Header Description

  Offset  Value   Type    Description
  0x00    (note)  ASCII   22 bytes "Compressed Square Wave" signature
  0x16    0x1A    BYTE    Terminator code
  0x17    0x02    BYTE    CSW major revision number
  0x18    0x00    BYTE    CSW minor revision number
  0x19            DWORD   Sample rate
  0x1D            DWORD   Total number of pulses (after decompression)
  0x21            BYTE    Compression type  0x01: RLE    0x02: Z-RLE
  0x22            BYTE    Flags   b0: initial polarity; if set, the signal starts at logical high
  0x23    HDR     BYTE    Header extension length in bytes (0x00)
  0x24            ASCII   Encoding application description
  0x34            BYTE    Header extension data (if present HDR>0)
  0x34+HDR                CSW data
*/

#include "csw_cas.h"
#include "imageutl.h"
#include "uef_cas.h"

#include "multibyte.h"

#include <zlib.h>

#include <cstring>


static const uint8_t CSW_HEADER[] = { "Compressed Square Wave" };


/*-------------------------------------------------
    csw_cassette_identify - identify cassette
-------------------------------------------------*/

static cassette_image::error csw_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	uint8_t header[0x34];

	cassette->image_read(header, 0, sizeof(header));
	if (memcmp(&header[0], CSW_HEADER, sizeof(CSW_HEADER) - 1)) {
		return cassette_image::error::INVALID_IMAGE;
	}

	opts->bits_per_sample = 8;
	opts->channels = 1;
	opts->sample_frequency = header[0x17] == 1 ? get_u16le(header + 0x19) : get_u32le(header + 0x19);
	return cassette_image::error::SUCCESS;
}


/*-------------------------------------------------
    csw_cassette_load - load cassette
-------------------------------------------------*/

static cassette_image::error csw_cassette_load(cassette_image *cassette)
{
	uint8_t  header[0x34];
	uint64_t image_size = cassette->image_size();
	std::vector<uint8_t> image_data(image_size);

	int8_t bit;
	uint8_t compression;
	int bsize = 0;
	size_t csw_data = 0;
	size_t sample_count = 0;
	uint32_t sample_rate;
	std::vector<int8_t> samples;

	/* csw header */
	cassette->image_read(header, 0, sizeof(header));

	if (header[0x16] != 0x1a)
	{
		LOG_FORMATS("csw_cassette_load: Terminator Code Not Found\n");
		return cassette_image::error::INVALID_IMAGE;
	}

	LOG_FORMATS("CSW Version %d.%d\n", header[0x17], header[0x18]);
	switch (header[0x17])
	{
	case 1:
		sample_rate = get_u16le(header + 0x19);
		compression = header[0x1b];
		bit = (header[0x1c] & 1) ? 127 : -128;
		csw_data = 0x20;

		LOG_FORMATS("Sample Rate: %u\n", sample_rate);
		LOG_FORMATS("CompressionType: %u   Flags: %u\n", header[0x1b], header[0x1c]);
		break;

	case 2:
		sample_rate = get_u32le(header + 0x19);
		compression = header[0x21];
		bit = (header[0x22] & 1) ? 127 : -128;
		csw_data = (size_t) header[0x23] + 0x34;

		LOG_FORMATS("Sample Rate: %u\n", sample_rate);
		LOG_FORMATS("Number of Pulses: %u\n", get_u32le(header + 0x1d));
		LOG_FORMATS("CompressionType: %u   Flags: %u\n", header[0x21], header[0x22]);
		LOG_FORMATS("Encoder: ");
		for (int i = 0; i < 16; i++)
			LOG_FORMATS("%c", header[0x24 + i]);
		LOG_FORMATS("\n");
		break;

	default:
		LOG_FORMATS("Unsupported Major Version\n");
		return cassette_image::error::INVALID_IMAGE;
	}

	/* csw data */
	switch (compression)
	{
	case 0x01:
		/* RLE (Run Length Encoding) */
		for (size_t pos = csw_data; pos < image_size; pos++)
		{
			bsize = image_data[pos];
			if (bsize == 0)
			{
				bsize = get_u32le(&image_data[pos + 1]);
				pos += 4;
			}
			for (int i = 0; i < bsize; i++)
			{
				samples.resize(sample_count + 1);
				samples[sample_count++] = bit;
			}
			bit ^= 0xff;
		}
		break;

	case 0x02:
		/* Z-RLE (CSW v2.xx only) */
		cassette->image_read(&image_data[0], 0, image_size);

		std::vector<uint8_t> gz_ptr;
		z_stream    d_stream;
		int         err;

		gz_ptr.resize(8);

		d_stream.next_in = (unsigned char *) &image_data[csw_data];
		d_stream.avail_in = image_size - 0x34 - header[0x23];
		d_stream.total_in = 0;

		d_stream.next_out = &gz_ptr[0];
		d_stream.avail_out = 1;
		d_stream.total_out = 0;

		d_stream.zalloc = nullptr;
		d_stream.zfree = nullptr;
		d_stream.opaque = nullptr;
		d_stream.data_type = 0;

		err = inflateInit(&d_stream);
		if (err != Z_OK)
		{
			LOG_FORMATS("inflateInit error: %d\n", err);
			return cassette_image::error::INVALID_IMAGE;
		}

		do
		{
			d_stream.next_out = &gz_ptr[0];
			d_stream.avail_out = 1;
			err = inflate(&d_stream, Z_SYNC_FLUSH);
			if (err == Z_OK)
			{
				bsize = gz_ptr[0];
				if (bsize == 0)
				{
					d_stream.avail_out = 4;
					d_stream.next_out = &gz_ptr[0];
					err = inflate(&d_stream, Z_SYNC_FLUSH);
					bsize = get_u32le(&gz_ptr[0]);
				}
				for (int i = 0; i < bsize; i++)
				{
					samples.resize(sample_count + 1);
					samples[sample_count++] = bit;
				}
				bit ^= 0xff;
			}
		}
		while (err == Z_OK);

		if (err != Z_STREAM_END)
		{
			LOG_FORMATS("inflate error: %d\n", err);
			return cassette_image::error::INVALID_IMAGE;
		}

		err = inflateEnd(&d_stream);
		if (err != Z_OK)
		{
			LOG_FORMATS("inflateEnd error: %d\n", err);
			return cassette_image::error::INVALID_IMAGE;
		}
		break;
	}

	return cassette->put_samples(0, 0.0, (double) sample_count / sample_rate, sample_count, 1, &samples[0], cassette_image::WAVEFORM_8BIT);
}


/*-------------------------------------------------
    CassetteFormat csw_cassette_format
-------------------------------------------------*/

const cassette_image::Format csw_cassette_format = {
	"csw",
	csw_cassette_identify,
	csw_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(csw_cassette_formats)
	CASSETTE_FORMAT(csw_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(bbc_cassette_formats)
	CASSETTE_FORMAT(csw_cassette_format)
	CASSETTE_FORMAT(uef_cassette_format)
CASSETTE_FORMATLIST_END
