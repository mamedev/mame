#include "sndintrf.h"
#include "streams.h"
#include "hc55516.h"
#include <math.h>


/* default to 4x oversampling */
#define DEFAULT_SAMPLE_RATE (48000 * 4)


#define	INTEGRATOR_LEAK_TC		0.001
#define	FILTER_DECAY_TC			0.004
#define	FILTER_CHARGE_TC		0.004
#define	FILTER_MIN				0.0416
#define	FILTER_MAX				1.0954
#define	SAMPLE_GAIN				10000.0


struct hc55516_data
{
	sound_stream *channel;
	int		sample_rate;

	UINT8	last_clock;
	UINT8	databit;
	UINT8	shiftreg;

	INT16	curr_value;
	INT16	next_value;

	UINT32	update_count;

	double 	filter;
	double	integrator;
};


static double charge, decay, leak;


static void hc55516_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length);



static void *hc55516_start(int sndindex, int clock, const void *config)
{
	struct hc55516_data *chip;

	/* allocate the chip */
	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	/* compute the fixed charge, decay, and leak time constants */
	charge = pow(exp(-1), 1.0 / (FILTER_CHARGE_TC * 16000.0));
	decay = pow(exp(-1), 1.0 / (FILTER_DECAY_TC * 16000.0));
	leak = pow(exp(-1), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0));

	/* create the stream */
	chip->sample_rate = clock ? clock : DEFAULT_SAMPLE_RATE;
	chip->channel = stream_create(0, 1, chip->sample_rate, chip, hc55516_update);

	state_save_register_item("hc55516", sndindex, chip->last_clock);
	state_save_register_item("hc55516", sndindex, chip->databit);
	state_save_register_item("hc55516", sndindex, chip->shiftreg);
	state_save_register_item("hc55516", sndindex, chip->curr_value);
	state_save_register_item("hc55516", sndindex, chip->next_value);
	state_save_register_item("hc55516", sndindex, chip->update_count);
	state_save_register_item("hc55516", sndindex, chip->filter);
	state_save_register_item("hc55516", sndindex, chip->integrator);

	/* success */
	return chip;
}


static void hc55516_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct hc55516_data *chip = param;
	stream_sample_t *buffer = _buffer[0];
	INT32 data, slope;
	int i;

	/* zero-length? bail */
	if (length == 0)
		return;

	/* track how many samples we've updated without a clock */
	chip->update_count += length;
	if (chip->update_count > chip->sample_rate / 32)
	{
		chip->update_count = chip->sample_rate;
		chip->next_value = 0;
	}

	/* compute the interpolation slope */
	data = chip->curr_value;
	slope = ((INT32)chip->next_value - data) / length;
	chip->curr_value = chip->next_value;

	/* reset the sample count */
	for (i = 0; i < length; i++, data += slope)
		*buffer++ = data;
}


void hc55516_clock_w(int num, int state)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);
	int clock = state & 1, diffclock;

	/* update the clock */
	diffclock = clock ^ chip->last_clock;
	chip->last_clock = clock;

	/* speech clock changing (active on rising edge) */
	if (diffclock && clock)
	{
		double integrator = chip->integrator, temp;

		/* clear the update count */
		chip->update_count = 0;

		/* move the estimator up or down a step based on the bit */
		if (chip->databit)
		{
			chip->shiftreg = ((chip->shiftreg << 1) | 1) & 7;
			integrator += chip->filter;
		}
		else
		{
			chip->shiftreg = (chip->shiftreg << 1) & 7;
			integrator -= chip->filter;
		}

		/* simulate leakage */
		integrator *= leak;

		/* if we got all 0's or all 1's in the last n bits, bump the step up */
		if (chip->shiftreg == 0 || chip->shiftreg == 7)
		{
			chip->filter = FILTER_MAX - ((FILTER_MAX - chip->filter) * charge);
			if (chip->filter > FILTER_MAX)
				chip->filter = FILTER_MAX;
		}

		/* simulate decay */
		else
		{
			chip->filter *= decay;
			if (chip->filter < FILTER_MIN)
				chip->filter = FILTER_MIN;
		}

		/* compute the sample as a 32-bit word */
		temp = integrator * SAMPLE_GAIN;
		chip->integrator = integrator;

		/* compress the sample range to fit better in a 16-bit word */
		if (temp < 0)
			chip->next_value = (int)(temp / (-temp * (1.0 / 32768.0) + 1.0));
		else
			chip->next_value = (int)(temp / (temp * (1.0 / 32768.0) + 1.0));

		/* update the output buffer before changing the registers */
		stream_update(chip->channel);
	}
}


void hc55516_digit_w(int num, int data)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);
	chip->databit = data & 1;
}


void hc55516_clock_clear_w(int num, int data)
{
	hc55516_clock_w(num, 0);
}


void hc55516_clock_set_w(int num, int data)
{
	hc55516_clock_w(num, 1);
}


void hc55516_digit_clock_clear_w(int num, int data)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);
	chip->databit = data & 1;
	hc55516_clock_w(num, 0);
}


WRITE8_HANDLER( hc55516_0_digit_w )	{ hc55516_digit_w(0,data); }
WRITE8_HANDLER( hc55516_0_clock_w )	{ hc55516_clock_w(0,data); }
WRITE8_HANDLER( hc55516_0_clock_clear_w )	{ hc55516_clock_clear_w(0,data); }
WRITE8_HANDLER( hc55516_0_clock_set_w )		{ hc55516_clock_set_w(0,data); }
WRITE8_HANDLER( hc55516_0_digit_clock_clear_w )	{ hc55516_digit_clock_clear_w(0,data); }

WRITE8_HANDLER( hc55516_1_digit_w ) { hc55516_digit_w(1,data); }
WRITE8_HANDLER( hc55516_1_clock_w ) { hc55516_clock_w(1,data); }
WRITE8_HANDLER( hc55516_1_clock_clear_w ) { hc55516_clock_clear_w(1,data); }
WRITE8_HANDLER( hc55516_1_clock_set_w )  { hc55516_clock_set_w(1,data); }
WRITE8_HANDLER( hc55516_1_digit_clock_clear_w ) { hc55516_digit_clock_clear_w(1,data); }





/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void hc55516_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void hc55516_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = hc55516_set_info;		break;
		case SNDINFO_PTR_START:							info->start = hc55516_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "HC55516";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Gaelco custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

