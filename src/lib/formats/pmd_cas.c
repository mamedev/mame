// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/********************************************************************

    Support for PMD 85 cassette images

    Supported formats:
    - pmd: raw image
    - ptp: PMD 85 tape package

********************************************************************/

#include <assert.h>

#include "pmd_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define PMD85_WAV_FREQUENCY 7200
#define PMD85_TIMER_FREQUENCY   1200
#define PMD85_BIT_LENGTH    (PMD85_WAV_FREQUENCY/PMD85_TIMER_FREQUENCY)
#define PMD85_PILOT_BITS    (PMD85_TIMER_FREQUENCY*3)
#define PMD85_PAUSE_BITS    (PMD85_TIMER_FREQUENCY/2)
#define PMD85_HEADER_BYTES  63
#define PMD85_BITS_PER_BYTE 11

// image size
static int pmd85_image_size;

static int pmd85_emit_level(INT16 *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i=0; i<count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int pmd85_output_bit(INT16 *buffer, int sample_pos, UINT8 bit)
{
	int samples = 0;

	if (bit)
	{
		samples += pmd85_emit_level (buffer, sample_pos + samples, PMD85_BIT_LENGTH/2, WAVEENTRY_LOW);
		samples += pmd85_emit_level (buffer, sample_pos + samples, PMD85_BIT_LENGTH/2, WAVEENTRY_HIGH);
	}
	else
	{
		samples += pmd85_emit_level (buffer, sample_pos + samples, PMD85_BIT_LENGTH/2, WAVEENTRY_HIGH);
		samples += pmd85_emit_level (buffer, sample_pos + samples, PMD85_BIT_LENGTH/2, WAVEENTRY_LOW);
	}

	return samples;
}

static int pmd85_output_byte(INT16 *buffer, int sample_pos, UINT8 byte)
{
	int samples = 0;

	/* start */
	samples += pmd85_output_bit (buffer, sample_pos + samples, 0);

	/* data */
	for (int i=0; i<8; i++)
		samples += pmd85_output_bit(buffer,sample_pos + samples, (byte>>i) & 0x01);

	/* stop */
	samples += pmd85_output_bit (buffer, sample_pos + samples, 1);
	samples += pmd85_output_bit (buffer, sample_pos + samples, 1);

	return samples;
}

static bool pmd85_is_header_block(const UINT8 *bytes)
{
	for (int i=0; i<0x10; i++)
	{
		if (bytes[i] != 0xff || bytes[i + 0x10] != 0x00 || bytes[i + 0x20] != 0x55)
			return false;
	}

	return true;
}

static void pmd85_printf_image_info(const UINT8 *bytes, int sample_count)
{
#if 0
	char track_name[9];
	UINT32 sec = (UINT32)(sample_count/PMD85_WAV_FREQUENCY);
	UINT16 addr = (bytes[0x33]<<8) | bytes[0x32];
	strncpy(track_name, (char*)&bytes[0x36], 8);
	track_name[8] = '\0';

	printf("Block ID: %02d     %s    0x%04x     Tape pos: %02d:%02d\n", bytes[0x30], track_name, addr, sec/60, sec%60);
#endif
}

static int pmd85_handle_cassette(INT16 *buffer, const UINT8 *bytes)
{
	int sample_count = 0;

	if (pmd85_is_header_block(bytes))
	{
		// PMD file

		/* pilot */
		for (int i=0; i<PMD85_PILOT_BITS; i++)
			sample_count += pmd85_output_bit(buffer, sample_count, 1);

		if (!buffer)
			pmd85_printf_image_info(bytes, sample_count);

		/* header */
		for (int i=0; i<PMD85_HEADER_BYTES; i++)
			sample_count += pmd85_output_byte(buffer, sample_count, bytes[i]);

		/* pause */
		for (int i=0; i<PMD85_PAUSE_BITS; i++)
			sample_count += pmd85_output_bit(buffer, sample_count, 1);

		/* data */
		for (int i=PMD85_HEADER_BYTES; i<pmd85_image_size; i++)
			sample_count += pmd85_output_byte(buffer, sample_count, bytes[i]);
	}
	else
	{
		// PTP file

		/* pilot */
		for (int i=0; i<PMD85_PILOT_BITS; i++)
			sample_count += pmd85_output_bit(buffer, sample_count, 1);

		int data_pos = 0;
		while (data_pos < pmd85_image_size)
		{
			UINT16 block_size = (bytes[data_pos + 1]<<8) | bytes[data_pos];
			int pause_len = PMD85_PAUSE_BITS;

			data_pos += 2;

			if (pmd85_is_header_block(bytes + data_pos))
			{
				if (!buffer)
					pmd85_printf_image_info(bytes + data_pos, sample_count);

				pause_len *= 2;
			}

			for (int i=0; i<pause_len; i++)
				sample_count += pmd85_output_bit(buffer, sample_count, 1);

			for (int i=0; i<block_size; i++)
				sample_count += pmd85_output_byte(buffer, sample_count, bytes[data_pos + i]);

			data_pos += block_size;
		}
	}

	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/

static int pmd85_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	return pmd85_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int pmd85_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	pmd85_image_size = length;

	return pmd85_handle_cassette(NULL, bytes);
}

static const struct CassetteLegacyWaveFiller pmd85_legacy_fill_wave =
{
	pmd85_cassette_fill_wave,                   /* fill_wave */
	-1,                                         /* chunk_size */
	0,                                          /* chunk_samples */
	pmd85_cassette_calculate_size_in_samples,   /* chunk_sample_calc */
	PMD85_WAV_FREQUENCY,                        /* sample_frequency */
	0,                                          /* header_samples */
	0                                           /* trailer_samples */
};

static casserr_t pmd85_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &pmd85_legacy_fill_wave);
}

static casserr_t pmd85_cassette_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &pmd85_legacy_fill_wave);
}

static const struct CassetteFormat pmd85_cassette_image_format =
{
	"pmd,tap,ptp",
	pmd85_cassette_identify,
	pmd85_cassette_load,
	NULL
};

CASSETTE_FORMATLIST_START(pmd85_cassette_formats)
	CASSETTE_FORMAT(pmd85_cassette_image_format)
CASSETTE_FORMATLIST_END
