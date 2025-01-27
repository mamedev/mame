// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/* .LVT tape images */

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

static int16_t *lviv_emit_level(int16_t *p, int count, int level)
{
	for (int i=0; i<count; i++)
	{
		*(p++) = level;
	}
	return p;
}

static int16_t* lviv_output_bit(int16_t *p, uint8_t b)
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

static int16_t* lviv_output_byte(int16_t *p, uint8_t byte)
{
	p = lviv_output_bit (p, 0);

	for (int i=0; i<8; i++)
		p = lviv_output_bit(p,(byte>>i) & 0x01);

	p = lviv_output_bit (p, 1);
	p = lviv_output_bit (p, 1);
	return p;
}

/*************************************************************************************/

static int lviv_cassette_calculate_size_in_samples(const uint8_t *bytes, int length)
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

static int lviv_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	int16_t * p = buffer;

	int data_size;

	for (int i=0; i<LVIV_LVT_HEADER_PILOT_LENGTH; i++)
		p = lviv_output_bit (p, 1);

	for (int i=0; i<10; i++)
		p = lviv_output_byte (p, bytes[0x09]);

	for (int i=0; i<6; i++)
		p = lviv_output_byte (p, bytes[0x0a+i]);

	p = lviv_emit_level (p, LVIV_LVT_PAUSE_SAMPLES, WAVEENTRY_HIGH);

	for (int i=0; i<LVIV_LVT_BLOCK_PILOT_LENGTH; i++)
		p = lviv_output_bit (p, 1);

	data_size = length - ( LVIV_LVT_HEADER_PILOT_SAMPLES +
					LVIV_LVT_HEADER_DATA_SAMPLES +
						LVIV_LVT_PAUSE_SAMPLES +
					LVIV_LVT_BLOCK_PILOT_SAMPLES );
	data_size/=660;

	for (int i=0; i<data_size; i++)
		p = lviv_output_byte (p, bytes[0x10+i]);

	return p - buffer;
}



static const cassette_image::LegacyWaveFiller lviv_legacy_fill_wave =
{
	lviv_cassette_fill_wave,                    /* fill_wave */
	-1,                                         /* chunk_size */
	0,                                          /* chunk_samples */
	lviv_cassette_calculate_size_in_samples,    /* chunk_sample_calc */
	44100,                                      /* sample_frequency */
	0,                                          /* header_samples */
	0                                           /* trailer_samples */
};



static cassette_image::error lviv_lvt_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &lviv_legacy_fill_wave);
}



static cassette_image::error lviv_lvt_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&lviv_legacy_fill_wave);
}



static const cassette_image::Format lviv_lvt_image_format =
{
	"lvt,lvr,lv0,lv1,lv2,lv3",
	lviv_lvt_identify,
	lviv_lvt_load,
	nullptr
};



CASSETTE_FORMATLIST_START(lviv_lvt_format)
	CASSETTE_FORMAT(lviv_lvt_image_format)
CASSETTE_FORMATLIST_END
