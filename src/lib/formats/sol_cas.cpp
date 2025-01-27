// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for SOL-20 cassette images


SOL20 tapes consist of these sections:
1. A high tone whenever idle
2. A header
3. The data, in blocks of 256 bytes plus a CRC byte
4. The last block may be shorter, depending on the number of bytes
   left to save.

Each byte has 1 start bit, 8 data bits (0-7), 2 stop bits.

The default speed is 1200 baud, which is what we emulate here.
A high bit is 1 cycle of 1200 Hz, while a low bit is half a cycle
of 600 Hz.

Formats:
SVT - The full explanation may be found on the Solace web site,
      however this is a summary of what we support.
      C (carrier) time in decaseconds
      D (data bytes) in ascii text
      H (header) tape header info
      Multiple programs
      Unsupported:
      B (set baud rate) B 300 or B 1200
      F load ENT file
      S (silence) time in decaseconds
      bad-byte symbols
      escaped characters

********************************************************************/

#include "sol_cas.h"

#include "multibyte.h"


#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define SOL20_WAV_FREQUENCY   4800

// image size
static uint32_t sol20_image_size; // FIXME: global variable prevent multiple instances
static bool level;
static uint8_t sol20_cksm_byte;
static uint32_t sol20_byte_num;
static uint8_t sol20_header[16];

static int sol20_put_samples(int16_t *buffer, int sample_pos, int count)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level ? WAVEENTRY_LOW : WAVEENTRY_HIGH;

		level ^= 1;
	}

	return count;
}

static int sol20_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	if (bit)
	{
		samples += sol20_put_samples(buffer, sample_pos + samples, 2);
		samples += sol20_put_samples(buffer, sample_pos + samples, 2);
	}
	else
	{
		samples += sol20_put_samples(buffer, sample_pos + samples, 4);
	}

	return samples;
}

static int sol20_output_byte(int16_t *buffer, int sample_pos, uint8_t byte)
{
	int samples = 0;

	/* start */
	samples += sol20_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (uint8_t i = 0; i<8; i++)
		samples += sol20_output_bit (buffer, sample_pos + samples, (byte >> i) & 1);

	/* stop */
	for (uint8_t i = 0; i<2; i++)
		samples += sol20_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

// Calculate checksum
static uint8_t sol20_calc_cksm(uint8_t cksm, uint8_t data)
{
	data -= cksm;
	cksm = data;
	data ^= cksm;
	data ^= 0xff;
	data -= cksm;
	return data;
}

// Ignore remainder of line
static void sol20_scan_to_eol(const uint8_t *bytes)
{
	bool t = 1;
	while (t)
	{
		if (sol20_byte_num >= sol20_image_size)
		{
			sol20_byte_num = 0;
			t = 0;
		}
		else
		if (bytes[sol20_byte_num] == 0x0d)
			t = 0;
		else
			sol20_byte_num++;
	}
}

// skip spaces and symbols looking for a hex digit
static void sol20_scan_to_hex(const uint8_t *bytes)
{
	bool t = 1;
	while (t)
	{
		if (sol20_byte_num >= sol20_image_size)
		{
			sol20_byte_num = 0;
			t = 0;
		}
		else
		{
			uint8_t chr = bytes[sol20_byte_num];
			if (chr == 0x0d)
				t = 0;
			else
			if (((chr >= '0') && (chr <= '9')) || ((chr >= 'A') && (chr <= 'F')))
				t = 0;
			else
				sol20_byte_num++;
		}
	}
}

// Turn n digits into hex
static int sol20_read_hex(const uint8_t *bytes, uint8_t numdigits)
{
	int data = 0;

	for (uint8_t i = 0; i < numdigits; i++)
	{
		uint8_t chr = bytes[sol20_byte_num];
		if ((chr >= '0') && (chr <= '9'))
		{
			data = (data << 4) | (chr-48);
			sol20_byte_num++;
		}
		else
		if ((chr >= 'A') && (chr <= 'F'))
		{
			data = (data << 4) | (chr-55);
			sol20_byte_num++;
		}
		else
			i = numdigits;
	}
	return data;
}

// Turn digits into decimal
static int sol20_read_dec(const uint8_t *bytes)
{
	int data = 0;

	while ((bytes[sol20_byte_num] >= '0') && (bytes[sol20_byte_num] <= '9'))
	{
		data = data*10 + bytes[sol20_byte_num] - 48;
		sol20_byte_num++;
	}

	return data;
}

static int sol20_handle_cassette(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;
	uint32_t t = 0;
	uint16_t cc = 0;
	sol20_byte_num = 1;
	bool process_d = 0;
	uint16_t length = 0;

	// 1st line of file must say SVT
	if ((bytes[0] == 'S') && (bytes[1] == 'V') && (bytes[2] == 'T'))
	{ }
	else
		return sample_count;

	// ignore remainder of line
	sol20_scan_to_eol(bytes);

	// process the commands
	while (sol20_byte_num)
	{
		sol20_byte_num+=2; // bump to start of next line
		uint8_t chr = bytes[sol20_byte_num];  // Get command
		if (sol20_byte_num >= sol20_image_size)
			sol20_byte_num = 0;
		else
		{
			switch (chr)
			{
				case 0x0d:
					break;
				case 'C': // carrier
					{
						if (cc) // if this is the next file, clean up after the previous one
						{
							sample_count += sol20_output_byte(buffer, sample_count, sol20_cksm_byte); // final checksum if needed
							cc = 0;
						}

						sol20_byte_num+=2; // bump to parameter
						t = sol20_read_dec(bytes) * 140; // convert 10th of seconds to number of ones
						for (uint32_t i = 0; i < t; i++)
							sample_count += sol20_output_bit(buffer, sample_count, 1);
						sol20_scan_to_eol(bytes);
						break;
					}
				case 'H': // header
					{
						if (cc) // if this is the next file, clean up after the previous one
						{
							sample_count += sol20_output_byte(buffer, sample_count, sol20_cksm_byte); // final checksum if needed
							cc = 0;
						}

						sol20_byte_num+=2; // bump to file name
						for (uint32_t i = 0; i < 5; i++)
							sol20_header[i] = 0x20;
						for (uint32_t i = 0; i < 5; i++)
						{
							sol20_header[i] = bytes[sol20_byte_num++];
							if (sol20_header[i] == 0x20)
								break;
						}
						sol20_header[5] = 0;
						sol20_scan_to_hex(bytes); // bump to file type
						sol20_header[6] = sol20_read_hex(bytes, 2);
						sol20_scan_to_hex(bytes); // bump to length
						length = sol20_read_hex(bytes, 4);
						put_u16le(&sol20_header[7], length);
						sol20_scan_to_hex(bytes); // bump to load-address
						put_u16le(&sol20_header[9], sol20_read_hex(bytes, 4));
						sol20_scan_to_hex(bytes); // bump to exec-address
						put_u16le(&sol20_header[11], sol20_read_hex(bytes, 4));
						sol20_header[13] = 0;
						sol20_header[14] = 0;
						sol20_header[15] = 0;
						sol20_cksm_byte = 0;
						for (uint32_t i = 0; i < 16; i++)
							sol20_cksm_byte = sol20_calc_cksm(sol20_cksm_byte, sol20_header[i]);
						// write leader
						for (uint32_t i = 0; i < 100; i++)
							sample_count += sol20_output_byte(buffer, sample_count, 0);
						// write SOH
						sample_count += sol20_output_byte(buffer, sample_count, 1);
						// write Header
						for (uint32_t i = 0; i < 16; i++)
							sample_count += sol20_output_byte(buffer, sample_count, sol20_header[i]);
						// write checksum
						sample_count += sol20_output_byte(buffer, sample_count, sol20_cksm_byte);

						sol20_cksm_byte = 0;
						process_d = 1;
						sol20_scan_to_eol(bytes);
						break;
					}
				case 'D':  // data
					{
						sol20_byte_num+=2; // bump to first byte
						while ((bytes[sol20_byte_num] != 0x0d) && sol20_byte_num && process_d)
						{
							t = sol20_read_hex(bytes, 2);
							sample_count += sol20_output_byte(buffer, sample_count, t);
							cc++;
							// if it's a data byte reduce remaining length and calculate checksum;
							// tape supplies checksums except last one
							if (cc < 257)
							{
								length--;
								sol20_cksm_byte = sol20_calc_cksm(sol20_cksm_byte, t);
							}
							else
							// didnt need it, throw away
							{
								cc = 0;
								sol20_cksm_byte = 0;
							}
							// see if finished tape
							if (!length)
								process_d = 0;
							// bump to next byte
							sol20_scan_to_hex(bytes);
						}
					}
					[[fallthrough]];
				default:  // everything else is ignored
					sol20_scan_to_eol(bytes);
					break;
			}
		}
	}

	if (cc)  // reached the end of the svt file
		sample_count += sol20_output_byte(buffer, sample_count, sol20_cksm_byte); // final checksum if needed

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int sol20_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return sol20_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int sol20_cassette_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	sol20_image_size = length;

	return sol20_handle_cassette(nullptr, bytes);
}

static const cassette_image::LegacyWaveFiller sol20_legacy_fill_wave =
{
	sol20_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	sol20_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	SOL20_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error sol20_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &sol20_legacy_fill_wave);
}

static cassette_image::error sol20_cassette_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&sol20_legacy_fill_wave);
}

static const cassette_image::Format sol20_cassette_image_format =
{
	"svt",
	sol20_cassette_identify,
	sol20_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(sol20_cassette_formats)
	CASSETTE_FORMAT(sol20_cassette_image_format)
CASSETTE_FORMATLIST_END
