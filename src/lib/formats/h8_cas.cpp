// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Heathkit H8 H8T cassette images


Standard Kansas City format (300 baud)

TODO - investigate 1200 buad support, H8 should support it, but H88 does not.

We output a leader, followed by the contents of the H8T file.

********************************************************************/

#include "h8_cas.h"

#include <algorithm>


static constexpr uint16_t WAVEENTRY_LOW         = -32768;
static constexpr uint16_t WAVEENTRY_HIGH        = 32767;
static constexpr uint16_t SILENCE               = 0;

// using a multiple of 4800 will ensure an integer multiple of samples for each wave.
static constexpr uint16_t H8_WAV_FREQUENCY      = 9600;
static constexpr uint16_t TAPE_BAUD_RATE        = 300;
static constexpr uint16_t SAMPLES_PER_BIT       = H8_WAV_FREQUENCY / TAPE_BAUD_RATE;
static constexpr uint16_t SAMPLES_PER_HALF_WAVE = SAMPLES_PER_BIT / 2;

static constexpr uint16_t ONE_FREQ              = 1200;
static constexpr uint16_t ZERO_FREQ             = 2400;
static constexpr uint16_t ONE_CYCLES            = H8_WAV_FREQUENCY / ONE_FREQ;
static constexpr uint16_t ZERO_CYCLES           = H8_WAV_FREQUENCY / ZERO_FREQ;

// image size
static int h8_image_size; // FIXME: global variable prevents multiple instances

static int h8_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		std::fill_n(&buffer[sample_pos], count, level);
	}

	return count;
}

static int h8_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	const int loops = bit ? ONE_CYCLES : ZERO_CYCLES;
	const int samplePerValue = SAMPLES_PER_HALF_WAVE / loops;

	for (int i = 0; i < loops; i++)
	{
		samples += h8_put_samples(buffer, sample_pos + samples, samplePerValue, WAVEENTRY_LOW);
		samples += h8_put_samples(buffer, sample_pos + samples, samplePerValue, WAVEENTRY_HIGH);
	}

	return samples;
}

static int h8_output_byte(int16_t *buffer, int sample_pos, uint8_t data)
{
	int samples = 0;

	// start bit
	samples += h8_output_bit(buffer, sample_pos + samples, 0);

	// data bits
	for (int i = 0; i < 8; i++)
	{
		samples += h8_output_bit(buffer, sample_pos + samples, data & 1);
		data >>= 1;
	}

	// stop bit
	samples += h8_output_bit(buffer, sample_pos + samples, 1);

	return samples;
}

static int h8_handle_cassette(int16_t *buffer, const uint8_t *bytes)
{
	int sample_count = 0;

	// leader - 1 second
	for (int i = 0; i < TAPE_BAUD_RATE; i++)
		sample_count += h8_output_bit(buffer, sample_count, 1);

	// data
	for (int i = 0; i < h8_image_size; i++)
		sample_count += h8_output_byte(buffer, sample_count, bytes[i]);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int h8_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
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
