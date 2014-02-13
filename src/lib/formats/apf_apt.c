// license:BSD-3-Clause
// copyright-holders: Original author, Robbbert
/********************************************************************

Support for APF Imagination Machine cassette images

CPF and CAS images consist of the screen and then the program,
and are exactly 1E00 bytes in length.

Each byte after conversion becomes bit 7,6,etc to 0, There are
no start or stop bits.

An actual tape consists of 6 sections
a. silence until you press Enter (no offset)
b. 11secs of high bits then 1 low bit
c. The screen ram
d. The program ram
e. A checksum byte (8-bit addition)

********************************************************************/

#include "formats/apf_apt.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

/* frequency of wave */
#define APF_WAV_FREQUENCY   8000

/* 500 microsecond of bit 0 and 1000 microsecond of bit 1 */
static int apf_image_size;

static int apf_put_samples(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int apf_output_bit(INT16 *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += apf_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
		samples += apf_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
	}
	else
	{
		samples += apf_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		samples += apf_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
	}

	return samples;
}

static int apf_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;
	UINT8 i;

	/* data */
	for (i = 0; i<8; i++)
		samples += apf_output_bit (buffer, sample_pos + samples, (byte >> (7-i)) & 1);

	return samples;
}

static int apf_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	UINT32 i;
	UINT8 cksm = 0;

	// silence
	sample_count += apf_put_samples(buffer, 0, 12000, 0);

	/* start */
	for (i=0; i<10000; i++)
		sample_count += apf_output_bit(buffer, sample_count, 1);

	sample_count += apf_output_bit(buffer, sample_count, 0);

	/* data */
	for (i=0; i<apf_image_size; i++)
	{
		cksm += bytes[i];
		sample_count += apf_output_byte(buffer, sample_count, bytes[i]);
	}

	/* checksum byte */
	sample_count += apf_output_byte(buffer, sample_count, cksm);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int apf_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return apf_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int apf_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	apf_image_size = length;

	return apf_handle_cassette(NULL, bytes);
}

static const struct CassetteLegacyWaveFiller apf_legacy_fill_wave =
{
	apf_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	apf_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	APF_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t apf_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &apf_legacy_fill_wave);
}

static casserr_t apf_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &apf_legacy_fill_wave);
}

static const struct CassetteFormat apf_apt_format =
{
	"cas,cpf",
	apf_cassette_identify,
	apf_cassette_load,
	NULL
};



CASSETTE_FORMATLIST_START(apf_cassette_formats)
	CASSETTE_FORMAT(apf_apt_format)
CASSETTE_FORMATLIST_END
