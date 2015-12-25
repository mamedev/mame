// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/********************************************************************

Support for EACA Colour Genie .cas cassette images

Current state: Not working. Only the sync signal and 0x66 byte get
               recognized.

********************************************************************/

#include <assert.h>

#include "formats/cgen_cas.h"

#define TAPE_HEADER "Colour Genie - Virtual Tape File"

#define SMPLO   -32768
#define SMPHI   32767


static int cas_size;
static int level;


static int cgenie_output_byte(INT16 *buffer, int sample_count, UINT8 data)
{
	int i;
	int samples = 0;

	for ( i = 0; i < 8; i++ )
	{
		/* Output bit boundary */
		level ^= 1;
		if ( buffer )
			buffer[ sample_count + samples ] = level ? SMPHI : SMPLO;
		samples++;

		/* Output bit */
		if ( data & 0x80 )
			level ^= 1;
		if ( buffer )
			buffer[ sample_count + samples ] = level ? SMPHI : SMPLO;
		samples++;

		data <<= 1;
	}
	return samples;
}


static int cgenie_handle_cas(INT16 *buffer, const UINT8 *casdata)
{
	int i, data_pos, sample_count;

	data_pos = 0;
	sample_count = 0;
	level = 0;

	/* Check for presence of optional header */
	if ( ! memcmp( casdata, TAPE_HEADER, sizeof(TAPE_HEADER) - 1 ) )
	{
		/* Search for 0x00 or end of file */
		while ( data_pos < cas_size && casdata[data_pos] )
			data_pos++;

		/* If we're at the end of the file it's not a valid .cas file */
		if ( data_pos == cas_size )
			return -1;

		/* Skip the 0x00 byte */
		data_pos++;
	}

	/* If we're at the end of the file it's not a valid .cas file */
	if ( data_pos == cas_size )
		return -1;

	/* Check for beginning of tape file marker */
	if ( casdata[data_pos] != 0x66 )
		return -1;

	/* Create header */
	for ( i = 0; i < 256; i++ )
		sample_count += cgenie_output_byte( buffer, sample_count, 0xaa );

	/* Start outputting data */
	while( data_pos < cas_size )
	{
		sample_count += cgenie_output_byte( buffer, sample_count, casdata[data_pos] );
		data_pos++;
	}
	sample_count += cgenie_output_byte( buffer, sample_count, 0x00 );

	return sample_count;
}

/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int cgenie_cas_fill_wave(INT16 *buffer, int sample_count, UINT8 *bytes)
{
	return cgenie_handle_cas( buffer, bytes );
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int cgenie_cas_to_wav_size(const UINT8 *casdata, int caslen)
{
	cas_size = caslen;

	return cgenie_handle_cas( nullptr, casdata );
}

static const struct CassetteLegacyWaveFiller cgenie_cas_legacy_fill_wave =
{
	cgenie_cas_fill_wave,                   /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	cgenie_cas_to_wav_size,                 /* chunk_sample_calc */
	2400,                                   /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static casserr_t cgenie_cas_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &cgenie_cas_legacy_fill_wave);
}


static casserr_t cgenie_cas_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &cgenie_cas_legacy_fill_wave);
}


static const struct CassetteFormat cgenie_cas_format =
{
	"cas",
	cgenie_cas_identify,
	cgenie_cas_load,
	nullptr
};


CASSETTE_FORMATLIST_START(cgenie_cassette_formats)
	CASSETTE_FORMAT(cgenie_cas_format)
CASSETTE_FORMATLIST_END
