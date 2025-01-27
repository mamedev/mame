// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/* .PTP Microkey Primo tape images */

#include "primoptp.h"
#include "imageutl.h"


#define PRIMO_WAVEENTRY_LOW     -32768
#define PRIMO_WAVEENTRY_HIGH        32767
#define PRIMO_WAVEENTRY_ZERO        0

#define PRIMO_WAV_FREQUENCY     22050
#define PRIMO_BIT_1_PERIOD      (312*2*0.000001)
#define PRIMO_BIT_0_PERIOD              (3*PRIMO_BIT_1_PERIOD)

#define PRIMO_BIT_1_LENGTH      (PRIMO_BIT_1_PERIOD*PRIMO_WAV_FREQUENCY)
#define PRIMO_BIT_0_LENGTH      (PRIMO_BIT_0_PERIOD*PRIMO_WAV_FREQUENCY)
/*
#define PRIMO_BIT_1_LENGTH      6
#define PRIMO_BIT_0_LENGTH      16
*/
#define PRIMO_PAUSE_LENGTH      2000
#define PRIMO_FILE_PILOT_LENGTH     ((4*PRIMO_BIT_1_LENGTH + 4*PRIMO_BIT_0_LENGTH)*512)
#define PRIMO_BLOCK_PILOT_LENGTH    ((8*PRIMO_BIT_1_LENGTH)*96 + (5*PRIMO_BIT_1_LENGTH + 3*PRIMO_BIT_0_LENGTH)*3)

static uint32_t primo_tape_image_length;

static int16_t *primo_emit_level(int16_t *p, int count, int level)
{
	for (int i=0; i<count; i++) *(p++) = level;

	return p;
}

static int16_t* primo_output_bit(int16_t *p, uint8_t bit)
{
	if (bit)
	{
		p = primo_emit_level (p, PRIMO_BIT_1_LENGTH/2, PRIMO_WAVEENTRY_HIGH);
		p = primo_emit_level (p, PRIMO_BIT_1_LENGTH/2, PRIMO_WAVEENTRY_LOW);
	}
	else
	{
		p = primo_emit_level (p, PRIMO_BIT_0_LENGTH/2, PRIMO_WAVEENTRY_HIGH);
		p = primo_emit_level (p, PRIMO_BIT_0_LENGTH/2, PRIMO_WAVEENTRY_LOW);
	}
		return p;
}

static int16_t* primo_output_byte(int16_t *p, uint8_t byte)
{
	int i;

	for (i=0; i<8; i++)
		p = primo_output_bit(p,(byte>>(7-i)) & 0x01);

	return p;
}

static uint32_t primo_cassette_calculate_number_of_1(const uint8_t *bytes, uint16_t length)
{
	int i,j;

	uint32_t number_of_1 = 0;

	for (i=0; i<length; i++)
		for (j=0; j<8; j++)
			if ((bytes[i]>>j)&0x01)
				number_of_1++;

	return number_of_1;
}

static int primo_cassette_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	int i = 0, j = 0;

	uint8_t *b = (uint8_t*) bytes;

	uint32_t file_size = 0;
	uint16_t block_size = 0;

	uint32_t number_of_1 = 0;
	uint32_t number_of_0 = 0;
	uint32_t size_in_samples = 0;

	primo_tape_image_length = length;

	while (i < length)
	{
		size_in_samples += PRIMO_PAUSE_LENGTH;
		LOG_FORMATS ("Samples (pause): %u\n", size_in_samples);

		size_in_samples += PRIMO_FILE_PILOT_LENGTH;
		LOG_FORMATS ("Samples (file pilot): %u\n", size_in_samples);

		/* file size with header */
		file_size = *(b+1) + *(b+2)*256;
		b += 3;
		LOG_FORMATS ("File size (with header): %u\n", file_size);

		/* b is now set on the first data byte of file
		   it means first byte (type) of block */

		j = 0;
		while (j < file_size-3)
		{
			size_in_samples += PRIMO_BLOCK_PILOT_LENGTH;
			LOG_FORMATS ("Samples (block pilot): %u\n", size_in_samples);

			/* block size without header but including CRC byte */
			block_size = *(b+1) + *(b+2)*256;

			/* b is set on the first byte of block data */
			b += 3;

			number_of_1 = primo_cassette_calculate_number_of_1(b, block_size);
			number_of_0 = (block_size)*8-number_of_1;

			size_in_samples += number_of_1 * PRIMO_BIT_1_LENGTH;
			size_in_samples += number_of_0 * PRIMO_BIT_0_LENGTH;

			LOG_FORMATS ("Samples (block data): %u\n", size_in_samples);

			LOG_FORMATS ("\tBlock size: %u\n", block_size);

			b += block_size;

			/* b is set on the first header byte of next block or file */

			j += block_size+3;
		}

		i += file_size;
	}

	return size_in_samples;
}

static int primo_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	int i = 0, j = 0, k;

	int16_t *p = buffer;
	const uint8_t *b = bytes;

	uint32_t file_size = 0;
	uint16_t block_size = 0;

	LOG_FORMATS ("Image size: %d\n", length);

	while (i < primo_tape_image_length)
	{
		LOG_FORMATS ("Beginning Primo file\n");
		/* pause */
		p = primo_emit_level (p, PRIMO_PAUSE_LENGTH, PRIMO_WAVEENTRY_ZERO);

		LOG_FORMATS ("Samples (pause): %ld\n", (long int)(p-buffer));

		/* file pilot */
		for (k=0; k<512; k++)
			p = primo_output_byte (p, 0xaa);

		LOG_FORMATS ("Samples (file pilot): %ld\n", (long int)(p-buffer));

		/* file size with header */
		file_size = *(b+1) + *(b+2)*256;
		b += 3;

		LOG_FORMATS ("File size: %u\n", file_size);

		/* b is now set on the first data byte of file
		   it means first byte (block type) of block header */

		j = 0;
		while (j < file_size-3)
		{
			/* block pilot */
			for (k=0; k<96; k++)
				p = primo_output_byte (p, 0xff);
			for (k=0; k<3; k++)
				p = primo_output_byte (p, 0xd3);

			LOG_FORMATS ("Samples (block pilot): %ld\n", (long int)(p-buffer));

			/* block size without header but including CRC byte */
			block_size = *(b+1) + *(b+2)*256;
			b += 3;

			for (k=0; k<block_size; k++)
				p = primo_output_byte (p, *(b+k));

			LOG_FORMATS ("Samples (block data): %ld\n", (long int)(p-buffer));

			b += block_size;

			/* b is now set on the first header byte of next block or file */

			j += block_size+3;
		}

		LOG_FORMATS ("Primo file finished\n");
		LOG_FORMATS ("i = %d\n", i);
		i += file_size;
		LOG_FORMATS ("i = %d\n", i);
	}

	LOG_FORMATS ("End of fill_wave/n");

	return p - buffer;
}

static const cassette_image::LegacyWaveFiller primo_legacy_fill_wave =
{
	primo_cassette_fill_wave,           /* fill_wave */
	-1,                                         /* chunk_size */
	0,                                          /* chunk_samples */
	primo_cassette_calculate_size_in_samples,   /* chunk_sample_calc */
	PRIMO_WAV_FREQUENCY,                                        /* sample_frequency */
	0,                                          /* header_samples */
	0                                           /* trailer_samples */
};

static cassette_image::error primo_ptp_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &primo_legacy_fill_wave);
}

static cassette_image::error primo_ptp_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&primo_legacy_fill_wave);
}

static const cassette_image::Format primo_ptp_image_format =
{
	"ptp",
	primo_ptp_identify,
	primo_ptp_load,
	nullptr
};

CASSETTE_FORMATLIST_START(primo_ptp_format)
	CASSETTE_FORMAT(primo_ptp_image_format)
CASSETTE_FORMATLIST_END
