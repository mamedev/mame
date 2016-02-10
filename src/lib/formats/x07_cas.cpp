// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/********************************************************************

    Support for Canon X-07 cassette images

********************************************************************/

#include <assert.h>

#include "x07_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define X07_WAV_FREQUENCY   4800
#define X07_TIMER_FREQUENCY 1200
#define X07_BIT_LENGTH      (X07_WAV_FREQUENCY/X07_TIMER_FREQUENCY)
#define X07_HEADER_BYTES    16

// image size
static int x07_image_size;

static int x07_put_samples(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int x07_output_bit(INT16 *buffer, int sample_pos, UINT8 bit)
{
	int samples = 0;

	if (bit)
	{
		samples += x07_put_samples(buffer, sample_pos + samples, X07_BIT_LENGTH/4, WAVEENTRY_HIGH);
		samples += x07_put_samples(buffer, sample_pos + samples, X07_BIT_LENGTH/4, WAVEENTRY_LOW);
		samples += x07_put_samples(buffer, sample_pos + samples, X07_BIT_LENGTH/4, WAVEENTRY_HIGH);
		samples += x07_put_samples(buffer, sample_pos + samples, X07_BIT_LENGTH/4, WAVEENTRY_LOW);
	}
	else
	{
		samples += x07_put_samples(buffer, sample_pos + samples, X07_BIT_LENGTH/2, WAVEENTRY_HIGH);
		samples += x07_put_samples(buffer, sample_pos + samples, X07_BIT_LENGTH/2, WAVEENTRY_LOW);
	}

	return samples;
}

static int x07_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;

	/* start */
	samples += x07_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (int i=0; i<8; i++)
		samples += x07_output_bit(buffer,sample_pos + samples, (byte>>i) & 0x01);

	/* stop */
	samples += x07_output_bit (buffer, sample_pos + samples, 1);
	samples += x07_output_bit (buffer, sample_pos + samples, 1);
	samples += x07_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static int x07_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	int sample_count = 0;
	int img_start = 0;

	/* start */
	for (int i=0; i<X07_WAV_FREQUENCY; i++)
		sample_count += x07_output_bit(buffer, sample_count, 1);

	/* header */
	if (bytes[0] == 0xd3 && bytes[1] == 0xd3 && bytes[2] == 0xd3 && bytes[3] == 0xd3)
	{
		// valid header
		for (int i=0; i<X07_HEADER_BYTES; i++)
			sample_count += x07_output_byte(buffer, sample_count, bytes[i]);

		img_start = X07_HEADER_BYTES;
	}
	else
	{
		// remove the NULL chars at start
		while (!bytes[img_start])
			img_start++;

		// insert 0xd3 bytes
		for (int i=0; i<10; i++)
			sample_count += x07_output_byte(buffer, sample_count, 0xd3);

		// fake filename
		for (int i=0; i<6; i++)
			sample_count += x07_output_byte(buffer, sample_count, 'A');
	}

	/* pause */
	for (int i=0; i<X07_WAV_FREQUENCY/16; i++)
		sample_count += x07_output_bit(buffer, sample_count, 1);

	/* data */
	for (int i=img_start; i<x07_image_size; i++)
		sample_count += x07_output_byte(buffer, sample_count, bytes[i]);

	/* end */
	for (int i=0; i<X07_WAV_FREQUENCY/8; i++)
		sample_count += x07_output_bit(buffer, sample_count, 1);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int x07_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return x07_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int x07_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	x07_image_size = length;

	return x07_handle_cassette(nullptr, bytes);
}

static const struct CassetteLegacyWaveFiller x07_legacy_fill_wave =
{
	x07_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	x07_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	X07_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t x07_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &x07_legacy_fill_wave);
}

static casserr_t x07_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &x07_legacy_fill_wave);
}

static const struct CassetteFormat x07_cassette_image_format =
{
	"k7,lst,cas",
	x07_cassette_identify,
	x07_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(x07_cassette_formats)
	CASSETTE_FORMAT(x07_cassette_image_format)
CASSETTE_FORMATLIST_END
