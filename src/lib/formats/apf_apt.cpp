// license:BSD-3-Clause
// copyright-holders: Original author, Robbbert
/********************************************************************

Support for APF Imagination Machine cassette images

CPF and CAS images consist of the screen and then the program,
and are exactly 1E00 bytes in length.

APT images are much the same, however it includes a series of FF
bytes as a header. There's also a large amount of what seems to
be rubbish at the end.

APW images are not emulated, and are used by the closed-source
emulator APF_EMUW. Quote: "They allow recording in special formats
and recording audio. They are audio files sampled at 11025 Hz 8 bits
unsigned mono, without header. The bit 1 stores the state of the
recording head."

S19 images are not emulated, however there's no need to as they
are only used to hold cartridge hex dumps.

TXT images can be copy/pasted by using the Paste menu option.

Each byte after conversion becomes bit 7,6,etc to 0, There are
no start or stop bits.

An actual tape consists of 6 sections
a. silence until you press Enter (no offset)
b. 11secs of high bits then 1 low bit
c. The screen ram
d. The program ram
e. A checksum byte (8-bit addition)

********************************************************************/

#include <assert.h>

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

static int apf_apt_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	UINT32 i;
	UINT8 cksm = 0;
	UINT32 temp = 0;

	// silence
	sample_count += apf_put_samples(buffer, 0, 12000, 0);

	for (i=0; i<apf_image_size; i++)
	{
		sample_count += apf_output_byte(buffer, sample_count, bytes[i]);
		if (bytes[i]==0xfe)
		{
			temp = i+1;
			i = apf_image_size;
		}
	}

	/* data */
	for (i= temp; i<(temp+0x1e00); i++)
	{
		cksm += bytes[i];
		sample_count += apf_output_byte(buffer, sample_count, bytes[i]);
	}

	/* checksum byte */
	sample_count += apf_output_byte(buffer, sample_count, cksm);

	return sample_count;
}

static int apf_cpf_handle_cassette(INT16 *buffer, const UINT8 *bytes)
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

static int apf_apt_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return apf_apt_handle_cassette(buffer, bytes);
}

static int apf_cpf_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return apf_cpf_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int apf_apt_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	apf_image_size = length;

	return apf_apt_handle_cassette(nullptr, bytes);
}

static int apf_cpf_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	apf_image_size = length;

	return apf_cpf_handle_cassette(nullptr, bytes);
}

//*********************************************************************************

static const struct CassetteLegacyWaveFiller apf_cpf_fill_intf =
{
	apf_cpf_fill_wave,                      /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	apf_cpf_calculate_size_in_samples,      /* chunk_sample_calc */
	APF_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t apf_cpf_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &apf_cpf_fill_intf);
}

static casserr_t apf_cpf_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &apf_cpf_fill_intf);
}

static const struct CassetteFormat apf_cpf_format =
{
	"cas,cpf",
	apf_cpf_identify,
	apf_cpf_load,
	nullptr
};

//*********************************************************************************

static const struct CassetteLegacyWaveFiller apf_apt_fill_intf =
{
	apf_apt_fill_wave,                      /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	apf_apt_calculate_size_in_samples,      /* chunk_sample_calc */
	APF_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t apf_apt_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &apf_apt_fill_intf);
}

static casserr_t apf_apt_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &apf_apt_fill_intf);
}

static const struct CassetteFormat apf_apt_format =
{
	"apt",
	apf_apt_identify,
	apf_apt_load,
	nullptr
};

//*********************************************************************************

CASSETTE_FORMATLIST_START(apf_cassette_formats)
	CASSETTE_FORMAT(apf_cpf_format)
	CASSETTE_FORMAT(apf_apt_format)
CASSETTE_FORMATLIST_END
