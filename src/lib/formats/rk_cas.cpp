// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*

    Tape support for RK format

*/

#include "rk_cas.h"


#define RK_WAV_FREQUENCY    44000
#define WAVE_HIGH       32767
#define WAVE_LOW        -32768

#define RK_HEADER_LEN   256

#define RK_SIZE_20 20
#define RK_SIZE_22 22
#define RK_SIZE_60 60

static int      data_size; // FIXME: global variable prevents multiple instances

static int16_t *rk_emit_level(int16_t *p, int count, int level)
{

	for (int i=0; i<count; i++)
	{
		*(p++) = level;
	}
	return p;
}

static int16_t* rk_output_bit(int16_t *p, uint8_t b,int bitsize)
{
	if (b)
	{
		p = rk_emit_level (p, bitsize, WAVE_HIGH);
		p = rk_emit_level (p, bitsize, WAVE_LOW);
	}
	else
	{
		p = rk_emit_level (p, bitsize, WAVE_LOW);
		p = rk_emit_level (p, bitsize, WAVE_HIGH);

	}
	return p;
}

static int16_t* rk_output_byte(int16_t *p, uint8_t byte,int bitsize)
{
	int i;
	for (i=7; i>=0; i--)
		p = rk_output_bit(p,(byte>>i) & 0x01, bitsize);

	return p;
}


static int rk20_cas_to_wav_size( const uint8_t *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  8*2 + caslen * 8 * 2) * RK_SIZE_20;
}

static int rk22_cas_to_wav_size( const uint8_t *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  8*2 + caslen * 8 * 2) * RK_SIZE_22;
}

static int rk60_cas_to_wav_size( const uint8_t *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  8*2 + caslen * 8 * 2) * RK_SIZE_60;
}

static int gam_cas_to_wav_size( const uint8_t *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  caslen * 8 * 2) * RK_SIZE_20;
}

static int rk20_cas_fill_wave( int16_t *buffer, int length, const uint8_t *bytes ) {
	int i;
	int16_t * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_20 );
	}

	p = rk_output_byte (p, 0xE6, RK_SIZE_20);

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_20);

	return p - buffer;
}

static int rk22_cas_fill_wave( int16_t *buffer, int length, const uint8_t *bytes ) {
	int i;
	int16_t * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_22 );
	}

	p = rk_output_byte (p, 0xE6, RK_SIZE_22);

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_22);

	return p - buffer;
}

static int rk60_cas_fill_wave( int16_t *buffer, int length, const uint8_t *bytes ) {
	int i;
	int16_t * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_60 );
	}

	p = rk_output_byte (p, 0xE6, RK_SIZE_60);

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_60);

	return p - buffer;
}

static int gam_cas_fill_wave( int16_t *buffer, int length, const uint8_t *bytes ) {
	int i;
	int16_t * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_20 );
	}

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_20);

	return p - buffer;
}

static const cassette_image::LegacyWaveFiller rk20_legacy_fill_wave = {
	rk20_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	rk20_cas_to_wav_size,           /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static cassette_image::error rk20_cassette_identify( cassette_image *cassette, cassette_image::Options *opts ) {
	return cassette->legacy_identify( opts, &rk20_legacy_fill_wave );
}

static cassette_image::error rk20_cassette_load( cassette_image *cassette ) {
	return cassette->legacy_construct( &rk20_legacy_fill_wave );
}

static const cassette_image::LegacyWaveFiller rk22_legacy_fill_wave = {
	rk22_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	rk22_cas_to_wav_size,           /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static cassette_image::error rk22_cassette_identify( cassette_image *cassette, cassette_image::Options *opts ) {
	return cassette->legacy_identify( opts, &rk22_legacy_fill_wave );
}

static cassette_image::error rk22_cassette_load( cassette_image *cassette ) {
	return cassette->legacy_construct( &rk22_legacy_fill_wave );
}

static const cassette_image::LegacyWaveFiller gam_legacy_fill_wave = {
	gam_cas_fill_wave,          /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	gam_cas_to_wav_size,            /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static cassette_image::error gam_cassette_identify( cassette_image *cassette, cassette_image::Options *opts ) {
	return cassette->legacy_identify( opts, &gam_legacy_fill_wave );
}

static cassette_image::error gam_cassette_load( cassette_image *cassette ) {
	return cassette->legacy_construct( &gam_legacy_fill_wave );
}

static const cassette_image::LegacyWaveFiller rk60_legacy_fill_wave = {
	rk60_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	rk60_cas_to_wav_size,           /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static cassette_image::error rk60_cassette_identify( cassette_image *cassette, cassette_image::Options *opts ) {
	return cassette->legacy_identify( opts, &rk60_legacy_fill_wave );
}

static cassette_image::error rk60_cassette_load( cassette_image *cassette ) {
	return cassette->legacy_construct( &rk60_legacy_fill_wave );
}

static const cassette_image::Format rku_cassette_format = {
	"rku",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const cassette_image::Format rk8_cassette_format = {
	"rk8",
	rk60_cassette_identify,
	rk60_cassette_load,
	nullptr
};

static const cassette_image::Format rks_cassette_format = {
	"rks",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const cassette_image::Format rko_cassette_format = {
	"rko",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const cassette_image::Format rkr_cassette_format = {
	"rk,rkr",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const cassette_image::Format rka_cassette_format = {
	"rka",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const cassette_image::Format rkm_cassette_format = {
	"rkm",
	rk22_cassette_identify,
	rk22_cassette_load,
	nullptr
};

static const cassette_image::Format rkp_cassette_format = {
	"rkp",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const cassette_image::Format gam_cassette_format = {
	"gam,g16,pki",
	gam_cassette_identify,
	gam_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(rku_cassette_formats)
	CASSETTE_FORMAT(rku_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rk8_cassette_formats)
	CASSETTE_FORMAT(rk8_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rks_cassette_formats)
	CASSETTE_FORMAT(rks_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rko_cassette_formats)
	CASSETTE_FORMAT(rko_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rkr_cassette_formats)
	CASSETTE_FORMAT(rkr_cassette_format)
	CASSETTE_FORMAT(gam_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rka_cassette_formats)
	CASSETTE_FORMAT(rka_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rkm_cassette_formats)
	CASSETTE_FORMAT(rkm_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(rkp_cassette_formats)
	CASSETTE_FORMAT(rkp_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(gam_cassette_formats)
	CASSETTE_FORMAT(gam_cassette_format)
CASSETTE_FORMATLIST_END
