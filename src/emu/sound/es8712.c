/**********************************************************************************************
 *
 *  Streaming singe channel ADPCM core for the ES8712 chip
 *  Chip is branded by Excellent Systems, probably OEM'd.
 *
 *  Samples are currently looped, but whether they should and how, is unknown.
 *  Interface to the chip is also not 100% clear.
 *  Should there be any status signals signifying busy, end of sample - etc?
 *
 *  Heavily borrowed from the OKI M6295 source
 *
 **********************************************************************************************/


#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "es8712.h"

#define MAX_SAMPLE_CHUNK	10000


/* struct describing a playing ADPCM chip */
struct es8712
{
	UINT8 playing;			/* 1 if we're actively playing */

	UINT32 base_offset;		/* pointer to the base memory location */
	UINT32 sample;			/* current sample number */
	UINT32 count;			/* total samples to play */

	UINT32 signal;			/* current ADPCM signal */
	UINT32 step;			/* current ADPCM step */

	UINT32 start;			/* starting address for the next loop */
	UINT32 end;				/* ending address for the next loop */
	UINT8  repeat;			/* Repeat current sample when 1 */

	INT32 bank_offset;
	UINT8 *region_base;		/* pointer to the base of the region */
	sound_stream *stream;	/* which stream are we playing on? */
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* useful interfaces */
const struct ES8712interface es8712_interface_region_1 = { REGION_SOUND1 };
const struct ES8712interface es8712_interface_region_2 = { REGION_SOUND2 };
const struct ES8712interface es8712_interface_region_3 = { REGION_SOUND3 };
const struct ES8712interface es8712_interface_region_4 = { REGION_SOUND4 };


/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables(void)
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}
}



/**********************************************************************************************

    generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(struct es8712 *chip, stream_sample_t *buffer, int samples)
{
	/* if this chip is active */
	if (chip->playing)
	{
		UINT8 *base = chip->region_base + chip->bank_offset + chip->base_offset;
		int sample = chip->sample;
		int signal = chip->signal;
		int count = chip->count;
		int step = chip->step;
		int val;

		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			val = base[sample / 2] >> (((sample & 1) << 2) ^ 4);
			signal += diff_lookup[step * 16 + (val & 15)];

			/* clamp to the maximum */
			if (signal > 2047)
				signal = 2047;
			else if (signal < -2048)
				signal = -2048;

			/* adjust the step size and clamp */
			step += index_shift[val & 7];
			if (step > 48)
				step = 48;
			else if (step < 0)
				step = 0;

			/* output to the buffer */
			*buffer++ = signal * 16;
			samples--;

			/* next! */
			if (++sample >= count)
			{
				if (chip->repeat)
				{
					sample = 0;
					signal = -2;
					step = 0;
				}
				else
				{
					chip->playing = 0;
					break;
				}
			}
		}

		/* update the parameters */
		chip->sample = sample;
		chip->signal = signal;
		chip->step = step;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;
}


/**********************************************************************************************

    es8712_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static void es8712_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];
	struct es8712 *chip = param;

	/* generate them into our buffer */
	generate_adpcm(chip, buffer, samples);
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

static void es8712_state_save_register(struct es8712 *chip, int sndindex)
{
	char buf[20];

	sprintf(buf,"ES8712");

	state_save_register_item(buf, sndindex, chip->bank_offset);

	state_save_register_item(buf, sndindex, chip->playing);
	state_save_register_item(buf, sndindex, chip->sample);
	state_save_register_item(buf, sndindex, chip->count);
	state_save_register_item(buf, sndindex, chip->signal);
	state_save_register_item(buf, sndindex, chip->step);

	state_save_register_item(buf, sndindex, chip->base_offset);

	state_save_register_item(buf, sndindex, chip->start);
	state_save_register_item(buf, sndindex, chip->end);
	state_save_register_item(buf, sndindex, chip->repeat);
}



/**********************************************************************************************

    ES8712_start -- start emulation of an ES8712 chip

***********************************************************************************************/

static void *es8712_start(int sndindex, int clock, const void *config)
{
	const struct ES8712interface *intf = config;
	struct es8712 *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	compute_tables();

	chip->start = 0;
	chip->end = 0;
	chip->repeat = 0;

	chip->bank_offset = 0;
	chip->region_base = memory_region(intf->region);

	/* generate the name and create the stream */
	chip->stream = stream_create(0, 1, clock, chip, es8712_update);

	/* initialize the rest of the structure */
	chip->signal = -2;

	es8712_state_save_register(chip, sndindex);

	/* success */
	return chip;
}



/*************************************************************************************

     ES8712_reset -- stop emulation of an ES8712-compatible chip

**************************************************************************************/

static void es8712_reset(void *chip_src)
{
	struct es8712 *chip = chip_src;

	if (chip->playing)
	{
		/* update the stream, then turn it off */
		stream_update(chip->stream);
		chip->playing = 0;
		chip->repeat = 0;
	}
}


/****************************************************************************

    ES8712_set_bank_base -- set the base of the bank on a given chip

*****************************************************************************/

void ES8712_set_bank_base(int which, int base)
{
	struct es8712 *chip = sndti_token(SOUND_ES8712, which);
	stream_update(chip->stream);
	chip->bank_offset = base;
}


/****************************************************************************

    ES8712_set_frequency -- dynamically adjusts the frequency of a given ADPCM chip

*****************************************************************************/

void ES8712_set_frequency(int which, int frequency)
{
	struct es8712 *chip = sndti_token(SOUND_ES8712, which);

	/* update the stream and set the new base */
	stream_update(chip->stream);
	stream_set_sample_rate(chip->stream, frequency);
}



/**********************************************************************************************

    ES8712_play -- Begin playing the addressed sample

***********************************************************************************************/

void ES8712_play(int which)
{
	struct es8712 *chip = sndti_token(SOUND_ES8712, which);


	if (chip->start < chip->end)
	{
		if (!chip->playing)
		{
			chip->playing = 1;
			chip->base_offset = chip->start;
			chip->sample = 0;
			chip->count = 2 * (chip->end - chip->start + 1);
			chip->repeat = 0;//1;

			/* also reset the ADPCM parameters */
			chip->signal = -2;
			chip->step = 0;
		}
	}
	/* invalid samples go here */
	else
	{
		logerror("ES871295:%d requested to play invalid sample range %06x-%06x\n",which,chip->start,chip->end);

		if (chip->playing)
		{
			/* update the stream */
			stream_update(chip->stream);
			chip->playing = 0;
		}
	}
}



/**********************************************************************************************

     ES8712_data_0_w -- generic data write functions
     ES8712_data_1_w

***********************************************************************************************/

/**********************************************************************************************
 *
 *  offset  Start       End
 *          0hmmll  -  0HMMLL
 *    00    ----ll
 *    01    --mm--
 *    02    0h----
 *    03               ----LL
 *    04               --MM--
 *    05               0H----
 *    06           Go!
 *
 * Offsets are written in the order -> 00, 02, 01, 03, 05, 04, 06
 * Offset 06 is written with the same value as offset 04.
 *
***********************************************************************************************/

static void ES8712_data_w(int which, int offset, UINT32 data)
{
	struct es8712 *chip = sndti_token(SOUND_ES8712, which);
	switch (offset)
	{
		case 00:	chip->start &= 0x000fff00;
					chip->start |= ((data & 0xff) <<  0); break;
		case 01:	chip->start &= 0x000f00ff;
					chip->start |= ((data & 0xff) <<  8); break;
		case 02:	chip->start &= 0x0000ffff;
					chip->start |= ((data & 0x0f) << 16); break;
		case 03:	chip->end   &= 0x000fff00;
					chip->end   |= ((data & 0xff) <<  0); break;
		case 04:	chip->end   &= 0x000f00ff;
					chip->end   |= ((data & 0xff) <<  8); break;
		case 05:	chip->end   &= 0x0000ffff;
					chip->end   |= ((data & 0x0f) << 16); break;
		case 06:
				ES8712_play(which);
				break;
		default:	break;
	}
	chip->start &= 0xfffff; chip->end &= 0xfffff;
}

WRITE8_HANDLER( ES8712_data_0_w )
{
	ES8712_data_w(0, offset, data);
}

WRITE8_HANDLER( ES8712_data_1_w )
{
	ES8712_data_w(1, offset, data);
}

WRITE8_HANDLER( ES8712_data_2_w )
{
	ES8712_data_w(2, offset, data);
}

WRITE16_HANDLER( ES8712_data_0_lsb_w )
{
	if (ACCESSING_LSB)
		ES8712_data_w(0, offset, data & 0xff);
}

WRITE16_HANDLER( ES8712_data_1_lsb_w )
{
	if (ACCESSING_LSB)
		ES8712_data_w(1, offset, data & 0xff);
}

WRITE16_HANDLER( ES8712_data_2_lsb_w )
{
	if (ACCESSING_LSB)
		ES8712_data_w(2, offset, data & 0xff);
}

WRITE16_HANDLER( ES8712_data_0_msb_w )
{
	if (ACCESSING_MSB)
		ES8712_data_w(0, offset, data >> 8);
}

WRITE16_HANDLER( ES8712_data_1_msb_w )
{
	if (ACCESSING_MSB)
		ES8712_data_w(1, offset, data >> 8);
}

WRITE16_HANDLER( ES8712_data_2_msb_w )
{
	if (ACCESSING_MSB)
		ES8712_data_w(2, offset, data >> 8);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void es8712_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void es8712_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = es8712_set_info;		break;
		case SNDINFO_PTR_START:							info->start = es8712_start;				break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = es8712_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "ES8712";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Excellent Systems ADPCM";			break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2005, The MAME Team"; break;
	}
}

