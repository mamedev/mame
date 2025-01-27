// license:BSD-3-Clause
// copyright-holders:Robbbert,Nigel Barnes
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

#include "camplynx_cas.h"

#include "multibyte.h"

#include "osdcore.h" // osd_printf_*


#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define LYNX48K_WAV_FREQUENCY   4000
#define LYNX128K_WAV_FREQUENCY  8000


// image size
static int camplynx_image_size;

static int camplynx_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int camplynx_output_bit(int16_t *buffer, int sample_pos, bool bit)
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

static int camplynx_output_byte(int16_t *buffer, int sample_pos, uint8_t byte)
{
	int samples = 0;
	uint8_t i;

	/* data */
	for (i = 0; i<8; i++)
		samples += camplynx_output_bit (buffer, sample_pos + samples, (byte >> (7-i)) & 1);

	return samples;
}

static int camplynx_handle_cassette(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;
	uint32_t byte_count = 0;
	uint32_t data_size = 0;
	uint32_t i;
	uint8_t file_type;
	std::string pgmname = "";

	while (byte_count < camplynx_image_size)
	{
		/* initial SYNC + A5 applies to all file types */
		for (i = 0; i < 555; i++)
			sample_count += camplynx_output_byte(buffer, sample_count, 0);
		sample_count += camplynx_output_byte(buffer, sample_count, 0xA5);

		/* some TAPs have a spurious A5 at the start, ignore */
		while (bytes[byte_count] == 0xA5)
			byte_count++;

		if (bytes[byte_count] == 0x22)
		{
			pgmname = " LOAD \"";
			byte_count++;
			sample_count += camplynx_output_byte(buffer, sample_count, 0x22);

			/* output program name - include protection in case tape is corrupt */
			for (i = byte_count; bytes[i] != 0x22; i++)
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

			/* read file type letter, should be 'B' or 'M' */
			file_type = bytes[byte_count];

			/* if a machine-language program, say to use MLOAD */
			if (file_type == 'M') pgmname[0] = 'M';

			/* tell user how to load the tape */
			if (buffer)
				osd_printf_info("%s\n", pgmname);

			/* second SYNC + A5 */
			for (i = 0; i < 555; i++)
				sample_count += camplynx_output_byte(buffer, sample_count, 0);
			sample_count += camplynx_output_byte(buffer, sample_count, 0xA5);
		}

		/* read file type letter, should be 'A', 'B' or 'M' */
		file_type = bytes[byte_count];

		/* determine the data size (as recorded in the file) + extra bytes per file type */
		switch (file_type)
		{
		case 'A':
			data_size = 5 + get_u16le(&bytes[byte_count + 3]) + 12;
			break;
		case 'B':
			data_size = 3 + get_u16le(&bytes[byte_count + 1]) + 3;
			break;
		case 'M':
			data_size = 3 + get_u16le(&bytes[byte_count + 1]) + 7;
			break;
		}

		/* output data  - include protection in case tape is corrupt */
		for (i = byte_count; i < byte_count + data_size; i++)
		{
			if (i < camplynx_image_size)
			{
				sample_count += camplynx_output_byte(buffer, sample_count, bytes[i]);
			}
		}
		byte_count += data_size;

		/* some TAPs have a spurious 00 at the end, ignore */
		while (bytes[byte_count] == 0x00)
			byte_count++;
	}

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int camplynx_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return camplynx_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int camplynx_cassette_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	camplynx_image_size = length;

	return camplynx_handle_cassette(nullptr, bytes);
}

static const cassette_image::LegacyWaveFiller lynx48k_legacy_fill_wave =
{
	camplynx_cassette_fill_wave,                 /* fill_wave */
	-1,                                          /* chunk_size */
	0,                                           /* chunk_samples */
	camplynx_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	LYNX48K_WAV_FREQUENCY,                       /* sample_frequency */
	0,                                           /* header_samples */
	0                                            /* trailer_samples */
};

static const cassette_image::LegacyWaveFiller lynx128k_legacy_fill_wave =
{
	camplynx_cassette_fill_wave,                 /* fill_wave */
	-1,                                          /* chunk_size */
	0,                                           /* chunk_samples */
	camplynx_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	LYNX128K_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                           /* header_samples */
	0                                            /* trailer_samples */
};

static cassette_image::error lynx48k_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &lynx48k_legacy_fill_wave);
}

static cassette_image::error lynx128k_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &lynx128k_legacy_fill_wave);
}

static cassette_image::error lynx48k_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&lynx48k_legacy_fill_wave);
}

static cassette_image::error lynx128k_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&lynx128k_legacy_fill_wave);
}

static const cassette_image::Format lynx48k_cassette_image_format =
{
	"tap",
	lynx48k_cassette_identify,
	lynx48k_cassette_load,
	nullptr
};

static const cassette_image::Format lynx128k_cassette_image_format =
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
