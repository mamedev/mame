// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

Atari 2600 SuperCharger support

*/

#include <assert.h>

#include "formats/a26_cas.h"


#define A26_CAS_SIZE            8448
#define A26_WAV_FREQUENCY       44100
#define BIT_ZERO_LENGTH         10
#define BIT_ONE_LENGTH          15
#define ZEROS_ONES              2755

static const UINT16 one_wave[BIT_ONE_LENGTH] = {
	0x2AE5, 0x4E60, 0x644E, 0x68E4, 0x5B56, 0x3DFE, 0x15ED, 0xEA13, 0xC202, 0xA4AA,
	0x971C, 0x9BB2, 0xB1A0, 0xD51B, 0x0000
};

static const UINT16 zero_wave[BIT_ZERO_LENGTH] = {
	0x3DFE, 0x644E, 0x644E, 0x3DFE, 0x0000, 0xC202, 0x9BB2, 0x9BB2, 0xC202, 0x0000
};

static int a26_cas_output_wave( INT16 **buffer, INT16 wave_data, int length ) {
	int i;

	if ( buffer ) {
		for ( i = 0 ; i < length; i++ ) {
			**buffer = wave_data;
			*buffer = *buffer + 1;
		}
	}
	return length;
}

static int a26_cas_output_bit( INT16 **buffer, int bit ) {
	int size = 0;
	int bit_size = bit ? BIT_ONE_LENGTH : BIT_ZERO_LENGTH;
	const INT16 *p = bit ? (const INT16 *)one_wave : (const INT16 *)zero_wave;
	int i;

	for ( i = 0; i < bit_size; i++ ) {
		size += a26_cas_output_wave( buffer, p[i], 1 );
	}

	return size;
}

static int a26_cas_output_byte( INT16 **buffer, UINT8 byte ) {
	int     size = 0;
	int     i;

	for( i = 0; i < 8; i++, byte <<= 1 ) {
		size += a26_cas_output_bit( buffer, ( ( byte & 0x80 ) ? 1 : 0 ) );
	}
	return size;
}

static int a26_cas_clearing_tone( INT16 **buffer ) {
	int     size = 0;

	size += a26_cas_output_wave( buffer, 0, 44100 );
	return size;
}

static int a26_cas_zeros_ones( INT16 **buffer ) {
	int     size = 0;
	int     i;

	for ( i = 0; i < ZEROS_ONES; i++ ) {
		size += a26_cas_output_bit( buffer, 0 );
		size += a26_cas_output_bit( buffer, 1 );
	}
	size += a26_cas_output_bit( buffer, 0 );
	size += a26_cas_output_bit( buffer, 0 );
	return size;
}

static int a26_cas_output_contents( INT16 **buffer, const UINT8 *bytes ) {
	int     size = 0;
	int     i, pages, j;

	/* There are 8 header bytes */
	for( i = 0; i < 8; i++ ) {
		size += a26_cas_output_byte( buffer, bytes[ 0x2000 + i ] );
	}

	pages = bytes[0x2003];

	/* Output each page prefixed with a small page header */
	for ( i = 0; i < pages; i++ ) {
		size += a26_cas_output_byte( buffer, bytes[ 0x2010 + i ] );
		size += a26_cas_output_byte( buffer, bytes[ 0x2040 + i ] );
		for ( j = 0; j < 256; j++ ) {
			size += a26_cas_output_byte( buffer, bytes[ i * 256 + j ] );
		}
	}

	return size;
}

static int a26_cas_do_work( INT16 **buffer, const UINT8 *bytes ) {
	int     size = 0;

	/* Output clearing tone */
	size += a26_cas_clearing_tone( buffer );

	/* Output header tone, alternating 1s and 0s for about a second ending with two 0s */
	size += a26_cas_zeros_ones( buffer );

	/* Output the actual contents of the tape */
	size += a26_cas_output_contents( buffer, bytes );

	/* Output footer tone, alternating 1s and 0s for about a second ending with two 0s */
	size += a26_cas_zeros_ones( buffer );

	return size;
}

static int a26_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes ) {
	INT16   *p = buffer;

	return a26_cas_do_work( &p, (const UINT8 *)bytes );
}

static int a26_cas_to_wav_size( const UINT8 *casdata, int caslen ) {
	return a26_cas_do_work( NULL, casdata );
}

static const struct CassetteLegacyWaveFiller a26_legacy_fill_wave = {
	a26_cas_fill_wave,
	-1,
	0,
	a26_cas_to_wav_size,
	A26_WAV_FREQUENCY,
	0,
	0
};

static casserr_t a26_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts ) {
	UINT64 size;

	size = cassette_image_size( cassette );
	if ( size == A26_CAS_SIZE ) {
		return cassette_legacy_identify( cassette, opts, &a26_legacy_fill_wave );
	}
	return CASSETTE_ERROR_INVALIDIMAGE;
}

static casserr_t a26_cassette_load( cassette_image *cassette ) {
	return cassette_legacy_construct( cassette, &a26_legacy_fill_wave );
}

static const struct CassetteFormat a26_cassette_format = {
	"a26",
	a26_cassette_identify,
	a26_cassette_load,
	NULL
};

CASSETTE_FORMATLIST_START(a26_cassette_formats)
	CASSETTE_FORMAT(a26_cassette_format)
CASSETTE_FORMATLIST_END
