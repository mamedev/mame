// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*

    Tape support for Glaksija GTP format

    Miodrag Milanovic
*/
#include <assert.h>

#include "gtp_cas.h"


#define GTP_WAV_FREQUENCY   44100
#define WAVE_LOW        -0x5a9e
#define WAVE_HIGH       0x5a9e
#define WAVE_NULL       0

#define GTP_BLOCK_STANDARD  0x00
#define GTP_BLOCK_TURBO     0x01
#define GTP_BLOCK_NAME      0x10

static INT16    wave_data;
static INT16    len;

#define PULSE_WIDTH     30
#define PERIOD_BASE     150
#define PERIOD_1        75
#define PERIOD_0        150

#define INTERBYTE_PAUSE     225
#define INTERBLOCK_PAUSE    100000


static void gtp_output_wave( INT16 **buffer, int length ) {
	if ( buffer == nullptr ) {
		return;
	}

	for( ; length > 0; length-- ) {
		**buffer = wave_data;
		*buffer = *buffer + 1;
	}
}



static int gtp_mod_1( INT16 **buffer )
{
	wave_data = WAVE_LOW;
	gtp_output_wave(buffer,PULSE_WIDTH);
	wave_data = WAVE_HIGH;
	gtp_output_wave(buffer,PULSE_WIDTH);
	wave_data = WAVE_NULL;
	gtp_output_wave(buffer,PERIOD_1 - 2 * PULSE_WIDTH);
	wave_data = WAVE_LOW;
	gtp_output_wave(buffer,PULSE_WIDTH);
	wave_data = WAVE_HIGH;
	gtp_output_wave(buffer,PULSE_WIDTH);
	wave_data = WAVE_NULL;
	gtp_output_wave(buffer,PERIOD_1 - 2 * PULSE_WIDTH);

	return PERIOD_1 * 2;
}

static int gtp_mod_0( INT16 **buffer )
{
	wave_data = WAVE_LOW;
	gtp_output_wave(buffer,PULSE_WIDTH);
	wave_data = WAVE_HIGH;
	gtp_output_wave(buffer,PULSE_WIDTH);
	wave_data = WAVE_NULL;
	gtp_output_wave(buffer,PERIOD_0 - 2 * PULSE_WIDTH);

	return PERIOD_0;
}

static int gtp_byte( INT16 **buffer, UINT8 val )
{
	UINT8 b;
	int j,size = 0;
	for (j=0;j<8;j++) {
		b = (val >> j) & 1;
		if (b==0) {
			size += gtp_mod_0(buffer);
		} else {
			size += gtp_mod_1(buffer);
		}
	}
	return size;
}

static int gtp_sync( INT16 **buffer )
{
	int i;
	int size = 0;

	for(i=0;i<100;i++) {
		if (i!=0) {
			// Interbyte pause
			wave_data = WAVE_NULL;
			gtp_output_wave(buffer,INTERBYTE_PAUSE);
			size += INTERBYTE_PAUSE;
		}
		size += gtp_byte(buffer,0);
	}
	return size;
}

static int gtp_cas_to_wav_size( const UINT8 *casdata, int caslen ) {
	int size,n;
	size = 0;
	n = 0;
	if (casdata == nullptr) return -1;
	while(n<caslen) {
		int block_type = casdata[n];
		int block_size = casdata[n+2]*256 + casdata[n+1];
		n+=5;
		if (block_type==GTP_BLOCK_STANDARD) {
			// Interblock pause
			size += INTERBLOCK_PAUSE;
			size += 100 * (PERIOD_0 * 8 + INTERBYTE_PAUSE) - INTERBYTE_PAUSE;
			size += (PERIOD_0 * 8 + INTERBYTE_PAUSE) * block_size;
		}
		n += block_size;
	}
	len = caslen;
	return size;
}

static int gtp_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes ) {
	int i,size,n;
	size = 0;
	n = 0;
	if (bytes == nullptr) return -1;
	while(n<len)
	{
		int block_type = bytes[n];
		int block_size = bytes[n+2]*256 + bytes[n+1];
		n+=5;
		if (block_type==GTP_BLOCK_STANDARD) {
			// Interblock pause
			wave_data = WAVE_NULL;
			gtp_output_wave(&buffer,INTERBLOCK_PAUSE);
			size += INTERBLOCK_PAUSE;
			size += gtp_sync(&buffer);

			for (i=0;i<block_size;i++) {
				// Interbyte pause
				wave_data = WAVE_NULL;
				gtp_output_wave(&buffer,INTERBYTE_PAUSE);
				size += INTERBYTE_PAUSE;

				size += gtp_byte(&buffer,bytes[n]);
				n++;
			}
		} else {
			n += block_size;
		}
	}
	return size;
}




static const struct CassetteLegacyWaveFiller gtp_legacy_fill_wave = {
	gtp_cas_fill_wave,          /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	gtp_cas_to_wav_size,            /* chunk_sample_calc */
	GTP_WAV_FREQUENCY,          /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};



static casserr_t gtp_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts ) {
	return cassette_legacy_identify( cassette, opts, &gtp_legacy_fill_wave );
}



static casserr_t gtp_cassette_load( cassette_image *cassette ) {
	return cassette_legacy_construct( cassette, &gtp_legacy_fill_wave );
}



static const struct CassetteFormat gtp_cassette_format = {
	"gtp",
	gtp_cassette_identify,
	gtp_cassette_load,
	nullptr
};


CASSETTE_FORMATLIST_START(gtp_cassette_formats)
	CASSETTE_FORMAT(gtp_cassette_format)
CASSETTE_FORMATLIST_END
