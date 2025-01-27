// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/********************************************************************

Support for TRS80 .cas cassette images
Types handled:
- Model 1 Level I: 250 baud
- Model 1 Level II: 500 baud
- Model 3/4: 1500 baud.

Level I and II tape format is completely identical, apart from the
baud rate. The contents are specific to each system though.
The Model 3 and 4 can load either Level II tapes (by answering L
to the Cass? prompt), or the fast format by hitting enter at Cass?

********************************************************************/

#include "formats/trs_cas.h"


#define SILENCE 0
#define SMPLO   -32768
#define SMPHI   32767


static int cas_size = 0; // FIXME: global variable prevents multiple instances


/*******************************************************************
   Generate one high-low cycle of sample data
********************************************************************/
static inline int trs80m1_cas_cycle(int16_t *buffer, int sample_pos, bool bit)
{
	uint8_t i;

	if ( buffer )
	{
		for (i = 0; i < 32; i++)
			buffer[ sample_pos++ ] = SILENCE;
		for (i = 0; i < 6; i++)
			buffer[ sample_pos++ ] = bit ? SMPHI : SILENCE;
		for (i = 0; i < 6; i++)
			buffer[ sample_pos++ ] = bit ? SMPLO : SILENCE;
	}
	return 44;
}


static int trs80m1_handle_cas(int16_t *buffer, const uint8_t *casdata)
{
	int data_pos = 0, sample_count = 0;
	bool sw = false;

	// Make sure this is a trs80 tape
	// Should have some zero bytes then one 0xA5
	while ((cas_size > data_pos) && (casdata[data_pos] == 0x00))
		data_pos++;
	if (casdata[data_pos] != 0xA5)
		return 0;

	data_pos = 0;
	while( data_pos < cas_size )
	{
		uint8_t data = casdata[data_pos];

		for (uint8_t i = 0; i < 8; i++ )
		{
			/* Signal code */
			sample_count += trs80m1_cas_cycle( buffer, sample_count, true );

			/* Bit code */
			sample_count += trs80m1_cas_cycle( buffer, sample_count, data >> 7 );

			data <<= 1;
		}

		if (!sw && (casdata[data_pos] == 0xA5))
		{
			sw = true;
			// Need 1ms silence here while rom is busy
			sample_count += trs80m1_cas_cycle( buffer, sample_count, false );
		}

		data_pos++;
	}

	// Specification requires a short silence to indicate EOF
	sample_count += trs80m1_cas_cycle( buffer, sample_count, false );
	sample_count += trs80m1_cas_cycle( buffer, sample_count, false );
	return sample_count;
}

static inline int trs80m3_cas_cycle(int16_t *buffer, int sample_pos, bool bit)
{
	uint8_t i, counts = bit ? 8 : 16;

	if ( buffer )
	{
		for (i = 0; i < counts; i++)
			buffer[ sample_pos++ ] = SMPHI;
		for (i = 0; i < counts; i++)
			buffer[ sample_pos++ ] = SMPLO;
	}
	return counts*2;
}


static int trs80m3_handle_cas(int16_t *buffer, const uint8_t *casdata)
{
	int data_pos = 0, sample_count = 0;
	uint8_t sw = 0, bitout = 0, byteout = 0;

	// Make sure this is a trs80m3 tape
	// Should have ~256 0x55 then one 0x7f
	// It's possible that 0x57 could be encountered instead,
	//   but no working tapes with it have been found.
	// Other bit-shifted variants might exist too.
	while ((cas_size > data_pos) && (casdata[data_pos] == 0x55))
		data_pos++;
	if (casdata[data_pos] != 0x7f)
		return 0;

	data_pos = 0;
	while( data_pos < cas_size )
	{
		uint8_t data = casdata[data_pos];

		for (uint8_t i = 0; i < 8; i++ )
		{
			sample_count += trs80m3_cas_cycle( buffer, sample_count, data >> 7 );

			// This paragraph unscrambles and prints the SYSTEM name.
			// If the first character is U, type SYSTEM, then the next 6 characters; otherwise use CLOAD.
			if ((sw == 1) && buffer)
			{
				if (bitout)
				{
					byteout = (byteout << 1) | (data >> 7);
					if (bitout == 8)
					{
						if (byteout == 0)
							sw = 2;
						printf("%c",byteout);
						byteout = 0;
						bitout = 0;
					}
					else
						bitout++;
				}
				else bitout++;
			}

			data <<= 1;
		}

		if (!sw && (casdata[data_pos] != 0x55))
		{
			sw = 1;
			// This 1ms of silence isn't absolutely necessary, but the system
			// writes it, so we may as well emulate it.
			sample_count += trs80m1_cas_cycle( buffer, sample_count, false );
		}

		data_pos++;
	}

	// Specification requires a short silence to indicate EOF
	sample_count += trs80m1_cas_cycle( buffer, sample_count, false );
	sample_count += trs80m1_cas_cycle( buffer, sample_count, false );
	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int trs80_cas_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	if (cas_size && (bytes[0] == 0x55))
		return trs80m3_handle_cas( buffer, bytes );
	else
		return trs80m1_handle_cas( buffer, bytes );
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int trs80_cas_to_wav_size(const uint8_t *casdata, int caslen)
{
	cas_size = caslen;

	if (cas_size && (casdata[0] == 0x55))
		return trs80m3_handle_cas( nullptr, casdata );
	else
		return trs80m1_handle_cas( nullptr, casdata );
}

static const cassette_image::LegacyWaveFiller trs80l1_cas_legacy_fill_wave =
{
	trs80_cas_fill_wave,                    /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	trs80_cas_to_wav_size,                  /* chunk_sample_calc */
	22050,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static cassette_image::error trs80l1_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &trs80l1_cas_legacy_fill_wave);
}


static cassette_image::error trs80l1_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&trs80l1_cas_legacy_fill_wave);
}


static const cassette_image::Format trs80l1_cas_format =
{
	"cas",
	trs80l1_cas_identify,
	trs80l1_cas_load,
	nullptr
};

static const cassette_image::LegacyWaveFiller trs80l2_cas_legacy_fill_wave =
{
	trs80_cas_fill_wave,                    /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	trs80_cas_to_wav_size,                  /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};


static cassette_image::error trs80l2_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &trs80l2_cas_legacy_fill_wave);
}


static cassette_image::error trs80l2_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&trs80l2_cas_legacy_fill_wave);
}


static const cassette_image::Format trs80l2_cas_format =
{
	"cas",
	trs80l2_cas_identify,
	trs80l2_cas_load,
	nullptr
};


CASSETTE_FORMATLIST_START(trs80l1_cassette_formats)
	CASSETTE_FORMAT(trs80l1_cas_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(trs80l2_cassette_formats)
	CASSETTE_FORMAT(trs80l2_cas_format)
CASSETTE_FORMATLIST_END
