/*****************************************************************************

    Harris HC-55516 (and related) emulator

    Copyright Nicola Salmoria and the MAME Team

    Notes:
        * The only difference between MC3417 and MC3418 is the number
          of bits checked for the Coincidence Output.  For the MC3417,
          it is three bits, while for the MC3418 it is four.
          This is not emulated.

*****************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "hc55516.h"
#include <math.h>


/* 4x oversampling */
#define SAMPLE_RATE 			(48000 * 4)

#define	INTEGRATOR_LEAK_TC		0.001
#define	FILTER_DECAY_TC			0.004
#define	FILTER_CHARGE_TC		0.004
#define	FILTER_MIN				0.0416
#define	FILTER_MAX				1.0954
#define	SAMPLE_GAIN				10000.0


struct hc55516_data
{
	sound_stream *channel;
	int		clock;		/* 0 = software driven, non-0 = oscillator */
	int		active_clock_hi;

	UINT8	last_clock_state;
	UINT8	digit;
	UINT8	new_digit;
	UINT8	shiftreg;

	INT16	curr_sample;
	INT16	next_sample;

	UINT32	update_count;

	double 	filter;
	double	integrator;
};


static double charge, decay, leak;


static void hc55516_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length);



static void *start_common(int sndindex, int clock, const void *config, int _active_clock_hi)
{
	struct hc55516_data *chip;

	/* allocate the chip */
	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	/* compute the fixed charge, decay, and leak time constants */
	charge = pow(exp(-1), 1.0 / (FILTER_CHARGE_TC * 16000.0));
	decay = pow(exp(-1), 1.0 / (FILTER_DECAY_TC * 16000.0));
	leak = pow(exp(-1), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0));

	chip->clock = clock;
	chip->active_clock_hi = _active_clock_hi;
	chip->last_clock_state = 0;

	/* create the stream */
	chip->channel = stream_create(0, 1, SAMPLE_RATE, chip, hc55516_update);

	state_save_register_item("hc55516", sndindex, chip->last_clock_state);
	state_save_register_item("hc55516", sndindex, chip->digit);
	state_save_register_item("hc55516", sndindex, chip->new_digit);
	state_save_register_item("hc55516", sndindex, chip->shiftreg);
	state_save_register_item("hc55516", sndindex, chip->curr_sample);
	state_save_register_item("hc55516", sndindex, chip->next_sample);
	state_save_register_item("hc55516", sndindex, chip->update_count);
	state_save_register_item("hc55516", sndindex, chip->filter);
	state_save_register_item("hc55516", sndindex, chip->integrator);

	/* success */
	return chip;
}


static void *hc55516_start(int sndindex, int clock, const void *config)
{
	return start_common(sndindex, clock, config, TRUE);
}


static void *mc3417_start(int sndindex, int clock, const void *config)
{
	return start_common(sndindex, clock, config, FALSE);
}


static void *mc3418_start(int sndindex, int clock, const void *config)
{
	return start_common(sndindex, clock, config, FALSE);
}



static void hc55516_reset(void *chip)
{
	((struct hc55516_data *)chip)->last_clock_state = 0;
}



INLINE int is_external_osciallator(struct hc55516_data *chip)
{
	return chip->clock != 0;
}


INLINE int is_active_clock_transition(struct hc55516_data *chip, int clock_state)
{
	return (( chip->active_clock_hi && !chip->last_clock_state &&  clock_state) ||
			(!chip->active_clock_hi &&  chip->last_clock_state && !clock_state));
}


INLINE int current_clock_state(struct hc55516_data *chip)
{
	return ((UINT64)chip->update_count * chip->clock * 2 / SAMPLE_RATE) & 0x01;
}


static void process_digit(struct hc55516_data *chip)
{
	double integrator = chip->integrator, temp;

	/* move the estimator up or down a step based on the bit */
	if (chip->digit)
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
		chip->next_sample = (int)(temp / (-temp * (1.0 / 32768.0) + 1.0));
	else
		chip->next_sample = (int)(temp / (temp * (1.0 / 32768.0) + 1.0));
}


static void hc55516_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct hc55516_data *chip = param;
	stream_sample_t *buffer = _buffer[0];
	int i;
	INT32 sample, slope;

	/* zero-length? bail */
	if (length == 0)
		return;

	if (!is_external_osciallator(chip))
	{
		/* track how many samples we've updated without a clock */
		chip->update_count += length;
		if (chip->update_count > SAMPLE_RATE / 32)
		{
			chip->update_count = SAMPLE_RATE;
			chip->next_sample = 0;
		}
	}

	/* compute the interpolation slope */
	sample = chip->curr_sample;
	slope = ((INT32)chip->next_sample - sample) / length;
	chip->curr_sample = chip->next_sample;

	if (is_external_osciallator(chip))
	{
		/* external oscillator */
		for (i = 0; i < length; i++, sample += slope)
		{
			UINT8 clock_state;

			*buffer++ = sample;

			chip->update_count++;

			clock_state = current_clock_state(chip);

			/* pull in next digit on the appropriate edge of the clock */
			if (is_active_clock_transition(chip, clock_state))
			{
				chip->digit = chip->new_digit;

				process_digit(chip);
			}

			chip->last_clock_state = clock_state;
		}
	}

	/* software driven clock */
	else
		for (i = 0; i < length; i++, sample += slope)
			*buffer++ = sample;
}


void hc55516_clock_w(int num, int state)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);
	UINT8 clock_state = state ? TRUE : FALSE;

	/* only makes sense for setups with a software driven clock */
	assert(!is_external_osciallator(chip));

	/* speech clock changing? */
	if (is_active_clock_transition(chip, clock_state))
	{
		/* update the output buffer before changing the registers */
		stream_update(chip->channel);

		/* clear the update count */
		chip->update_count = 0;

		process_digit(chip);
	}

	/* update the clock */
	chip->last_clock_state = clock_state;
}


void hc55516_digit_w(int num, int digit)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);

	if (is_external_osciallator(chip))
	{
		stream_update(chip->channel);
		chip->new_digit = digit & 1;
	}
	else
		chip->digit = digit & 1;
}


void hc55516_clock_clear_w(int num)
{
	hc55516_clock_w(num, 0);
}


void hc55516_clock_set_w(int num)
{
	hc55516_clock_w(num, 1);
}


void hc55516_digit_clock_clear_w(int num, int digit)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);
	chip->digit = digit & 1;
	hc55516_clock_w(num, 0);
}


int hc55516_clock_state_r(int num)
{
	struct hc55516_data *chip = sndti_token(SOUND_HC55516, num);

	/* only makes sense for setups with an external oscillator */
	assert(is_external_osciallator(chip));

	stream_update(chip->channel);

	return current_clock_state(chip);
}



WRITE8_HANDLER( hc55516_0_digit_w )	{ hc55516_digit_w(0, data); }
WRITE8_HANDLER( hc55516_0_clock_w )	{ hc55516_clock_w(0, data); }
WRITE8_HANDLER( hc55516_0_clock_clear_w )	{ hc55516_clock_clear_w(0); }
WRITE8_HANDLER( hc55516_0_clock_set_w )		{ hc55516_clock_set_w(0); }
WRITE8_HANDLER( hc55516_0_digit_clock_clear_w )	{ hc55516_digit_clock_clear_w(0, data); }
READ8_HANDLER ( hc55516_0_clock_state_r )	{ return hc55516_clock_state_r(0); }

WRITE8_HANDLER( hc55516_1_digit_w ) { hc55516_digit_w(1, data); }
WRITE8_HANDLER( hc55516_1_clock_w ) { hc55516_clock_w(1, data); }
WRITE8_HANDLER( hc55516_1_clock_clear_w ) { hc55516_clock_clear_w(1); }
WRITE8_HANDLER( hc55516_1_clock_set_w )  { hc55516_clock_set_w(1); }
WRITE8_HANDLER( hc55516_1_digit_clock_clear_w ) { hc55516_digit_clock_clear_w(1, data); }
READ8_HANDLER ( hc55516_1_clock_state_r )	{ return hc55516_clock_state_r(1); }



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void hc55516_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_ALIAS:							info->i = SOUND_HC55516;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_START:							info->start = hc55516_start;			break;
		case SNDINFO_PTR_RESET:							info->reset = hc55516_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "HC-55516";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "CVSD";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "2.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}


void mc3417_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = mc3417_start;				break;
		case SNDINFO_PTR_RESET:							/* chip has no reset pin */				break;
		case SNDINFO_STR_NAME:							info->s = "MC3417";						break;
		default: 										hc55516_get_info(token, state, info);	break;
	}
}


void mc3418_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = mc3418_start;				break;
		case SNDINFO_PTR_RESET:							/* chip has no reset pin */				break;
		case SNDINFO_STR_NAME:							info->s = "MC3418";						break;
		default: 										hc55516_get_info(token, state, info);	break;
	}
}
