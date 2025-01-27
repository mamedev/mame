// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*
 * NEC PC-6001 cassette format handling
 */

#include "p6001_cas.h"

#define WAVE_HIGH        0x5a9e
#define WAVE_LOW        -0x5a9e

static int cas_size; // FIXME: global variable prevents multiple instances

static int pc6001_fill_wave(int16_t* buffer, uint8_t data, int sample_pos)
{
	int sample_count = 0;

	// one byte = 8 samples
	for(int x=0;x<8;x++)
	{
		if(buffer)
			buffer[sample_pos+x] = ((data >> (7-x)) & 1) ? WAVE_HIGH : WAVE_LOW;
	}

	sample_count+=8;
	return sample_count;
}

static int pc6001_handle_cas(int16_t* buffer, const uint8_t* casdata)
{
	int sample_count = 0;
	int data_pos = 0;

	while(sample_count < cas_size)
	{
		sample_count += pc6001_fill_wave(buffer,casdata[data_pos],sample_count);
		data_pos++;
	}

	return sample_count;
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int pc6001_cas_to_wav_size (const uint8_t *casdata, int caslen)
{
	cas_size = caslen*8;

	return pc6001_handle_cas(nullptr,casdata);
}

/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int pc6001_cas_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return pc6001_handle_cas(buffer,bytes);
}

static const cassette_image::LegacyWaveFiller pc6001_legacy_fill_wave =
{
	pc6001_cas_fill_wave,                   /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	pc6001_cas_to_wav_size,                 /* chunk_sample_calc */
	8000,                                   /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error pc6001_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &pc6001_legacy_fill_wave);
}



static cassette_image::error pc6001_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&pc6001_legacy_fill_wave);
}


static const cassette_image::Format pc6001_cassette_format = {
	"cas",
	pc6001_cas_identify,
	pc6001_cas_load,
	nullptr
};

CASSETTE_FORMATLIST_START(pc6001_cassette_formats)
	CASSETTE_FORMAT(pc6001_cassette_format)
CASSETTE_FORMATLIST_END
