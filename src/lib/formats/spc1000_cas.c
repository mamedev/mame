// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Samsung SPC-1000 cassette images


Tape formats:

TAP: This is a series of 0x30 and 0x31 bytes, representing binary
     0 and 1. It includes the header and leaders.

CAS: This format has not been investigated yet.

STA: This format has not been investigated yet, but is assumed to
     be the save state of some other emulator.

IPL: This format has not been investigated yet.

********************************************************************/

#include "spc1000_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define SPC1000_WAV_FREQUENCY   17000

// image size
static int spc1000_image_size;

static int spc1000_put_samples(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int spc1000_output_bit(INT16 *buffer, int sample_pos, bool bit)
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

static int spc1000_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	UINT32 i;

	/* data */
	for (i=0; i<spc1000_image_size; i++)
		sample_count += spc1000_output_bit(buffer, sample_count, bytes[i]&1);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int spc1000_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return spc1000_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int spc1000_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	spc1000_image_size = length;

	return spc1000_handle_cassette(NULL, bytes);
}

static const struct CassetteLegacyWaveFiller spc1000_legacy_fill_wave =
{
	spc1000_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	spc1000_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	SPC1000_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t spc1000_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &spc1000_legacy_fill_wave);
}

static casserr_t spc1000_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &spc1000_legacy_fill_wave);
}

static const struct CassetteFormat spc1000_cassette_image_format =
{
	"tap",
	spc1000_cassette_identify,
	spc1000_cassette_load,
	NULL
};

CASSETTE_FORMATLIST_START(spc1000_cassette_formats)
	CASSETTE_FORMAT(spc1000_cassette_image_format)
CASSETTE_FORMATLIST_END
