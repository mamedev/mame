/*****************************************************************************

    Harris HC-55516 (and related) emulator

    Copyright Nicola Salmoria and the MAME Team

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
	UINT8   shiftreg_mask;

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


static STREAM_UPDATE( hc55516_update );



static device_start_err start_common(const device_config *device, int clock,
						  UINT8 _shiftreg_mask, int _active_clock_hi)
{
	struct hc55516_data *chip = device->token;

	/* compute the fixed charge, decay, and leak time constants */
	charge = pow(exp(-1), 1.0 / (FILTER_CHARGE_TC * 16000.0));
	decay = pow(exp(-1), 1.0 / (FILTER_DECAY_TC * 16000.0));
	leak = pow(exp(-1), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0));

	chip->clock = clock;
	chip->shiftreg_mask = _shiftreg_mask;
	chip->active_clock_hi = _active_clock_hi;
	chip->last_clock_state = 0;

	/* create the stream */
	chip->channel = stream_create(device, 0, 1, SAMPLE_RATE, chip, hc55516_update);

	state_save_register_device_item(device, 0, chip->last_clock_state);
	state_save_register_device_item(device, 0, chip->digit);
	state_save_register_device_item(device, 0, chip->new_digit);
	state_save_register_device_item(device, 0, chip->shiftreg);
	state_save_register_device_item(device, 0, chip->curr_sample);
	state_save_register_device_item(device, 0, chip->next_sample);
	state_save_register_device_item(device, 0, chip->update_count);
	state_save_register_device_item(device, 0, chip->filter);
	state_save_register_device_item(device, 0, chip->integrator);

	/* success */
	return DEVICE_START_OK;
}


static SND_START( hc55516 )
{
	return start_common(device, clock, 0x07, TRUE);
}


static SND_START( mc3417 )
{
	return start_common(device, clock, 0x07, FALSE);
}


static SND_START( mc3418 )
{
	return start_common(device, clock, 0x0f, FALSE);
}



static SND_RESET( hc55516 )
{
	struct hc55516_data *chip = device->token;
	chip->last_clock_state = 0;
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

	/* shift the bit into the shift register */
	chip->shiftreg = (chip->shiftreg << 1) | chip->digit;

	/* move the estimator up or down a step based on the bit */
	if (chip->digit)
		integrator += chip->filter;
	else
		integrator -= chip->filter;

	/* simulate leakage */
	integrator *= leak;

	/* if we got all 0's or all 1's in the last n bits, bump the step up */
	if (((chip->shiftreg & chip->shiftreg_mask) == 0) ||
		((chip->shiftreg & chip->shiftreg_mask) == chip->shiftreg_mask))
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


static STREAM_UPDATE( hc55516_update )
{
	struct hc55516_data *chip = param;
	stream_sample_t *buffer = outputs[0];
	int i;
	INT32 sample, slope;

	/* zero-length? bail */
	if (samples == 0)
		return;

	if (!is_external_osciallator(chip))
	{
		/* track how many samples we've updated without a clock */
		chip->update_count += samples;
		if (chip->update_count > SAMPLE_RATE / 32)
		{
			chip->update_count = SAMPLE_RATE;
			chip->next_sample = 0;
		}
	}

	/* compute the interpolation slope */
	sample = chip->curr_sample;
	slope = ((INT32)chip->next_sample - sample) / samples;
	chip->curr_sample = chip->next_sample;

	if (is_external_osciallator(chip))
	{
		/* external oscillator */
		for (i = 0; i < samples; i++, sample += slope)
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
		for (i = 0; i < samples; i++, sample += slope)
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

SND_GET_INFO( hc55516 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct hc55516_data);		break;
		case SNDINFO_FCT_ALIAS:							info->type = SOUND_HC55516;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( hc55516 );	break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( hc55516 );	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "HC-55516");				break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "CVSD");					break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "2.1");						break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);					break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


SND_GET_INFO( mc3417 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( mc3417 );		break;
		case SNDINFO_PTR_RESET:							/* chip has no reset pin */					break;
		case SNDINFO_STR_NAME:							strcpy(info->s, "MC3417");					break;
		default: 										SND_GET_INFO_CALL(hc55516);					break;
	}
}


SND_GET_INFO( mc3418 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( mc3418 );		break;
		case SNDINFO_PTR_RESET:							/* chip has no reset pin */					break;
		case SNDINFO_STR_NAME:							strcpy(info->s, "MC3418");					break;
		default: 										SND_GET_INFO_CALL(hc55516);					break;
	}
}
