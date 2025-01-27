// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*

    Tape support for Orao  TAP format

*/

#include "orao_cas.h"


#define ORAO_WAV_FREQUENCY  44100
#define WAVE_HIGH       -24576
#define WAVE_LOW        24576

#define ORAO_WAVE_ONE   17
#define ORAO_WAVE_ZERO  9

#define ORAO_HEADER_SIZE 360

static int16_t    wave_data; // FIXME: global variables prevent multiple instances
static int        len;
static int        startpos;
static bool       newformat;

static void orao_output_wave( int16_t **buffer, int length ) {
	if ( buffer == nullptr ) {
		return;
	}

	for( ; length > 0; length-- ) {
		**buffer = wave_data;
		*buffer = *buffer + 1;
	}
}
static int orao_cas_to_wav_size( const uint8_t *casdata, int caslen ) {
	int i,j,size;
	uint8_t b;

	if (casdata == nullptr) return -1;
	if (caslen <= ORAO_HEADER_SIZE) {
		return -1;
	}
	size = 0;
	startpos = 0;
	newformat = true;
	if (casdata[0]==0x68 && casdata[1]==0x01 && casdata[2]==0x00) {
		startpos = ORAO_HEADER_SIZE;
		newformat = false;
	}
	for (i=startpos;i<caslen-startpos;i++) {
		for (j=0;j<8;j++) {
			b = (casdata[i] >> j) & 1;
			if (b==0) {
				size += 2*ORAO_WAVE_ZERO;
			} else {
				size += 2*ORAO_WAVE_ONE;
			}
		}
	}
	len = caslen;
	return size;
}

static int orao_cas_fill_wave( int16_t *buffer, int length, const uint8_t *bytes ) {
	int i,j,size,k;
	uint8_t b;
	size = 0;
	if (bytes == nullptr) return -1;
	for (i=startpos;i<len-startpos;i++) {
		for (j=0;j<8;j++) {
			k = newformat ? (7-j) : j;
			b = (bytes[i] >> k) & 1;
			if (b==0) {
				wave_data = WAVE_LOW;
				orao_output_wave(&buffer,ORAO_WAVE_ZERO);
				wave_data = WAVE_HIGH;
				orao_output_wave(&buffer,ORAO_WAVE_ZERO);
				size += 2 *ORAO_WAVE_ZERO;
			} else {
				wave_data = WAVE_LOW;
				orao_output_wave(&buffer,ORAO_WAVE_ONE);
				wave_data = WAVE_HIGH;
				orao_output_wave(&buffer,ORAO_WAVE_ONE);
				size += 2 * ORAO_WAVE_ONE;
			}
		}
	}

	return size;
}




static const cassette_image::LegacyWaveFiller orao_legacy_fill_wave = {
	orao_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	orao_cas_to_wav_size,           /* chunk_sample_calc */
	ORAO_WAV_FREQUENCY,         /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};



static cassette_image::error orao_cassette_identify( cassette_image *cassette, cassette_image::Options *opts ) {
	return cassette->legacy_identify( opts, &orao_legacy_fill_wave );
}



static cassette_image::error orao_cassette_load( cassette_image *cassette ) {
	return cassette->legacy_construct( &orao_legacy_fill_wave );
}



static const cassette_image::Format orao_cassette_format = {
	"tap",
	orao_cassette_identify,
	orao_cassette_load,
	nullptr
};


CASSETTE_FORMATLIST_START(orao_cassette_formats)
	CASSETTE_FORMAT(orao_cassette_format)
CASSETTE_FORMATLIST_END
