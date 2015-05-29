// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/* .LVT tape images */
#include <assert.h>

#include "lviv_lvt.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define LVIV_LVT_BIT_SAMPLES                60
#define LVIV_LVT_HEADER_PILOT_SAMPLES       5190*60
#define LVIV_LVT_HEADER_DATA_SAMPLES        16*11*60
#define LVIV_LVT_PAUSE_SAMPLES              69370
#define LVIV_LVT_BLOCK_PILOT_SAMPLES        1298*60

#define LVIV_LVT_HEADER_PILOT_LENGTH        5190
#define LVIV_LVT_BLOCK_PILOT_LENGTH         1298

static INT16 *lviv_emit_level(INT16 *p, int count, int level)
{
	int i;

	for (i=0; i<count; i++)
	{
		*(p++) = level;
	}
	return p;
}

static INT16* lviv_output_bit(INT16 *p, UINT8 b)
{
	if (b)
	{
		p = lviv_emit_level (p, 15, WAVEENTRY_HIGH);
		p = lviv_emit_level (p, 15, WAVEENTRY_LOW);
		p = lviv_emit_level (p, 15, WAVEENTRY_HIGH);
		p = lviv_emit_level (p, 15, WAVEENTRY_LOW);
	}
	else
	{
		p = lviv_emit_level (p, 30, WAVEENTRY_HIGH);
		p = lviv_emit_level (p, 30, WAVEENTRY_LOW);
	}
		return p;
}

static INT16* lviv_output_byte(INT16 *p, UINT8 byte)
{
	int i;

	p = lviv_output_bit (p, 0);

	for (i=0; i<8; i++)
		p = lviv_output_bit(p,(byte>>i) & 0x01);

	p = lviv_output_bit (p, 1);
	p = lviv_output_bit (p, 1);
	return p;
}

/*************************************************************************************/

static int lviv_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	int size;

	size =  LVIV_LVT_HEADER_PILOT_SAMPLES +
		LVIV_LVT_HEADER_DATA_SAMPLES +
		LVIV_LVT_PAUSE_SAMPLES +
		LVIV_LVT_BLOCK_PILOT_SAMPLES +
		(length-0x10)*11*LVIV_LVT_BIT_SAMPLES;

	return size;
}

/*************************************************************************************/

static int lviv_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	int i;
	INT16 * p = buffer;

	int data_size;

	for (i=0; i<LVIV_LVT_HEADER_PILOT_LENGTH; i++)
		p = lviv_output_bit (p, 1);

	for (i=0; i<10; i++)
		p = lviv_output_byte (p, bytes[0x09]);

	for (i=0; i<6; i++)
		p = lviv_output_byte (p, bytes[0x0a+i]);

	p = lviv_emit_level (p, LVIV_LVT_PAUSE_SAMPLES, WAVEENTRY_HIGH);

	for (i=0; i<LVIV_LVT_BLOCK_PILOT_LENGTH; i++)
		p = lviv_output_bit (p, 1);

	data_size = length - ( LVIV_LVT_HEADER_PILOT_SAMPLES +
					LVIV_LVT_HEADER_DATA_SAMPLES +
						LVIV_LVT_PAUSE_SAMPLES +
					LVIV_LVT_BLOCK_PILOT_SAMPLES );
	data_size/=660;

	for (i=0; i<data_size; i++)
		p = lviv_output_byte (p, bytes[0x10+i]);

	return p - buffer;
}



static const struct CassetteLegacyWaveFiller lviv_legacy_fill_wave =
{
	lviv_cassette_fill_wave,                    /* fill_wave */
	-1,                                         /* chunk_size */
	0,                                          /* chunk_samples */
	lviv_cassette_calculate_size_in_samples,    /* chunk_sample_calc */
	44100,                                      /* sample_frequency */
	0,                                          /* header_samples */
	0                                           /* trailer_samples */
};



static casserr_t lviv_lvt_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &lviv_legacy_fill_wave);
}



static casserr_t lviv_lvt_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &lviv_legacy_fill_wave);
}



static const struct CassetteFormat lviv_lvt_image_format =
{
	"lvt,lvr,lv0,lv1,lv2,lv3",
	lviv_lvt_identify,
	lviv_lvt_load,
	NULL
};



CASSETTE_FORMATLIST_START(lviv_lvt_format)
	CASSETTE_FORMAT(lviv_lvt_image_format)
CASSETTE_FORMATLIST_END
