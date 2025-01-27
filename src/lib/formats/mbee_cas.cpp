// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************

Support for Microbee cassette images

Microbee tapes consist of 3 sections
1. A leader of 63 zeroes
2. A header which contains the program name and other info
3. The main program

Each byte after conversion becomes a start bit, bit 0,1,etc to 7,
then 2 stop bits.

At 1200 baud, a high = 2 cycles of 2400Hz and a low = 1 cycle of 1200Hz
At 300 baud, a high = 8 cycles of 2400Hz and a low = 4 cycles of 1200Hz

The header bytes are arranged thus:
1 (SOH) 0x01
6 File name
1 file type (M=machine language, B=Basic)
2 length
2 load address
2 exec address
1 tape speed (0 = 300 baud; other = 1200 baud)
1 auto-start (0 = no)
1 unassigned byte
1 CRC byte

The header is always at 300 baud; the program will be at the
speed indicated by the speed byte.

By coincidence (or not), the header is the same format as that
of the Sorcerer and SOL-20. In these, the speed and auto-start
bytes are unassigned. The CRC uses the same algorithm.

The main program is broken into blocks of 256, with each block
having its own CRC byte.

Microbee tape and quickload formats:

BEE - straight binary dump to address 0900, no header. For Machine
      Language programs.

BIN - the standard z80bin format.

COM - straight binary dump to address 0100, no header. For Machine
      Language programs.

MWB - straight binary dump to address 08C0, no header. For BASIC
      programs.

TAP - has an ID header of TAP_DGOS_BEE or MBEE, null terminated.
      This is followed by the binary dump with the leader and CRC
      bytes included.

********************************************************************/

#include "mbee_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define MBEE_WAV_FREQUENCY   9600

// image size
static int mbee_image_size;
static bool mbee_speed;

static int mbee_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int mbee_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;
	if (mbee_speed)
	{
		if (bit)
		{
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		}
		else
		{
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
		}
	}
	else
	{
		if (bit)
		{
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 2, WAVEENTRY_HIGH);
		}
		else
		{
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_LOW);
			samples += mbee_put_samples(buffer, sample_pos + samples, 4, WAVEENTRY_HIGH);
		}
	}

	return samples;
}

static int mbee_output_byte(int16_t *buffer, int sample_pos, uint8_t byte)
{
	int samples = 0;
	uint8_t i;

	/* start */
	samples += mbee_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (i = 0; i<8; i++)
		samples += mbee_output_bit (buffer, sample_pos + samples, (byte >> i) & 1);

	/* stop */
	for (i = 0; i<2; i++)
		samples += mbee_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static int mbee_handle_tap(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;
	uint32_t byte_count = 0;
	uint32_t i = 0;
	bool temp_speed = 0;
	uint8_t temp_blocks = 0;
	uint16_t temp_size = 0;

	// TAP file starts with a null-terminate ID string. We just skip this.
	while (bytes[byte_count])
		byte_count++;

	// there can be a library of files, loop through them all
	while (byte_count < mbee_image_size)
	{
		mbee_speed = 0;

		// now output the leader
		while ( (!bytes[byte_count]) && (byte_count < mbee_image_size) )
			sample_count += mbee_output_byte(buffer, sample_count, bytes[byte_count++]);

		// make sure SOH is where we expect
		if (bytes[byte_count] != 1 )
			break;

		// store the size for later
		temp_blocks = bytes[byte_count + 9];
		temp_size = bytes[byte_count + 8] + temp_blocks*256;

		// store the speed for later
		temp_speed = (bytes[byte_count + 15]) ? 1 : 0;

		/* header */
		for (i=0; i<18; i++)
			sample_count += mbee_output_byte(buffer, sample_count, bytes[byte_count++]);

		// change speed
		mbee_speed = temp_speed;

		// calculate size of program including CRC bytes
		temp_size = temp_size + temp_blocks + 1;

		/* data */
		for (i=0; i<temp_size; i++)
			sample_count += mbee_output_byte(buffer, sample_count, bytes[byte_count++]);

	}

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int mbee_tap_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return mbee_handle_tap(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int mbee_tap_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	mbee_image_size = length;

	return mbee_handle_tap(nullptr, bytes);
}

static const cassette_image::LegacyWaveFiller mbee_tap_config =
{
	mbee_tap_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	mbee_tap_calculate_size_in_samples, /* chunk_sample_calc */
	MBEE_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error mbee_tap_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &mbee_tap_config);
}

static cassette_image::error mbee_tap_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&mbee_tap_config);
}

static const cassette_image::Format mbee_tap_image_format =
{
	"tap",
	mbee_tap_identify,
	mbee_tap_load,
	nullptr
};

CASSETTE_FORMATLIST_START(mbee_cassette_formats)
	CASSETTE_FORMAT(mbee_tap_image_format)
CASSETTE_FORMATLIST_END
