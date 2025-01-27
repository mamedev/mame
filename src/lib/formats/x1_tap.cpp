// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *
 *   Sharp X1 TAP image format
 *
 *   "New" header format:
 *   0x00: Index - must be "TAPE" (4 bytes)
 *   0x04: Image title. (null-terminated string, 17 bytes)
 *   0x15: Reserved (5 bytes)
 *   0x1a: Write protect (bit 4, 1 byte)
 *   0x1b: Format (bit 0, 1 byte)
 *            if bit 0 is high, uses "speed limit sampling method"
 *   0x1c: Sample rate, per bit (in Hz, 4 bytes)
 *   0x20: Tape data size, in bits (4 bytes)
 *   0x24: Tape position (4 bytes, usually 0)
 *   0x28: Tape data (data size / 8)
 *
 *   "Old" header format:
 *   0x00: Sampling rate (4 bytes)
 *
 */
#include "x1_tap.h"
#include "imageutl.h"

#include "multibyte.h"

#include <cstring>


#define WAVE_HIGH        0x5a9e
#define WAVE_LOW        -0x5a9e

static int cas_size; // FIXME: global variables prevent multiple instances
static int samplerate;
static int new_format;

static int x1_fill_wave(int16_t* buffer, uint8_t data, int sample_pos)
{
	int x;
	int sample_count = 0;

	// one byte = 8 samples
	for(x=0;x<8;x++)
	{
		if(buffer)
			buffer[sample_pos+x] = (data & (1 << (7-x))) ? WAVE_HIGH : WAVE_LOW;
		sample_count++;
	}

	return sample_count;
}

static int x1_handle_tap(int16_t* buffer, const uint8_t* casdata)
{
	int sample_count = 0;
	int data_pos = new_format ? 0x28 : 0x04;

	if (samplerate != 8000)
	{
		LOG_FORMATS("TAP: images that are not 8000Hz are not yet supported\n");
		return -1;
	}

	while (sample_count < cas_size)
	{
		sample_count += x1_fill_wave(buffer, casdata[data_pos], sample_count);
		data_pos++;
	}

	return sample_count;
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int x1_cas_to_wav_size (const uint8_t *casdata, int caslen)
{
	uint32_t ret;

	if (!memcmp(casdata, "TAPE", 4))  // new TAP format
	{
		ret = casdata[0x20] | (casdata[0x21] << 8) | (casdata[0x22] << 16) | (casdata[0x23] << 24);
		cas_size = ret;

		samplerate = get_u32le(&casdata[0x1c]);
		new_format = 1;
	}
	else  // old TAP format
	{
		ret = (caslen - 4) * 8; // each byte = 8 samples
		cas_size = ret;

		samplerate = get_u32le(&casdata[0x00]);
		new_format = 0;
	}

	return ret;
}

/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int x1_cas_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return x1_handle_tap(buffer,bytes);
}

static const cassette_image::LegacyWaveFiller x1_legacy_fill_wave =
{
	x1_cas_fill_wave,                       /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	x1_cas_to_wav_size,                 /* chunk_sample_calc */
	8000,                                   /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error x1_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &x1_legacy_fill_wave);
}



static cassette_image::error x1_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&x1_legacy_fill_wave);
}


static const cassette_image::Format x1_cassette_format = {
	"tap",
	x1_cas_identify,
	x1_cas_load,
	nullptr
};

CASSETTE_FORMATLIST_START(x1_cassette_formats)
	CASSETTE_FORMAT(x1_cassette_format)
CASSETTE_FORMATLIST_END
