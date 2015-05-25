// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Fujitsu FM-7 series cassette handling
 */

#include <assert.h>

#include "fm7_cas.h"

#define WAVE_HIGH        0x5a9e
#define WAVE_LOW        -0x5a9e

static int cas_size;

static int fm7_fill_wave(INT16* buffer, UINT8 high, UINT8 low, int sample_pos)
{
	UINT16 data = (high << 8) + low;
	int sample_count = 0;
	int x = 0;
	int count = (data & 0x7fff);

	if(data & 0x8000)
	{
		for(x=0;x<count;x++)
		{
			if(buffer)
				buffer[sample_pos+x] = WAVE_HIGH;
		}
	}
	else
	{
		for(x=0;x<count;x++)
		{
			if(buffer)
				buffer[sample_pos+x] = WAVE_LOW;
		}
	}

	sample_count += count;
	return sample_count;
}

static int fm7_handle_t77(INT16* buffer, const UINT8* casdata)
{
	int sample_count = 0;
	int data_pos = 16;

	if(memcmp(casdata, "XM7 TAPE IMAGE 0",16))  // header check
		return -1;

	while(data_pos < cas_size)
	{
		sample_count += fm7_fill_wave(buffer,casdata[data_pos],casdata[data_pos+1],sample_count);
		data_pos+=2;
	}

	return sample_count;
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int fm7_cas_to_wav_size (const UINT8 *casdata, int caslen)
{
	cas_size = caslen;

	return fm7_handle_t77(NULL,casdata);
}

/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int fm7_cas_fill_wave(INT16 *buffer, int sample_count, UINT8 *bytes)
{
	return fm7_handle_t77(buffer,bytes);
}

static const struct CassetteLegacyWaveFiller fm7_legacy_fill_wave =
{
	fm7_cas_fill_wave,                      /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	fm7_cas_to_wav_size,                    /* chunk_sample_calc */
	110250,                                 /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t fm7_cas_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &fm7_legacy_fill_wave);
}



static casserr_t fm7_cas_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &fm7_legacy_fill_wave);
}


static const struct CassetteFormat fm7_cassette_format = {
	"t77",
	fm7_cas_identify,
	fm7_cas_load,
	NULL
};

CASSETTE_FORMATLIST_START(fm7_cassette_formats)
	CASSETTE_FORMAT(fm7_cassette_format)
CASSETTE_FORMATLIST_END
