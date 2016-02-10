// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*

    Tape support for RK format

*/
#include <assert.h>

#include "rk_cas.h"


#define RK_WAV_FREQUENCY    44000
#define WAVE_HIGH       32767
#define WAVE_LOW        -32768

#define RK_HEADER_LEN   256

#define RK_SIZE_20 20
#define RK_SIZE_22 22
#define RK_SIZE_60 60

static int      data_size;

static INT16 *rk_emit_level(INT16 *p, int count, int level)
{
	int i;

	for (i=0; i<count; i++)
	{
		*(p++) = level;
	}
	return p;
}

static INT16* rk_output_bit(INT16 *p, UINT8 b,int bitsize)
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

static INT16* rk_output_byte(INT16 *p, UINT8 byte,int bitsize)
{
	int i;
	for (i=7; i>=0; i--)
		p = rk_output_bit(p,(byte>>i) & 0x01, bitsize);

	return p;
}


static int rk20_cas_to_wav_size( const UINT8 *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  8*2 + caslen * 8 * 2) * RK_SIZE_20;
}

static int rk22_cas_to_wav_size( const UINT8 *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  8*2 + caslen * 8 * 2) * RK_SIZE_22;
}

static int rk60_cas_to_wav_size( const UINT8 *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  8*2 + caslen * 8 * 2) * RK_SIZE_60;
}

static int gam_cas_to_wav_size( const UINT8 *casdata, int caslen ) {
	data_size = caslen;
	return  (RK_HEADER_LEN  * 8 * 2 +  caslen * 8 * 2) * RK_SIZE_20;
}

static int rk20_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes ) {
	int i;
	INT16 * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_20 );
	}

	p = rk_output_byte (p, 0xE6, RK_SIZE_20);

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_20);

	return p - buffer;
}

static int rk22_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes ) {
	int i;
	INT16 * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_22 );
	}

	p = rk_output_byte (p, 0xE6, RK_SIZE_22);

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_22);

	return p - buffer;
}

static int rk60_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes ) {
	int i;
	INT16 * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_60 );
	}

	p = rk_output_byte (p, 0xE6, RK_SIZE_60);

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_60);

	return p - buffer;
}

static int gam_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes ) {
	int i;
	INT16 * p = buffer;

	for (i=0; i<RK_HEADER_LEN; i++) {
		p = rk_output_byte (p, 0x00, RK_SIZE_20 );
	}

	for (i=0; i<data_size; i++)
		p = rk_output_byte (p, bytes[i], RK_SIZE_20);

	return p - buffer;
}

static const struct CassetteLegacyWaveFiller rk20_legacy_fill_wave = {
	rk20_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	rk20_cas_to_wav_size,           /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static casserr_t rk20_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts ) {
	return cassette_legacy_identify( cassette, opts, &rk20_legacy_fill_wave );
}

static casserr_t rk20_cassette_load( cassette_image *cassette ) {
	return cassette_legacy_construct( cassette, &rk20_legacy_fill_wave );
}

static const struct CassetteLegacyWaveFiller rk22_legacy_fill_wave = {
	rk22_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	rk22_cas_to_wav_size,           /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static casserr_t rk22_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts ) {
	return cassette_legacy_identify( cassette, opts, &rk22_legacy_fill_wave );
}

static casserr_t rk22_cassette_load( cassette_image *cassette ) {
	return cassette_legacy_construct( cassette, &rk22_legacy_fill_wave );
}

static const struct CassetteLegacyWaveFiller gam_legacy_fill_wave = {
	gam_cas_fill_wave,          /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	gam_cas_to_wav_size,            /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static casserr_t gam_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts ) {
	return cassette_legacy_identify( cassette, opts, &gam_legacy_fill_wave );
}

static casserr_t gam_cassette_load( cassette_image *cassette ) {
	return cassette_legacy_construct( cassette, &gam_legacy_fill_wave );
}

static const struct CassetteLegacyWaveFiller rk60_legacy_fill_wave = {
	rk60_cas_fill_wave,         /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	rk60_cas_to_wav_size,           /* chunk_sample_calc */
	RK_WAV_FREQUENCY,           /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static casserr_t rk60_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts ) {
	return cassette_legacy_identify( cassette, opts, &rk60_legacy_fill_wave );
}

static casserr_t rk60_cassette_load( cassette_image *cassette ) {
	return cassette_legacy_construct( cassette, &rk60_legacy_fill_wave );
}

static const struct CassetteFormat rku_cassette_format = {
	"rku",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const struct CassetteFormat rk8_cassette_format = {
	"rk8",
	rk60_cassette_identify,
	rk60_cassette_load,
	nullptr
};

static const struct CassetteFormat rks_cassette_format = {
	"rks",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const struct CassetteFormat rko_cassette_format = {
	"rko",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const struct CassetteFormat rkr_cassette_format = {
	"rk,rkr",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const struct CassetteFormat rka_cassette_format = {
	"rka",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const struct CassetteFormat rkm_cassette_format = {
	"rkm",
	rk22_cassette_identify,
	rk22_cassette_load,
	nullptr
};

static const struct CassetteFormat rkp_cassette_format = {
	"rkp",
	rk20_cassette_identify,
	rk20_cassette_load,
	nullptr
};

static const struct CassetteFormat gam_cassette_format = {
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
