// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/********************************************************************

Support for Jupiter Ace .tap cassette images

For more information see:
- http://www.jupiter-ace.co.uk/faq_ace_tap_format.html
- http://www.jupiter-ace.co.uk/doc_AceTapeFormat.html

********************************************************************/

#include <assert.h>

#include "ace_tap.h"


#define SMPLO   -32768
#define SILENCE 0
#define SMPHI   32767


static int cas_size;


/*******************************************************************
   Generate one high-low cycle of sample data
********************************************************************/
INLINE int ace_tap_cycle(INT16 *buffer, int sample_pos, int high, int low)
{
	int i = 0;

	if ( buffer )
	{
		while( i < high)
		{
			buffer[ sample_pos + i ] = SMPHI;
			i++;
		}

		while( i < high + low )
		{
			buffer[ sample_pos + i ] = SMPLO;
			i++;
		}
	}
	return high + low;
}


INLINE int ace_tap_silence(INT16 *buffer, int sample_pos, int samples)
{
	int i = 0;

	if ( buffer )
	{
		while( i < samples )
		{
			buffer[ sample_pos + i ] = SILENCE;
			i++;
		}
	}
	return samples;
}


INLINE int ace_tap_byte(INT16 *buffer, int sample_pos, UINT8 data)
{
	int i, samples;

	samples = 0;
	for ( i = 0; i < 8; i++ )
	{
		if ( data & 0x80 )
			samples += ace_tap_cycle( buffer, sample_pos + samples, 21, 22 );
		else
			samples += ace_tap_cycle( buffer, sample_pos + samples, 10, 11 );

		data <<= 1;
	}
	return samples;
}


static int ace_handle_tap(INT16 *buffer, const UINT8 *casdata)
{
	int data_pos, sample_count;

	/* Make sure the file starts with a valid header */
	if ( cas_size < 0x1C )
		return -1;
	if ( casdata[0] != 0x1A || casdata[1] != 0x00 )
		return -1;

	data_pos = 0;
	sample_count = 0;

	while( data_pos < cas_size )
	{
		UINT16  block_size;
		int     i;

		/* Handle a block of tape data */
		block_size = casdata[data_pos] + ( casdata[data_pos + 1] << 8 );
		data_pos += 2;

		/* Make sure there are enough bytes left */
		if ( data_pos > cas_size )
			return -1;

		/* 2 seconds silence */
		sample_count += ace_tap_silence( buffer, sample_count, 2 * 44100 );

		/* Add pilot tone samples: 4096 for header, 512 for data */
		for( i = ( block_size == 0x001A ) ? 4096 : 512; i; i-- )
			sample_count += ace_tap_cycle( buffer, sample_count, 27, 27 );

		/* Sync samples */
		sample_count += ace_tap_cycle( buffer, sample_count, 8, 11 );

		/* Output block type identification byte */
		sample_count += ace_tap_byte( buffer, sample_count, ( block_size != 0x001A ) ? 0xFF : 0x00 );

		/* Data samples */
		for ( ; block_size ; data_pos++, block_size-- )
			sample_count += ace_tap_byte( buffer, sample_count, casdata[data_pos] );

		/* End mark samples */
		sample_count += ace_tap_cycle( buffer, sample_count, 12, 57 );

		/* 3 seconds silence */
		sample_count += ace_tap_silence( buffer, sample_count, 3 * 44100 );
	}
	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int ace_tap_fill_wave(INT16 *buffer, int sample_count, UINT8 *bytes)
{
	return ace_handle_tap( buffer, bytes );
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int ace_tap_to_wav_size(const UINT8 *casdata, int caslen)
{
	cas_size = caslen;

	return ace_handle_tap( NULL, casdata );
}


static const struct CassetteLegacyWaveFiller ace_legacy_fill_wave =
{
	ace_tap_fill_wave,                  /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	ace_tap_to_wav_size,                /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static casserr_t ace_tap_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &ace_legacy_fill_wave);
}


static casserr_t ace_tap_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &ace_legacy_fill_wave);
}


static const struct CassetteFormat ace_tap_format =
{
	"tap",
	ace_tap_identify,
	ace_tap_load,
	NULL
};


CASSETTE_FORMATLIST_START(ace_cassette_formats)
	CASSETTE_FORMAT(ace_tap_format)
CASSETTE_FORMATLIST_END
