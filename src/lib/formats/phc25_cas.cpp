// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Sanyo PHC25 cassette images

phc images consist of 5 sections
1. 10 A5 bytes
2. 6  name of the program
3. The basic program file. Each line is delimited by null. This
   section is terminated by 3 nulls (1 of them for the preceeding
   line, the other 2 indicate the end).
4. The line numbers and pointers to them. It ends at the image size-1
5. A 1-byte trailer of FF which we do not pass on

Each byte after conversion becomes a start bit, bit 0,1,etc to 7,
then 4 stop bits.

An actual tape consists of 6 sections
a. 2.656secs of silence
b. 4.862secs of high bits
c. The header which is parts 1 and 2 above
d. 0.652secs of high bits
e. The main program which is parts 3 and 4 above
f. 1.771secs of silence

We don't emulate the full silence and high-bits periods, only just
enough to make it work.

********************************************************************/

#include "phc25_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define PHC25_WAV_FREQUENCY   9600
#define PHC25_HEADER_BYTES    16

// image size
static int phc25_image_size; // FIXME: global variable prevents multiple instances

static int phc25_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int phc25_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += phc25_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
		samples += phc25_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		samples += phc25_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
		samples += phc25_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
	}
	else
	{
		samples += phc25_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
		samples += phc25_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
	}

	return samples;
}

static int phc25_output_byte(int16_t *buffer, int sample_pos, uint8_t byte)
{
	int samples = 0;
	uint8_t i;

	/* start */
	samples += phc25_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (i = 0; i<8; i++)
		samples += phc25_output_bit (buffer, sample_pos + samples, (byte >> i) & 1);

	/* stop */
	for (i = 0; i<4; i++)
		samples += phc25_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static int phc25_handle_cassette(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;
	uint32_t byte_count = 0;
	uint32_t i;

	// silence
//  sample_count += phc25_put_samples(buffer, 6640, 2, WAVEENTRY_HIGH);

	/* start */
//  for (i=0; i<12155; i++)
	for (i=0; i<2155; i++)
		sample_count += phc25_output_bit(buffer, sample_count, 1);

	/* header */
	for (int i=0; i<PHC25_HEADER_BYTES; i++)
		sample_count += phc25_output_byte(buffer, sample_count, bytes[i]);

	byte_count = PHC25_HEADER_BYTES;

	/* pause */
	for (i=0; i<1630; i++)
		sample_count += phc25_output_bit(buffer, sample_count, 1);

	/* data */
	for (i=byte_count; i<phc25_image_size-1; i++)
		sample_count += phc25_output_byte(buffer, sample_count, bytes[i]);

	// silence
	sample_count += phc25_put_samples(buffer, 1000, 2, WAVEENTRY_HIGH);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int phc25_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return phc25_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int phc25_cassette_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	phc25_image_size = length;

	return phc25_handle_cassette(nullptr, bytes);
}

static const cassette_image::LegacyWaveFiller phc25_legacy_fill_wave =
{
	phc25_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	phc25_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	PHC25_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error phc25_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &phc25_legacy_fill_wave);
}

static cassette_image::error phc25_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&phc25_legacy_fill_wave);
}

static const cassette_image::Format phc25_cassette_image_format =
{
	"phc",
	phc25_cassette_identify,
	phc25_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(phc25_cassette_formats)
	CASSETTE_FORMAT(phc25_cassette_image_format)
CASSETTE_FORMATLIST_END
