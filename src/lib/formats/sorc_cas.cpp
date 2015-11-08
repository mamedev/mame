// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Exidy Sorcerer cassette images


Sorcerer tapes consist of these sections:
1. A high tone whenever idle
2. A header
3. The data, in blocks of 256 bytes plus a CRC byte
4. The last block may be shorter, depending on the number of bytes
   left to save.

Each byte has 1 start bit, 8 data bits (0-7), 2 stop bits.

The default speed is 1200 baud, which is what we emulate here.
A high bit is 1 cycle of 1200 Hz, while a low bit is half a cycle
of 600 Hz.

Formats:
TAPE - this contains a byte for each real byte, including all the
header and leader bytes.


********************************************************************/

#include <assert.h>

#include "sorc_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define SORCERER_WAV_FREQUENCY   4800

// image size
static int sorcerer_image_size;
static bool level;

static int sorcerer_put_samples(INT16 *buffer, int sample_pos, int count)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level ? WAVEENTRY_LOW : WAVEENTRY_HIGH;

		level ^= 1;
	}

	return count;
}

static int sorcerer_output_bit(INT16 *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += sorcerer_put_samples(buffer, sample_pos + samples, 2);
		samples += sorcerer_put_samples(buffer, sample_pos + samples, 2);
	}
	else
	{
		samples += sorcerer_put_samples(buffer, sample_pos + samples, 4);
	}

	return samples;
}

static int sorcerer_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;
	UINT8 i;

	/* start */
	samples += sorcerer_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (i = 0; i<8; i++)
		samples += sorcerer_output_bit (buffer, sample_pos + samples, (byte >> i) & 1);

	/* stop */
	for (i = 0; i<2; i++)
		samples += sorcerer_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static int sorcerer_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	UINT32 i;

	/* idle */
	for (i=0; i<2000; i++)
		sample_count += sorcerer_output_bit(buffer, sample_count, 1);

	/* data */
	for (i=0; i<sorcerer_image_size; i++)
		sample_count += sorcerer_output_byte(buffer, sample_count, bytes[i]);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int sorcerer_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return sorcerer_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int sorcerer_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	sorcerer_image_size = length;

	return sorcerer_handle_cassette(NULL, bytes);
}

static const struct CassetteLegacyWaveFiller sorcerer_legacy_fill_wave =
{
	sorcerer_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	sorcerer_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	SORCERER_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t sorcerer_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &sorcerer_legacy_fill_wave);
}

static casserr_t sorcerer_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &sorcerer_legacy_fill_wave);
}

static const struct CassetteFormat sorcerer_cassette_image_format =
{
	"tape",
	sorcerer_cassette_identify,
	sorcerer_cassette_load,
	NULL
};

CASSETTE_FORMATLIST_START(sorcerer_cassette_formats)
	CASSETTE_FORMAT(sorcerer_cassette_image_format)
CASSETTE_FORMATLIST_END
