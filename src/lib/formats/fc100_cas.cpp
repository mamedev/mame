// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Goldstar FC-100 cassette images


We don't actually have any info on the cassette frequencies, so
it's all a guess.

********************************************************************/

#include <assert.h>

#include "fc100_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define FC100_WAV_FREQUENCY   9600
#define FC100_HEADER_BYTES    16

// image size
static int fc100_image_size;

static int fc100_put_samples(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int fc100_output_bit(INT16 *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += fc100_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
		samples += fc100_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		samples += fc100_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
		samples += fc100_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
	}
	else
	{
		samples += fc100_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
		samples += fc100_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
	}

	return samples;
}

static int fc100_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;
	UINT8 i;

	/* start */
	samples += fc100_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (i = 0; i<8; i++)
		samples += fc100_output_bit (buffer, sample_pos + samples, (byte >> i) & 1);

	/* stop */
	for (i = 0; i<4; i++)
		samples += fc100_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static int fc100_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	UINT32 byte_count = 0;
	UINT32 i;


	/* start */
	for (i=0; i<2155; i++)
		sample_count += fc100_output_bit(buffer, sample_count, 1);

	/* header */
	for (int i=0; i<FC100_HEADER_BYTES; i++)
		sample_count += fc100_output_byte(buffer, sample_count, bytes[i]);

	byte_count = FC100_HEADER_BYTES;

	/* pause */
	for (i=0; i<1630; i++)
		sample_count += fc100_output_bit(buffer, sample_count, 1);

	/* data */
	for (i=byte_count; i<fc100_image_size; i++)
		sample_count += fc100_output_byte(buffer, sample_count, bytes[i]);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int fc100_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return fc100_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int fc100_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	fc100_image_size = length;

	return fc100_handle_cassette(nullptr, bytes);
}

static const struct CassetteLegacyWaveFiller fc100_legacy_fill_wave =
{
	fc100_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	fc100_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	FC100_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t fc100_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &fc100_legacy_fill_wave);
}

static casserr_t fc100_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &fc100_legacy_fill_wave);
}

static const struct CassetteFormat fc100_cassette_image_format =
{
	"cas",
	fc100_cassette_identify,
	fc100_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(fc100_cassette_formats)
	CASSETTE_FORMAT(fc100_cassette_image_format)
CASSETTE_FORMATLIST_END
