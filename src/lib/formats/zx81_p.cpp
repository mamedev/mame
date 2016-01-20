// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Krzysztof Strzecha, Robbbert
/*****************************************************************************

Taken from nocash ZX81 docs by Martin Korth.

ZX81 Cassette File Structure
  5 seconds    pilot
  1-127 bytes  name (bit7 set in last char)
  LEN bytes    data, loaded to address 4009h, LEN=(4014h)-4009h.
  1 pulse      video retrace signal (only if display was enabled)
The data field contains the system area, the basic program, the video memory,
and VARS area.

ZX80 Cassette File Structure
  5 seconds    pilot
  LEN bytes    data, loaded to address 4000h, LEN=(400Ah)-4000h.
ZX80 files do not have filenames, and video memory is not included in the file.

Bits and Bytes
Each byte consists of 8 bits (MSB first) without any start and stop bits,
directly followed by the next byte. A "0" bit consists of four high pulses,
a "1" bit of nine pulses, either one followed by a silence period.
  0:  /\/\/\/\________
  1:  /\/\/\/\/\/\/\/\/\________
Each pulse is split into a 150us High period, and 150us Low period. The
duration of the silence between each bit is 1300us. The baud rate is thus 400
bps (for a "0" filled area) downto 250 bps (for a "1" filled area). Average
medium transfer rate is approx. 307 bps (38 bytes/sec) for files that contain
50% of "0" and "1" bits each.

*****************************************************************************/

#include <assert.h>

#include "zx81_p.h"


#define WAVEENTRY_LOW   -32768
#define WAVEENTRY_HIGH   32767
#define WAVEENTRY_ZERO       0

#define ZX81_WAV_FREQUENCY  44100

/* all following are in samples */
#define ZX81_PULSE_LENGTH   16
#define ZX81_PAUSE_LENGTH   56
#define ZX81_PILOT_LENGTH   220500

#define ZX81_LOW_BIT_LENGTH (ZX81_PULSE_LENGTH*4+ZX81_PAUSE_LENGTH)
#define ZX81_HIGH_BIT_LENGTH    (ZX81_PULSE_LENGTH*9+ZX81_PAUSE_LENGTH)

#define ZX81_START_LOAD_ADDRESS 0x4009
#define ZX80_START_LOAD_ADDRESS 0x4000
#define ZX81_DATA_LENGTH_OFFSET 0x0b
#define ZX80_DATA_LENGTH_OFFSET 0x04

static UINT8 zx_file_name[128];
static UINT16 real_data_length = 0;
static UINT8 zx_file_name_length = 0;

/* common functions */

static INT16 *zx81_emit_level(INT16 *p, int count, int level)
{
	int i;

	for (i=0; i<count; i++) *(p++) = level;

	return p;
}

static INT16* zx81_emit_pulse(INT16 *p)
{
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_LOW);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_LOW);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_ZERO);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_HIGH);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_HIGH);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_ZERO);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_LOW);
	p = zx81_emit_level (p, ZX81_PULSE_LENGTH/8, WAVEENTRY_LOW);

	return p;
}

static INT16* zx81_emit_pause(INT16 *p)
{
	p = zx81_emit_level (p, ZX81_PAUSE_LENGTH, WAVEENTRY_ZERO);

	return p;
}

static INT16* zx81_output_bit(INT16 *p, UINT8 bit)
{
	int i;

	if (bit)
		for (i=0; i<9; i++)
			p = zx81_emit_pulse (p);
	else
		for (i=0; i<4; i++)
			p = zx81_emit_pulse (p);

	p = zx81_emit_pause(p);

		return p;
}

static INT16* zx81_output_byte(INT16 *p, UINT8 byte)
{
	int i;

	for (i=0; i<8; i++)
		p = zx81_output_bit(p,(byte>>(7-i)) & 0x01);

	return p;
}

static UINT16 zx81_cassette_calculate_number_of_1(const UINT8 *bytes, UINT16 length)
{
	UINT16 number_of_1 = 0;
	int i,j;

	for (i=0; i<length; i++)
		for (j=0; j<8; j++)
			if ((bytes[i]>>j)&0x01)
				number_of_1++;

	return number_of_1;
}

/* ZX-81 functions */

static const UINT8 zx81_chars[]={
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*00h-07h*/
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*08h-0fh*/
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*10h-17h*/
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*18h-1fh*/
				0x00, 0x00, 0x0b, 0x00, 0x0d, 0x00, 0x00, 0x00, /*20h-27h*/
				0x10, 0x11, 0x17, 0x15, 0x1a, 0x16, 0x1b, 0x18, /*28h-2fh*/
				0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, /*30h-37h*/
				0x24, 0x25, 0x0e, 0x19, 0x13, 0x14, 0x12, 0x0f, /*38h-3fh*/
				0x00, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, /*40h-47h*/
				0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, /*48h-4fh*/
				0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, /*50h-57h*/
				0x3d, 0x3e, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, /*58h-5fh*/
				0x00, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, /*60h-67h*/
				0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, /*68h-6fh*/
				0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, /*70h-77h*/
				0x3d, 0x3e, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, /*78h-7fh*/
};

static void zx81_fill_file_name(const char* name)
{
	for (zx_file_name_length=0; (zx_file_name_length<128) && name[zx_file_name_length]; zx_file_name_length++)
		zx_file_name[zx_file_name_length] = ((UINT8) name[zx_file_name_length]<0x80) ? zx81_chars[(UINT8) name[zx_file_name_length]] : 0x00;
	zx_file_name[zx_file_name_length-1] |= 0x80;
}

static int zx81_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	unsigned int number_of_0_data = 0;
	unsigned int number_of_1_data = 0;
	unsigned int number_of_0_name = 0;
	unsigned int number_of_1_name = 0;

	real_data_length = bytes[ZX81_DATA_LENGTH_OFFSET] + bytes[ZX81_DATA_LENGTH_OFFSET+1]*256 - ZX81_START_LOAD_ADDRESS;

	number_of_1_data = zx81_cassette_calculate_number_of_1(bytes, real_data_length);
	number_of_0_data = length*8-number_of_1_data;

	number_of_1_name = zx81_cassette_calculate_number_of_1(zx_file_name, zx_file_name_length);
	number_of_0_name = zx_file_name_length*8-number_of_1_name;

	return (number_of_0_data+number_of_0_name)*ZX81_LOW_BIT_LENGTH + (number_of_1_data+number_of_1_name)*ZX81_HIGH_BIT_LENGTH + ZX81_PILOT_LENGTH;
}

static int zx81_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	INT16 * p = buffer;
	int i;

	/* pilot */
	p = zx81_emit_level (p, ZX81_PILOT_LENGTH, WAVEENTRY_ZERO);

	/* name */
	for (i=0; i<zx_file_name_length; i++)
		p = zx81_output_byte(p, zx_file_name[i]);

	/* data */
	for (i=0; i<real_data_length; i++)
		p = zx81_output_byte(p, bytes[i]);

	return p - buffer;
}

static const struct CassetteLegacyWaveFiller zx81_legacy_fill_wave =
{
	zx81_cassette_fill_wave,            /* fill_wave */
	-1,                     /* chunk_size */
	0,                      /* chunk_samples */
	zx81_cassette_calculate_size_in_samples,    /* chunk_sample_calc */
	ZX81_WAV_FREQUENCY,             /* sample_frequency */
	0,                      /* header_samples */
	0                       /* trailer_samples */
};

static casserr_t zx81_p_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &zx81_legacy_fill_wave);
}

static casserr_t zx81_p_load(cassette_image *cassette)
{
	/* The filename of the file is used to create the wave stream for the emulated machine. Why is this information not
	   part of the image file itself?
	   Hardcoding this to "cassette".
	*/
	zx81_fill_file_name ("cassette" /*image_basename_noext(device_list_find_by_tag( Machine->config->m_devicelist, CASSETTE, "cassette" ))*/ );
	return cassette_legacy_construct(cassette, &zx81_legacy_fill_wave);
}

static const struct CassetteFormat zx81_p_image_format =
{
	"p,81",
	zx81_p_identify,
	zx81_p_load,
	nullptr
};

CASSETTE_FORMATLIST_START(zx81_p_format)
	CASSETTE_FORMAT(zx81_p_image_format)
CASSETTE_FORMATLIST_END

/* ZX-80 functions */

static int zx80_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	unsigned int number_of_0_data = 0;
	unsigned int number_of_1_data = 0;

	real_data_length = bytes[ZX80_DATA_LENGTH_OFFSET] + bytes[ZX80_DATA_LENGTH_OFFSET+1]*256 - ZX80_START_LOAD_ADDRESS - 1;

	number_of_1_data = zx81_cassette_calculate_number_of_1(bytes, real_data_length);
	number_of_0_data = length*8-number_of_1_data;

	return number_of_0_data*ZX81_LOW_BIT_LENGTH + number_of_1_data*ZX81_HIGH_BIT_LENGTH + ZX81_PILOT_LENGTH;
}

static int zx80_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	INT16 * p = buffer;
	int i;

	/* pilot */
	p = zx81_emit_level (p, ZX81_PILOT_LENGTH, WAVEENTRY_ZERO);

	/* data */
	for (i=0; i<real_data_length; i++)
		p = zx81_output_byte(p, bytes[i]);

	return p - buffer;
}

static const struct CassetteLegacyWaveFiller zx80_legacy_fill_wave =
{
	zx80_cassette_fill_wave,            /* fill_wave */
	-1,                                         /* chunk_size */
	0,                                          /* chunk_samples */
	zx80_cassette_calculate_size_in_samples,    /* chunk_sample_calc */
	ZX81_WAV_FREQUENCY,                                     /* sample_frequency */
	0,                                          /* header_samples */
	0                                           /* trailer_samples */
};

static casserr_t zx80_o_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &zx80_legacy_fill_wave);
}

static casserr_t zx80_o_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &zx80_legacy_fill_wave);
}

static const struct CassetteFormat zx80_o_image_format =
{
	"o,80",
	zx80_o_identify,
	zx80_o_load,
	nullptr
};

CASSETTE_FORMATLIST_START(zx80_o_format)
	CASSETTE_FORMAT(zx80_o_image_format)
CASSETTE_FORMATLIST_END
