// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Samsung SPC-1000 cassette images


Tape formats:

TAP: This is a series of 0x30 and 0x31 bytes, representing binary
     0 and 1. It includes the header and leaders.

CAS: Files in this format consist of a 16 bytes header (SPC-1000.CASfmt )
     followed by cassette bits packed together (each byte of a .cas file
     are 8 bits, most significant bit first)

STA: This format has not been investigated yet, but is assumed to
     be the save state of some other emulator.

IPL: This seems a quickload format containing RAM dump, not a real tape

********************************************************************/

#include "spc1000_cas.h"


#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define SPC1000_WAV_FREQUENCY   17000

// image size
static int spc1000_image_size; // FIXME: global variable prevents multiple instances

static int spc1000_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int spc1000_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += spc1000_put_samples(buffer, sample_pos + samples, 9, WAVEENTRY_LOW);
		samples += spc1000_put_samples(buffer, sample_pos + samples, 9, WAVEENTRY_HIGH);
	}
	else
	{
		samples += spc1000_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
		samples += spc1000_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
	}

	return samples;
}

static int spc1000_handle_tap(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;

	/* data */
	for (uint32_t i = 0; i < spc1000_image_size; i++)
		sample_count += spc1000_output_bit(buffer, sample_count, bytes[i] & 1);

	return sample_count;
}

static int spc1000_handle_cas(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;

	/* data (skipping first 16 bytes, which is CAS header) */
	for (uint32_t i = 0x10; i < spc1000_image_size; i++)
		for (int j = 0; j < 8; j++)
			sample_count += spc1000_output_bit(buffer, sample_count, (bytes[i] >> (7 - j)) & 1);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int spc1000_tap_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return spc1000_handle_tap(buffer, bytes);
}

static int spc1000_cas_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return spc1000_handle_cas(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int spc1000_tap_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	spc1000_image_size = length;

	return spc1000_handle_tap(nullptr, bytes);
}

static int spc1000_cas_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	spc1000_image_size = length;

	return spc1000_handle_cas(nullptr, bytes);
}


/*******************************************************************
   Formats
 ********************************************************************/


// TAP
static const cassette_image::LegacyWaveFiller spc1000_tap_legacy_fill_wave =
{
	spc1000_tap_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	spc1000_tap_calculate_size_in_samples, /* chunk_sample_calc */
	SPC1000_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error spc1000_tap_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &spc1000_tap_legacy_fill_wave);
}

static cassette_image::error spc1000_tap_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&spc1000_tap_legacy_fill_wave);
}

static const cassette_image::Format spc1000_tap_cassette_image_format =
{
	"tap",
	spc1000_tap_cassette_identify,
	spc1000_tap_cassette_load,
	nullptr
};


// CAS
static const cassette_image::LegacyWaveFiller spc1000_cas_legacy_fill_wave =
{
	spc1000_cas_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	spc1000_cas_calculate_size_in_samples, /* chunk_sample_calc */
	SPC1000_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error spc1000_cas_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &spc1000_cas_legacy_fill_wave);
}

static cassette_image::error spc1000_cas_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&spc1000_cas_legacy_fill_wave);
}

static const cassette_image::Format spc1000_cas_cassette_image_format =
{
	"cas",
	spc1000_cas_cassette_identify,
	spc1000_cas_cassette_load,
	nullptr
};



CASSETTE_FORMATLIST_START(spc1000_cassette_formats)
	CASSETTE_FORMAT(spc1000_tap_cassette_image_format)
	CASSETTE_FORMAT(spc1000_cas_cassette_image_format)
CASSETTE_FORMATLIST_END
