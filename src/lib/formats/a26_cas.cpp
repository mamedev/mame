// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

Atari 2600 SuperCharger support

*/

#include "formats/a26_cas.h"


#define A26_CAS_SIZE            8448
#define A26_WAV_FREQUENCY       44100
#define BIT_ZERO_LENGTH         10
#define BIT_ONE_LENGTH          15
#define ZEROS_ONES              2755

static const uint16_t one_wave[BIT_ONE_LENGTH] = {
	0x2AE5, 0x4E60, 0x644E, 0x68E4, 0x5B56, 0x3DFE, 0x15ED, 0xEA13, 0xC202, 0xA4AA,
	0x971C, 0x9BB2, 0xB1A0, 0xD51B, 0x0000
};

static const uint16_t zero_wave[BIT_ZERO_LENGTH] = {
	0x3DFE, 0x644E, 0x644E, 0x3DFE, 0x0000, 0xC202, 0x9BB2, 0x9BB2, 0xC202, 0x0000
};

static int a26_cas_output_wave( int16_t **buffer, int16_t wave_data, int length ) {
	int i;

	if ( buffer ) {
		for ( i = 0 ; i < length; i++ ) {
			**buffer = wave_data;
			*buffer = *buffer + 1;
		}
	}
	return length;
}

static int a26_cas_output_bit( int16_t **buffer, int bit ) {
	int size = 0;
	int bit_size = bit ? BIT_ONE_LENGTH : BIT_ZERO_LENGTH;
	const int16_t *p = bit ? (const int16_t *)one_wave : (const int16_t *)zero_wave;
	int i;

	for ( i = 0; i < bit_size; i++ ) {
		size += a26_cas_output_wave( buffer, p[i], 1 );
	}

	return size;
}

static int a26_cas_output_byte( int16_t **buffer, uint8_t byte ) {
	int     size = 0;
	int     i;

	for( i = 0; i < 8; i++, byte <<= 1 ) {
		size += a26_cas_output_bit( buffer, ( ( byte & 0x80 ) ? 1 : 0 ) );
	}
	return size;
}

static int a26_cas_clearing_tone( int16_t **buffer ) {
	int     size = 0;

	size += a26_cas_output_wave( buffer, 0, 44100 );
	return size;
}

static int a26_cas_zeros_ones( int16_t **buffer ) {
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

static int a26_cas_output_contents( int16_t **buffer, const uint8_t *bytes ) {
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

static int a26_cas_do_work( int16_t **buffer, const uint8_t *bytes ) {
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

static int a26_cas_fill_wave( int16_t *buffer, int length, const uint8_t *bytes ) {
	int16_t   *p = buffer;

	return a26_cas_do_work( &p, (const uint8_t *)bytes );
}

static int a26_cas_to_wav_size( const uint8_t *casdata, int caslen ) {
	return a26_cas_do_work( nullptr, casdata );
}

static const cassette_image::LegacyWaveFiller a26_legacy_fill_wave = {
	a26_cas_fill_wave,
	-1,
	0,
	a26_cas_to_wav_size,
	A26_WAV_FREQUENCY,
	0,
	0
};

static cassette_image::error a26_cassette_identify( cassette_image *cassette, cassette_image::Options *opts ) {
	uint64_t size;

	size = cassette->image_size( );
	if ( size == A26_CAS_SIZE ) {
		return cassette->legacy_identify( opts, &a26_legacy_fill_wave );
	}
	return cassette_image::error::INVALID_IMAGE;
}

static cassette_image::error a26_cassette_load( cassette_image *cassette ) {
	return cassette->legacy_construct( &a26_legacy_fill_wave );
}

static const cassette_image::Format a26_cassette_format = {
	"a26",
	a26_cassette_identify,
	a26_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(a26_cassette_formats)
	CASSETTE_FORMAT(a26_cassette_format)
CASSETTE_FORMATLIST_END
