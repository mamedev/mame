// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "kim1_cas.h"

#include "multibyte.h"

#include <cstring>

#define SMPLO   -32768
#define SMPHI   32767


static int cas_size; // FIXME: global variable prevents multiple instances


static inline int kim1_output_signal( int16_t *buffer, int sample_pos, int high )
{
	int sample_count, i, j;

	sample_count = 0;

	if ( high )
	{
		/* high frequency (~3600Hz) */
		for ( i = 0; i < 9; i++ )
		{
			if ( buffer )
			{
				for ( j = 0; j < 6; j++ )
					buffer[ sample_pos + sample_count + j ] = SMPHI;
			}
			sample_count += 6;

			if ( buffer )
			{
				for ( j = 0; j < 6; j++ )
					buffer[ sample_pos + sample_count + j ] = SMPLO;
			}
			sample_count += 6;
		}
	}
	else
	{
		/* low frequency (~2400Hz) */
		for ( i = 0; i < 6; i++ )
		{
			if ( buffer )
			{
				for ( j = 0; j < 9; j++ )
					buffer[ sample_pos + sample_count + j ] = SMPHI;
			}
			sample_count += 9;

			if ( buffer )
			{
				for ( j = 0; j < 9; j++ )
					buffer[ sample_pos + sample_count + j ] = SMPLO;
			}
			sample_count += 9;
		}
	}

	return sample_count;
}


static inline int kim1_output_byte( int16_t *buffer, int sample_pos, uint8_t byte )
{
	int i;
	int sample_count = 0;

	for ( i = 0; i < 8; i++ )
	{
		sample_count += kim1_output_signal( buffer, sample_pos + sample_count, 1 );
		sample_count += kim1_output_signal( buffer, sample_pos + sample_count, byte & 0x01 ? 0 : 1 );
		sample_count += kim1_output_signal( buffer, sample_pos + sample_count, 0 );
		byte >>= 1;
	}
	return sample_count;
}


static int kim1_handle_kim(int16_t *buffer, const uint8_t *casdata)
{
	static const uint8_t encoding[16] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46 };
	int i, data_pos, sample_count;
	uint16_t size, address, file_id, checksum;

	if ( cas_size < 9 ) return -1;
	if ( memcmp( casdata, "KIM1", 4 ) ) return -1;

	address = get_u16le( &casdata[4] );
	size = get_u16le( &casdata[6] );
	file_id = casdata[8];

	data_pos = 9;
	sample_count = 0;
	checksum = casdata[4] + casdata[5];

	/* First output a sync header: 100 x 16h */
	for ( i = 0; i < 100; i++ )
		sample_count += kim1_output_byte( buffer, sample_count, 0x16 );

	/* Output end of sync: 2Ah */
	sample_count += kim1_output_byte( buffer, sample_count, 0x2A );

	/* Output ID */
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ file_id >> 4 ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ file_id & 0x0f ] );

	/* Output starting address */
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ ( address & 0xff ) >> 4 ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ address & 0x0f ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ address >> 12 ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ ( address >> 8 ) & 0x0f ] );

	/* Output the data */
	while( data_pos < cas_size && data_pos < ( size + 9 ) )
	{
		uint8_t data = casdata[data_pos];

		sample_count += kim1_output_byte( buffer, sample_count, encoding[ data >> 4 ] );
		sample_count += kim1_output_byte( buffer, sample_count, encoding[ data & 0x0f ] );
		checksum += data;
		data_pos++;
	}

	/* Output end of data marker: 2Fh */
	sample_count += kim1_output_byte( buffer, sample_count, 0x2F );

	/* Output checksum */
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ ( checksum & 0xff ) >> 4 ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ checksum & 0x0f ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ checksum >> 12 ] );
	sample_count += kim1_output_byte( buffer, sample_count, encoding[ ( checksum >> 8 ) & 0x0f ] );

	/* Output end of transmission marker: 2 x 04h */
	sample_count += kim1_output_byte( buffer, sample_count, 0x04 );
	sample_count += kim1_output_byte( buffer, sample_count, 0x04 );

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int kim1_kim_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return kim1_handle_kim( buffer, bytes );
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int kim1_kim_to_wav_size(const uint8_t *casdata, int caslen)
{
	cas_size = caslen;

	return kim1_handle_kim( nullptr, casdata );
}

static const cassette_image::LegacyWaveFiller kim1_kim_legacy_fill_wave =
{
	kim1_kim_fill_wave,                     /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	kim1_kim_to_wav_size,                   /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static cassette_image::error kim1_kim_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &kim1_kim_legacy_fill_wave);
}


static cassette_image::error kim1_kim_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&kim1_kim_legacy_fill_wave);
}


static const cassette_image::Format kim1_kim_format =
{
	"kim,kim1",
	kim1_kim_identify,
	kim1_kim_load,
	nullptr
};


CASSETTE_FORMATLIST_START(kim1_cassette_formats)
	CASSETTE_FORMAT(kim1_kim_format)
CASSETTE_FORMATLIST_END
