// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Camputers Lynx cassette images


We support TAP files used by the Pale and Jynx emulators.

Tape format:
- about 7 seconds of zeroes
- A5 byte
- 22 byte
- program name
- 22 byte
- about 7 seconds of zeroes
- A5 byte
- header
- main program
- checksum

Each byte is 8 bits (MSB first) with no start or stop bits.

********************************************************************/

#include "emu.h"   // for popmessage and <string>

#include "camplynx_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define LYNX48K_WAV_FREQUENCY   4000
#define LYNX128K_WAV_FREQUENCY  8000


// image size
static int camplynx_image_size;

static int camplynx_put_samples(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int camplynx_output_bit(INT16 *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += camplynx_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
		samples += camplynx_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
	}
	else
	{
		samples += camplynx_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		samples += camplynx_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
	}

	return samples;
}

static int camplynx_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;
	UINT8 i;

	/* data */
	for (i = 0; i<8; i++)
		samples += camplynx_output_bit (buffer, sample_pos + samples, (byte >> (7-i)) & 1);

	return samples;
}

static int camplynx_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	UINT32 sample_count = 0;
	UINT32 byte_count = 0;
	UINT32 i;

	/* header zeroes */
	for (i=0; i<555; i++)
		sample_count += camplynx_output_byte(buffer, sample_count, 0);

	if (bytes[0] == 0x22)
	{
		std::string pgmname = " LOAD \"";
		byte_count++;
		sample_count += camplynx_output_byte(buffer, sample_count, 0xA5);
		sample_count += camplynx_output_byte(buffer, sample_count, 0x22);

		/* program name - include protection in case tape is corrupt */
		for (i=1; bytes[i]!=0x22; i++)
		{
			if (i < camplynx_image_size)
			{
				sample_count += camplynx_output_byte(buffer, sample_count, bytes[i]);
				pgmname.append(1, (char)bytes[i]);
			}
			else
				return sample_count;
			byte_count++;
		}

		pgmname.append(1, (char)0x22);
		sample_count += camplynx_output_byte(buffer, sample_count, bytes[byte_count++]); // should be 0x22

		// if a machine-language program, say to use MLOAD
		if (bytes[byte_count] == 0x4D)
			pgmname[0] = (char)0x4D;

		// Tell user how to load the tape
		osd_printf_info("%s",pgmname.c_str());

		/* data zeroes */
		for (i=0; i<555; i++)
			sample_count += camplynx_output_byte(buffer, sample_count, 0);

		sample_count += camplynx_output_byte(buffer, sample_count, 0xA5);
	}

	/* data */
	for (i=byte_count; i<camplynx_image_size; i++)
		sample_count += camplynx_output_byte(buffer, sample_count, bytes[i]);

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int camplynx_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return camplynx_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int camplynx_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	camplynx_image_size = length;

	return camplynx_handle_cassette(nullptr, bytes);
}

static const struct CassetteLegacyWaveFiller lynx48k_legacy_fill_wave =
{
	camplynx_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	camplynx_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	LYNX48K_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static const struct CassetteLegacyWaveFiller lynx128k_legacy_fill_wave =
{
	camplynx_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	camplynx_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	LYNX128K_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static casserr_t lynx48k_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &lynx48k_legacy_fill_wave);
}

static casserr_t lynx128k_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &lynx128k_legacy_fill_wave);
}

static casserr_t lynx48k_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &lynx48k_legacy_fill_wave);
}

static casserr_t lynx128k_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &lynx128k_legacy_fill_wave);
}

static const struct CassetteFormat lynx48k_cassette_image_format =
{
	"tap",
	lynx48k_cassette_identify,
	lynx48k_cassette_load,
	nullptr
};

static const struct CassetteFormat lynx128k_cassette_image_format =
{
	"tap",
	lynx128k_cassette_identify,
	lynx128k_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(lynx48k_cassette_formats)
	CASSETTE_FORMAT(lynx48k_cassette_image_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(lynx128k_cassette_formats)
	CASSETTE_FORMAT(lynx128k_cassette_image_format)
CASSETTE_FORMATLIST_END
