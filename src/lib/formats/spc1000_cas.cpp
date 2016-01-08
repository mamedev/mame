// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Samsung SPC-1000/SPC-1500 cassette images


Tape formats:

TAP: This is a series of 0x30 and 0x31 bytes, representing binary
     0 and 1. It includes the header and leaders.

CAS: Files in this format consist of a 16 bytes header (SPC-1000.CASfmt )
     followed by cassette bits packed together (each byte of a .cas file
     are 8 bits, most significant bit first)
	 
SPC: This format contains real bytes without header/body leaders, 
     checksum and delimiter '1' for each byte (Miso Kim).

STA: This format has not been investigated yet, but is assumed to
     be the save state of some other emulator.

IPL: This seems a quickload format containing RAM dump, not a real tape

********************************************************************/

#include <assert.h>

#include "spc1000_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define SPC1000_WAV_FREQUENCY   17000

// image size
static int spc1000_image_size;

static int spc1000_put_samples(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int spc1000_output_bit(INT16 *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += spc1000_put_samples(buffer, sample_pos + samples, 9, WAVEENTRY_LOW);
		samples += spc1000_put_samples(buffer, sample_pos + samples, 9, WAVEENTRY_HIGH);
	}
	else
	{
		samples += spc1000_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
		samples += spc1000_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
	}

	return samples;
}

static int spc1000_output_bits(INT16 *buffer, int sample_pos, bool bit, int length)
{
	int samples = 0;
	for (int i = 0; i < length; i++)
		samples += spc1000_output_bit(buffer, sample_pos + samples, bit);
	return samples;
}

static int spc1000_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;
	for (int i = 0; i < 8; i++)
		samples += spc1000_output_bit(buffer, sample_pos + samples, ((byte >> (7 - i)) & 1));
	samples += spc1000_output_bit(buffer, sample_pos + samples, 1);
	return samples;
}


static int spc1000_handle_tap(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;

	/* data */
	for (UINT32 i = 0; i < spc1000_image_size; i++)
		sample_count += spc1000_output_bit(buffer, sample_count, bytes[i] & 1);

	return sample_count;
}

static int spc1000_handle_cas(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;

	/* data (skipping first 16 bytes, which is CAS header) */
	for (UINT32 i = 0x1; i < spc1000_image_size; i++)
		for (int j = 0; j < 8; j++)
			sample_count += spc1000_output_bit(buffer, sample_count, (bytes[i] >> (7 - j)) & 1);

	return sample_count;
}

#define FILL_SAMPLE(bit)     sample_count += spc1000_output_bit(buffer, sample_count, bit)
#define FILL_SAMPLES(bit, n) sample_count += spc1000_output_bits(buffer, sample_count, bit, n)

static int spc1000_handle_spc(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	int cs = 0;

	/* make header leader virtually for spc format */
	FILL_SAMPLES(0,100);
	FILL_SAMPLES(1,40);
	FILL_SAMPLES(0,40);
	FILL_SAMPLES(1,2);
	for (UINT32 i = 0x0; i < 0x80; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			sample_count += spc1000_output_bit(buffer, sample_count, (bytes[i] >> (7 - j)) & 1);
			cs += (bytes[i] >> (7 - j)) & 1 ? 1 : 0;
		}
		FILL_SAMPLE(1);
	}
	
	/* put checksum data virtually for spc format */
	sample_count += spc1000_output_byte(buffer, sample_count, cs >> 8);
	sample_count += spc1000_output_byte(buffer, sample_count, cs & 0xff);

	/* make body leader virtually for spc format */
	FILL_SAMPLES(0,400);
	FILL_SAMPLES(1,20);
	FILL_SAMPLES(0,20);
	FILL_SAMPLES(1,2);
	cs = 0;
	for (UINT32 i = 0x80; i < spc1000_image_size; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			sample_count += spc1000_output_bit(buffer, sample_count, (bytes[i] >> (7 - j)) & 1);
			cs += (bytes[i] >> (7 - j)) & 1 ? 1 : 0;
		}
		FILL_SAMPLE(1);
	}

	/* put checksum data virtually for spc format */
	sample_count += spc1000_output_byte(buffer, sample_count, cs >> 8);
	sample_count += spc1000_output_byte(buffer, sample_count, cs & 0xff);
	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int spc1000_tap_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return spc1000_handle_tap(buffer, bytes);
}

static int spc1000_cas_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return spc1000_handle_cas(buffer, bytes);
}

static int spc1000_spc_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return spc1000_handle_spc(buffer, bytes);
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int spc1000_tap_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	spc1000_image_size = length;

	return spc1000_handle_tap(nullptr, bytes);
}

static int spc1000_cas_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	spc1000_image_size = length;

	return spc1000_handle_cas(nullptr, bytes);
}

static int spc1000_spc_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	spc1000_image_size = length;

	return spc1000_handle_spc(nullptr, bytes);
}


/*******************************************************************
   Formats
 ********************************************************************/


// TAP
static const struct CassetteLegacyWaveFiller spc1000_tap_legacy_fill_wave =
{
	spc1000_tap_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	spc1000_tap_calculate_size_in_samples, /* chunk_sample_calc */
	SPC1000_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t spc1000_tap_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &spc1000_tap_legacy_fill_wave);
}

static casserr_t spc1000_tap_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &spc1000_tap_legacy_fill_wave);
}

static const struct CassetteFormat spc1000_tap_cassette_image_format =
{
	"tap",
	spc1000_tap_cassette_identify,
	spc1000_tap_cassette_load,
	nullptr
};


// CAS
static const struct CassetteLegacyWaveFiller spc1000_cas_legacy_fill_wave =
{
	spc1000_cas_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	spc1000_cas_calculate_size_in_samples, /* chunk_sample_calc */
	SPC1000_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t spc1000_cas_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &spc1000_cas_legacy_fill_wave);
}

static casserr_t spc1000_cas_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &spc1000_cas_legacy_fill_wave);
}

static const struct CassetteFormat spc1000_cas_cassette_image_format =
{
	"cas",
	spc1000_cas_cassette_identify,
	spc1000_cas_cassette_load,
	nullptr
};


// SPC
static const struct CassetteLegacyWaveFiller spc1000_spc_legacy_fill_wave =
{
	spc1000_spc_fill_wave,                 /* fill_wave */
	-1,                                    /* chunk_size */
	0,                                     /* chunk_samples */
	spc1000_spc_calculate_size_in_samples, /* chunk_sample_calc */
	SPC1000_WAV_FREQUENCY,                 /* sample_frequency */
	0,                                     /* header_samples */
	0                                      /* trailer_samples */
};

static casserr_t spc1000_spc_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &spc1000_spc_legacy_fill_wave);
}

static casserr_t spc1000_spc_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &spc1000_spc_legacy_fill_wave);
}

static const struct CassetteFormat spc1000_spc_cassette_image_format =
{
	"spc",
	spc1000_spc_cassette_identify,
	spc1000_spc_cassette_load,
	nullptr
};


CASSETTE_FORMATLIST_START(spc1000_cassette_formats)
	CASSETTE_FORMAT(spc1000_tap_cassette_image_format)
	CASSETTE_FORMAT(spc1000_cas_cassette_image_format)
	CASSETTE_FORMAT(spc1000_spc_cassette_image_format)
CASSETTE_FORMATLIST_END
