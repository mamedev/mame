// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/********************************************************************

Support for TRS80 .cas cassette images

********************************************************************/
#include "formats/trs_cas.h"

#include <cassert>


#define SILENCE 0
#define SMPLO   -32768
#define SMPHI   32767


static int cas_size; // FIXME: global variable prevents multiple instances


/*******************************************************************
   Generate one high-low cycle of sample data
********************************************************************/
static inline int trs80l2_cas_cycle(int16_t *buffer, int sample_pos, bool bit)
{
	uint8_t i;

	if ( buffer )
	{
		for (i = 0; i < 32; i++)
			buffer[ sample_pos++ ] = SILENCE;
		for (i = 0; i < 6; i++)
			buffer[ sample_pos++ ] = bit ? SMPHI : SILENCE;
		for (i = 0; i < 6; i++)
			buffer[ sample_pos++ ] = bit ? SMPLO : SILENCE;
	}
	return 44;
}


static int trs80l2_handle_cas(int16_t *buffer, const uint8_t *casdata)
{
	int data_pos = 0, sample_count = 0;
	bool a5sw = false;

	data_pos = 0;
	sample_count = 0;

	while( data_pos < cas_size )
	{
		uint8_t   data = casdata[data_pos];

		for (uint8_t i = 0; i < 8; i++ )
		{
			/* Signal code */
			sample_count += trs80l2_cas_cycle( buffer, sample_count, true );

			/* Bit code */
			sample_count += trs80l2_cas_cycle( buffer, sample_count, data >> 7 );

			data <<= 1;
		}

		if (!a5sw && (casdata[data_pos] == 0xA5))
		{
			a5sw = true;
			// Need 1ms silence here while rom is busy
			sample_count += trs80l2_cas_cycle( buffer, sample_count, false );
		}

		data_pos++;
	}

	// Specification requires a short silence to indicate EOF
	sample_count += trs80l2_cas_cycle( buffer, sample_count, false );
	sample_count += trs80l2_cas_cycle( buffer, sample_count, false );
	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int trs80l2_cas_fill_wave(int16_t *buffer, int sample_count, uint8_t *bytes)
{
	return trs80l2_handle_cas( buffer, bytes );
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int trs80l2_cas_to_wav_size(const uint8_t *casdata, int caslen)
{
	cas_size = caslen;

	return trs80l2_handle_cas( nullptr, casdata );
}

static const cassette_image::LegacyWaveFiller trs80l2_cas_legacy_fill_wave =
{
	trs80l2_cas_fill_wave,                  /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	trs80l2_cas_to_wav_size,                /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static cassette_image::error trs80l2_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &trs80l2_cas_legacy_fill_wave);
}


static cassette_image::error trs80l2_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&trs80l2_cas_legacy_fill_wave);
}


static const cassette_image::Format trs80l2_cas_format =
{
	"cas",
	trs80l2_cas_identify,
	trs80l2_cas_load,
	nullptr
};


CASSETTE_FORMATLIST_START(trs80l2_cassette_formats)
	CASSETTE_FORMAT(trs80l2_cas_format)
CASSETTE_FORMATLIST_END
