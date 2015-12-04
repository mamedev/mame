// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/********************************************************************

    Support for VG-5000 .k7 cassette images

********************************************************************/
#include <assert.h>

#include "vg5k_cas.h"


#define SMPLO   -32768
#define SILENCE 0
#define SMPHI   32767


static int k7_size;

/*******************************************************************
   Generate one high-low cycle of sample data
********************************************************************/
INLINE int vg5k_cas_cycle(INT16 *buffer, int sample_pos, int len)
{
	int i = 0;

	if (buffer)
	{
		while( i < len)
		{
			buffer[ sample_pos + i ] = SMPHI;
			i++;
		}

		while( i < len * 2 )
		{
			buffer[ sample_pos + i ] = SMPLO;
			i++;
		}
	}
	return len * 2;
}


/*******************************************************************
   Generate n samples of silence
********************************************************************/
INLINE int vg5k_cas_silence(INT16 *buffer, int sample_pos, int len)
{
	int i = 0;

	if ( buffer )
		for( i = 0; i < len; i++)
			buffer[ sample_pos + i ] = SILENCE;

	return len;
}


/*******************************************************************
   Generate the end-byte samples
********************************************************************/
INLINE int vg5k_cas_eob(INT16 *buffer, int sample_pos)
{
	int i, samples = 0;

	for (i = 0; i < 4; i++)
		samples += vg5k_cas_cycle( buffer, sample_pos + samples, 5);

	samples += vg5k_cas_cycle( buffer, sample_pos + samples, 10);

	return samples;
}


INLINE int vg5k_cas_byte(INT16 *buffer, int sample_pos, UINT8 data)
{
/* Writing an entire byte */
	int i, samples;

	samples = 0;
	for ( i = 0; i < 8; i++ )
	{
		if ( data & 0x01 )
		{
			samples += vg5k_cas_cycle( buffer, sample_pos + samples, 5 );
			samples += vg5k_cas_cycle( buffer, sample_pos + samples, 5 );
		}
		else
		{
			samples += vg5k_cas_cycle( buffer, sample_pos + samples, 10 );
		}

		data >>= 1;
	}
	return samples;
}


/*******************************************************************
   Generate n sample of synchro
********************************************************************/
INLINE int vg5k_k7_synchro(INT16 *buffer, int sample_pos, int len)
{
	int i, samples = 0;

	for ( i = 0; i < len ; i++ )
			samples += vg5k_cas_cycle( buffer, sample_pos + samples, 5);

	samples += vg5k_cas_eob( buffer, sample_pos + samples);

	return samples;
}


static int vg5k_handle_tap(INT16 *buffer, const UINT8 *casdata)
{
	int data_pos, sample_count;

	data_pos = 0;
	sample_count = 0;

	/* file has to start with an head block */
	if (casdata[0] != 0xd3  || casdata[1] != 0xd3 || casdata[2] != 0xd3)
		return -1;

	/* on the entire file*/
	while( data_pos < k7_size )
	{
		UINT16  block_size = 0;

		/* Identify type of block */
		if (casdata[data_pos] == 0xd3)
		{
			/* head block have fixed size of 32 byte */
			block_size = 0x20;

			/* 1 sec of silence before the head block */
			sample_count += vg5k_cas_silence(buffer, sample_count, 44100);

			/* head block starts with 30000 samples of synchro */
			sample_count += vg5k_k7_synchro( buffer, sample_count, 30000 );
		}
		else if (casdata[data_pos] == 0xd6)
		{
			/* data block size is defined in head block */
			block_size = (casdata[data_pos - 4] | casdata[data_pos - 3]<<8) +  20;

			/* 10000 samples of silence before the data block */
			sample_count += vg5k_cas_silence(buffer, sample_count, 10000);

			/* data block starts with 7200 samples of synchro */
			sample_count += vg5k_k7_synchro( buffer, sample_count, 7200);
		}
		else
		{
			/* tries to handle files that do not respect the size declared in the head block */
			while (data_pos < k7_size && casdata[data_pos] != 0xd3 && casdata[data_pos] != 0xd6)
				data_pos++;
		}

		/* Data samples */
		for ( ; block_size ; data_pos++, block_size-- )
		{
			/* Make sure there are enough bytes left */
			if (data_pos > k7_size)
				return -1;

			sample_count += vg5k_cas_byte( buffer, sample_count, casdata[data_pos] );

			/* generate the end-byte samples */
			sample_count += vg5k_cas_eob( buffer, sample_count);
		}
	}

	/* Finish with 10000 samples of silence */
	sample_count += vg5k_cas_silence(buffer, sample_count, 10000);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int vg5k_k7_fill_wave(INT16 *buffer, int sample_count, UINT8 *bytes)
{
	return vg5k_handle_tap(buffer, bytes);
}


/*******************************************************************
   Calculate the number of samples needed for this tape image classical
********************************************************************/
static int vg5k_k7_to_wav_size(const UINT8 *casdata, int caslen)
{
	k7_size = caslen ;

	return vg5k_handle_tap( nullptr, casdata );
}


static const struct CassetteLegacyWaveFiller vg5k_legacy_fill_wave =
{
	vg5k_k7_fill_wave,                      /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	vg5k_k7_to_wav_size,                    /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t vg5k_k7_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &vg5k_legacy_fill_wave);
}


static casserr_t vg5k_k7_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &vg5k_legacy_fill_wave);
}


static const struct CassetteFormat vg5k_k7_format =
{
	"k7",
	vg5k_k7_identify,
	vg5k_k7_load,
	nullptr
};


CASSETTE_FORMATLIST_START(vg5k_cassette_formats)
	CASSETTE_FORMAT(vg5k_k7_format)
CASSETTE_FORMATLIST_END
