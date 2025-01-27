// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/********************************************************************

Support for Micronique machine .K7 and *.FOR cassette images

Note that the usual type for hector cassette is *.K7,
     the *.FOR type is only for programming screen in forth format.

You can find some *.K7 file on serveral server in France
(Micronique is a French factory) Enjoy !

 jj.stacino@aliceadsl.fr

Updated 3/1/10 : use real value for timing.
********************************************************************/

#include "hect_tap.h"


#define SMPLO   -32768
#define SILENCE 0
#define SMPHI   32767


static int cas_size; // FIXME: global variable prevents multiple instances


enum
{
	Header_cycles = 77, /* Valeur Th?orique 66 = 44100 * 1.5 / 1000  // mesur? sur jeu Formule1 = 1,75ms*/
	Zero_cycles =   27, /* Valeur Th?orique 17 = 44100 * 0.4 / 1000  // mesur? sur jeu Formule1 = 0,61ms*/
	Un_cycles =     50  /* Valeur Th?orique 40 = 44100 * 0.9 / 1000  // mesur? sur jeu Formule1 = 1,13ms*/
};

/* Here I prefer use the value that I read on a real tape, and not the theorical value; note that these
   value work best on my HRX...    Yo_fr   (jj.stacino@aliceadsl.fr)  */

/*******************************************************************
   Generate one high-low cycle of sample data
********************************************************************/
static inline int hector_tap_cycle(int16_t *buffer, int sample_pos, int high, int low)
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


static inline int hector_tap_byte(int16_t *buffer, int sample_pos, uint8_t data)
{
/* Writing an entire byte */
	int i, samples;

	samples = 0;
	for ( i = 0; i < 8; i++ )
	{
		if ( data & 0x01 )
			samples += hector_tap_cycle( buffer, sample_pos + samples, Un_cycles/2, Un_cycles/2 );
		else
			samples += hector_tap_cycle( buffer, sample_pos + samples, Zero_cycles/2, Zero_cycles/2 );

		data >>= 1;
	}
	return samples;
}


static inline int hector_tap_synchro(int16_t *buffer, int sample_pos, int nb_synchro)
{
/* Writing an entire byte */
	int i, samples;

	samples = 0;
	for ( i = 0; i < nb_synchro ; i++ )
			samples += hector_tap_cycle( buffer, sample_pos + samples, Header_cycles/2, Header_cycles/2 );

	return samples;
}


static int hector_handle_tap(int16_t *buffer, const uint8_t *casdata)
{
	int data_pos, sample_count/*, block_count*/;
	int previous_block=0;

	data_pos = 0;
	sample_count = 0;
	/*block_count = 0;*/
	previous_block = 0;

	/* First 768 cycle of synchro */
	sample_count += hector_tap_synchro( buffer, sample_count, 768-4 );

	/* on the entire file*/
	while( data_pos < cas_size )
	{
		uint16_t  block_size;

		if (previous_block == 0xFE)
				/* Starting a block with 150 cycle of synchro to let time to Hector to do the job ! */
				sample_count += hector_tap_synchro( buffer, sample_count, 150 );
		else
					/* Starting a block with 4 cycle of synchro */
				sample_count += hector_tap_synchro( buffer, sample_count, 4 );

		if (data_pos>1)
				previous_block = casdata[data_pos-1];

		/* Handle block length on tape data */
		block_size = casdata[data_pos] ;
		if (block_size==0)
			block_size=256;

		/*block_count++;*/
		sample_count += hector_tap_byte(buffer, sample_count, casdata[data_pos] );
		data_pos++;

		/* Data samples */
		for ( ; block_size ; data_pos++, block_size-- )
		{
			/* Make sure there are enough bytes left */
			if ( data_pos > cas_size )
				return -1;

			sample_count += hector_tap_byte( buffer, sample_count, casdata[data_pos] );

		}
	}
	/*Finish by a zero*/
	sample_count += hector_tap_byte( buffer, sample_count, 0 );

	return sample_count;
}
/*******************************************************************
////  FORTH DATA CASSETTE
*******************************************************************/


static int hector_handle_forth_tap(int16_t *buffer, const uint8_t *casdata)
{
	int data_pos, sample_count/*, block_count*/;
	/*int previous_block=0;*/

	data_pos = 0;
	sample_count = 0;
	/*block_count = 0;*/
	/*previous_block = 0;*/

	/* Out if len of file not modulo 822 octets    */
	if ( (cas_size % 822) != 0 )
		return -1;

	/* on the entire file*/
	while( data_pos < cas_size )
	{
		uint16_t  block_size;

		/* Starting a block with 768 cycle of synchro*/
		sample_count += hector_tap_synchro( buffer, sample_count, 768 );

		/* Handle block length on tape data */
		block_size = 822 ; /* Fixed size for the forth */

		/*block_count=0;*/

		/* Data samples */
		for ( ; block_size ; data_pos++, block_size-- )
		{
			/* Make sure there are enough bytes left */
			if ( data_pos > cas_size )
				return -1;

			sample_count += hector_tap_byte( buffer, sample_count, casdata[data_pos] );

		}
	}

	/*Finish by a zero*/
	sample_count += hector_tap_byte( buffer, sample_count, 0 );

	return sample_count;
}
/*******************************************************************
/////  END FORTH DATA CASSETTE
*******************************************************************/


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int hector_tap_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return hector_handle_tap( buffer, bytes );
}


/*******************************************************************
   Calculate the number of samples needed for this tape image  FORTH
********************************************************************/
static int hector_tap_forth_to_wav_size(const uint8_t *casdata, int caslen)
{
	cas_size = caslen ;

	return hector_handle_forth_tap( nullptr, casdata );
}

/*******************************************************************
   Generate samples for the tape image FORTH
********************************************************************/
static int hector_tap_forth_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return hector_handle_forth_tap( buffer, bytes ); //forth removed here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}


/*******************************************************************
   Calculate the number of samples needed for this tape image classical
********************************************************************/
static int hector_tap_to_wav_size(const uint8_t *casdata, int caslen)
{
	cas_size = caslen ;

	return hector_handle_tap( nullptr, casdata );//forth removed here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}


static const cassette_image::LegacyWaveFiller hector_legacy_fill_wave =
{
	hector_tap_fill_wave,                   /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	hector_tap_to_wav_size,                 /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static const cassette_image::LegacyWaveFiller hector_forth_legacy_fill_wave =
{
	hector_tap_forth_fill_wave,             /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	hector_tap_forth_to_wav_size,           /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static cassette_image::error hector_k7_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &hector_legacy_fill_wave);
}


static cassette_image::error hector_k7_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&hector_legacy_fill_wave);
}

static cassette_image::error hector_k7forth_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &hector_forth_legacy_fill_wave);
}


static cassette_image::error hector_k7forth_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&hector_forth_legacy_fill_wave);
}


static const cassette_image::Format hector_k7_format =
{
	"k7,cin",
	hector_k7_identify,
	hector_k7_load,
	nullptr
};

static const cassette_image::Format hector_k7Forth_format =
{
	"for",
	hector_k7forth_identify,
	hector_k7forth_load,
	nullptr
};


CASSETTE_FORMATLIST_START(hector_cassette_formats)
	CASSETTE_FORMAT(hector_k7_format)
	CASSETTE_FORMAT(hector_k7Forth_format)
CASSETTE_FORMATLIST_END
