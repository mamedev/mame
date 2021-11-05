// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Heathkit H8 H8T cassette images


Standard Kansas City format (300 baud)

We output a leader, followed by the contents of the H8T file.

********************************************************************/

#include "h8_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define H8_WAV_FREQUENCY   9600

// image size
static int h8_image_size; // FIXME: global variable prevents multiple instances

static int h8_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int h8_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	for (uint8_t i = 0; i < 4; i++)
	{
		if (bit)
		{
			samples += h8_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += h8_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += h8_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += h8_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		}
		else
		{
			samples += h8_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
			samples += h8_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
		}
	}

	return samples;
}

static int h8_output_byte(int16_t *buffer, int sample_pos, uint8_t byte)
{
	int samples = 0;
	uint8_t i;

	// start bit
	samples += h8_output_bit (buffer, sample_pos + samples, 0);

	// data bits
	for (i = 0; i<8; i++)
		samples += h8_output_bit (buffer, sample_pos + samples, (byte >> i) & 1);

	// stop bits
	for (i = 0; i<2; i++)
		samples += h8_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static int h8_handle_cassette(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;
	uint32_t byte_count = 0;
	uint32_t i;


	// leader
	for (i=0; i<2000; i++)
		sample_count += h8_output_bit(buffer, sample_count, 1);

	// data
	for (i=byte_count; i<h8_image_size; i++)
		sample_count += h8_output_byte(buffer, sample_count, bytes[i]);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int h8_cassette_fill_wave(int16_t *buffer, int length, uint8_t *bytes)
{
	return h8_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int h8_cassette_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	h8_image_size = length;

	return h8_handle_cassette(nullptr, bytes);
}

static const cassette_image::LegacyWaveFiller h8_legacy_fill_wave =
{
	h8_cassette_fill_wave,                  // fill_wave
	-1,                                     // chunk_size
	0,                                      // chunk_samples
	h8_cassette_calculate_size_in_samples,  // chunk_sample_calc
	H8_WAV_FREQUENCY,                       // sample_frequency
	0,                                      // header_samples
	0                                       // trailer_samples
};

static cassette_image::error h8_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &h8_legacy_fill_wave);
}

static cassette_image::error h8_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&h8_legacy_fill_wave);
}

static const cassette_image::Format h8_cassette_image_format =
{
	"h8t",
	h8_cassette_identify,
	h8_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(h8_cassette_formats)
	CASSETTE_FORMAT(h8_cassette_image_format)
CASSETTE_FORMATLIST_END
