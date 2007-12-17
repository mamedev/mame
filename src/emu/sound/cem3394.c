/***************************************************************************

    CEM3394 sound driver.

    This driver handles CEM-3394 analog synth chip. Very crudely.

    Still to do:
        - adjust the overall volume when multiple waves are being generated
        - filter internal sound
        - support resonance (don't understand how it works)

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "cem3394.h"
#include <math.h>



/* waveform generation parameters */
#define ENABLE_PULSE		1
#define ENABLE_TRIANGLE		1
#define ENABLE_SAWTOOTH		1
#define ENABLE_EXTERNAL		1


/* pulse shaping parameters */
/* examples: */
/*    hat trick - skidding ice sounds too loud if minimum width is too big */
/*    snake pit - melody during first level too soft if minimum width is too small */
/*    snake pit - bonus counter at the end of level */
/*    snacks'n jaxson - laugh at end of level is too soft if minimum width is too small */

#define LIMIT_WIDTH			1
#define MINIMUM_WIDTH		0.25
#define MAXIMUM_WIDTH		0.75


/********************************************************************************

    From the datasheet:

    CEM3394_VCO_FREQUENCY:
        -4.0 ... +4.0
        -0.75 V/octave
        f = exp(V) * 431.894

    CEM3394_MODULATION_AMOUNT
         0.0 ... +3.5
         0.0 == 0.01 x frequency
         3.5 == 2.00 x frequency

    CEM3394_WAVE_SELECT
        -0.5 ... -0.2 == triangle
        +0.9 ... +1.5 == triangle + sawtooth
        +2.3 ... +3.9 == sawtooth

    CEM3394_PULSE_WIDTH
         0.0 ... +2.0
         0.0 ==   0% duty cycle
        +2.0 == 100% duty cycle

    CEM3394_MIXER_BALANCE
        -4.0 ... +4.0
         0.0 both at -6dB
         -20 dB/V

    CEM3394_FILTER_RESONANCE
         0.0 ... +2.5
         0.0 == no resonance
        +2.5 == oscillation

    CEM3394_FILTER_FREQENCY
        -3.0 ... +4.0
        -0.375 V/octave
         0.0 == 1300Hz

    CEM3394_FINAL_GAIN
         0.0 ... +4.0
         -20 dB/V
         0.0 == -90dB
         4.0 == 0dB

    Square wave output = 160 (average is constant regardless of duty cycle)
    Sawtooth output = 200
    Triangle output = 250
    Sawtooth + triangle output = 330
    Maximum output = 400

********************************************************************************/


/* various waveforms */
#define WAVE_TRIANGLE		1
#define WAVE_SAWTOOTH		2
#define WAVE_PULSE			4

/* keep lots of fractional bits */
#define FRACTION_BITS		28
#define FRACTION_ONE		(1 << FRACTION_BITS)
#define FRACTION_ONE_D		((double)(1 << FRACTION_BITS))
#define FRACTION_MASK		(FRACTION_ONE - 1)
#define FRACTION_MULT(a,b)	(((a) >> (FRACTION_BITS / 2)) * ((b) >> (FRACTION_BITS - FRACTION_BITS / 2)))


/* this structure defines the parameters for a channel */
typedef struct
{
	sound_stream * stream;			/* our stream */
	int index;
	void (*external)(int, int, short *);/* callback to generate external samples */
	double vco_zero_freq;			/* frequency of VCO at 0.0V */
	double filter_zero_freq;		/* frequency of filter at 0.0V */

	double values[8];				/* raw values of registers */
	UINT8 wave_select;				/* flags which waveforms are enabled */

	UINT32 volume;					/* linear overall volume (0-256) */
	UINT32 mixer_internal;			/* linear internal volume (0-256) */
	UINT32 mixer_external;			/* linear external volume (0-256) */

	UINT32 position;				/* current VCO frequency position (0.FRACTION_BITS) */
	UINT32 step;					/* per-sample VCO step (0.FRACTION_BITS) */

	UINT32 filter_position;			/* current filter frequency position (0.FRACTION_BITS) */
	UINT32 filter_step;				/* per-sample filter step (0.FRACTION_BITS) */
	UINT32 modulation_depth;		/* fraction of total by which we modulate (0.FRACTION_BITS) */
	INT16 last_ext;					/* last external sample we read */

	UINT32 pulse_width;				/* fractional pulse width (0.FRACTION_BITS) */

	double inv_sample_rate;
	int sample_rate;

	INT16 *mixer_buffer;
	INT16 *external_buffer;
} sound_chip;


/* generate sound to the mix buffer in mono */
static void cem3394_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	sound_chip *chip = param;
	int int_volume = (chip->volume * chip->mixer_internal) / 256;
	int ext_volume = (chip->volume * chip->mixer_external) / 256;
	UINT32 step = chip->step, position, end_position = 0;
	stream_sample_t *buffer = _buffer[0];
	INT16 *mix, *ext;
	int i;

	/* external volume is effectively 0 if no external function */
	if (!chip->external || !ENABLE_EXTERNAL)
		ext_volume = 0;

	/* adjust the volume for the filter */
	if (step > chip->filter_step)
		int_volume /= step - chip->filter_step;

	/* bail if nothing's going on */
	if (int_volume == 0 && ext_volume == 0)
	{
		memset(buffer, 0, sizeof(*buffer) * length);
		return;
	}

	/* if there's external stuff, fetch and process it now */
	if (ext_volume != 0)
	{
		UINT32 fposition = chip->filter_position, fstep = chip->filter_step, depth;
		INT16 last_ext = chip->last_ext;

		/* fetch the external data */
		(*chip->external)(chip->index, length, chip->external_buffer);

		/* compute the modulation depth, and adjust fstep to the maximum frequency */
		/* we lop off 13 bits of depth so that we can multiply by stepadjust, below, */
		/* which has 13 bits of precision */
		depth = FRACTION_MULT(fstep, chip->modulation_depth);
		fstep += depth;
		depth >>= 13;

		/* "apply" the filter: note this is pretty cheesy; it basically just downsamples the
           external sample to filter_freq by allowing only 2 transitions for every cycle */
		for (i = 0, ext = chip->external_buffer, position = chip->position; i < length; i++, ext++)
		{
			UINT32 newposition;
			INT32 stepadjust;

			/* update the position and compute the adjustment from a triangle wave */
			if (position & (1 << (FRACTION_BITS - 1)))
				stepadjust = 0x2000 - ((position >> (FRACTION_BITS - 14)) & 0x1fff);
			else
				stepadjust = (position >> (FRACTION_BITS - 14)) & 0x1fff;
			position += step;

			/* if we cross a half-step boundary, allow the next byte of the external input */
			newposition = fposition + fstep - (stepadjust * depth);
			if ((newposition ^ fposition) & ~(FRACTION_MASK >> 1))
				last_ext = *ext;
			else
				*ext = last_ext;
			fposition = newposition & FRACTION_MASK;
		}

		/* update the final filter values */
		chip->filter_position = fposition;
		chip->last_ext = last_ext;
	}

	/* if there's internal stuff, generate it */
	if (int_volume != 0)
	{
		if (chip->wave_select == 0 && !ext_volume)
			logerror("%f V didn't cut it\n", chip->values[CEM3394_WAVE_SELECT]);

		/* handle the pulse component; it maxes out at 0x1932, which is 27% smaller than */
		/* the sawtooth (since the value is constant, this is the best place to have an */
		/* odd value for volume) */
		if (ENABLE_PULSE && (chip->wave_select & WAVE_PULSE))
		{
			UINT32 pulse_width = chip->pulse_width;

			/* if the width is wider than the step, we're guaranteed to hit it once per cycle */
			if (pulse_width >= step)
			{
				for (i = 0, mix = chip->mixer_buffer, position = chip->position; i < length; i++, mix++)
				{
					if (position < pulse_width)
						*mix = 0x1932;
					else
						*mix = 0x0000;
					position = (position + step) & FRACTION_MASK;
				}
			}

			/* otherwise, we compute a volume and watch for cycle boundary crossings */
			else
			{
				INT16 volume = 0x1932 * pulse_width / step;
				for (i = 0, mix = chip->mixer_buffer, position = chip->position; i < length; i++, mix++)
				{
					UINT32 newposition = position + step;
					if ((newposition ^ position) & ~FRACTION_MASK)
						*mix = volume;
					else
						*mix = 0x0000;
					position = newposition & FRACTION_MASK;
				}
			}
			end_position = position;
		}

		/* otherwise, clear the mixing buffer */
		else
			memset(chip->mixer_buffer, 0, sizeof(INT16) * length);

		/* handle the sawtooth component; it maxes out at 0x2000, which is 27% larger */
		/* than the pulse */
		if (ENABLE_SAWTOOTH && (chip->wave_select & WAVE_SAWTOOTH))
		{
			for (i = 0, mix = chip->mixer_buffer, position = chip->position; i < length; i++, mix++)
			{
				*mix += ((position >> (FRACTION_BITS - 14)) & 0x3fff) - 0x2000;
				position += step;
			}
			end_position = position & FRACTION_MASK;
		}

		/* handle the triangle component; it maxes out at 0x2800, which is 25% larger */
		/* than the sawtooth (should be 27% according to the specs, but 25% saves us */
		/* a multiplication) */
		if (ENABLE_TRIANGLE && (chip->wave_select & WAVE_TRIANGLE))
		{
			for (i = 0, mix = chip->mixer_buffer, position = chip->position; i < length; i++, mix++)
			{
				INT16 value;
				if (position & (1 << (FRACTION_BITS - 1)))
					value = 0x2000 - ((position >> (FRACTION_BITS - 14)) & 0x1fff);
				else
					value = (position >> (FRACTION_BITS - 14)) & 0x1fff;
				*mix += value + (value >> 2);
				position += step;
			}
			end_position = position & FRACTION_MASK;
		}

		/* update the final position */
		chip->position = end_position;
	}

	/* mix it down */
	mix = chip->mixer_buffer;
	ext = chip->external_buffer;
	{
		/* internal + external */
		if (ext_volume != 0 && int_volume != 0)
		{
			for (i = 0; i < length; i++, mix++, ext++)
				*buffer++ = (*mix * int_volume + *ext * ext_volume) / 128;
		}
		/* internal only */
		else if (int_volume != 0)
		{
			for (i = 0; i < length; i++, mix++)
				*buffer++ = *mix * int_volume / 128;
		}
		/* external only */
		else
		{
			for (i = 0; i < length; i++, ext++)
				*buffer++ = *ext * ext_volume / 128;
		}
	}
}


static void *cem3394_start(int sndindex, int clock, const void *config)
{
	const struct cem3394_interface *intf = config;
	sound_chip *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));
	chip->index = sndindex;

	/* copy global parameters */
	chip->sample_rate = CEM3394_SAMPLE_RATE;
	chip->inv_sample_rate = 1.0 / (double)chip->sample_rate;

	/* allocate stream channels, 1 per chip */
	chip->stream = stream_create(0, 1, chip->sample_rate, chip, cem3394_update);
	chip->external = intf->external;
	chip->vco_zero_freq = intf->vco_zero_freq;
	chip->filter_zero_freq = intf->filter_zero_freq;

	/* allocate memory for a mixer buffer and external buffer (1 second should do it!) */
	chip->mixer_buffer = auto_malloc(chip->sample_rate * sizeof(INT16));
	chip->external_buffer = auto_malloc(chip->sample_rate * sizeof(INT16));

	state_save_register_item_array("cem3394", sndindex, chip->values);
	state_save_register_item("cem3394", sndindex, chip->wave_select);
	state_save_register_item("cem3394", sndindex, chip->volume);
	state_save_register_item("cem3394", sndindex, chip->mixer_internal);
	state_save_register_item("cem3394", sndindex, chip->mixer_external);
	state_save_register_item("cem3394", sndindex, chip->position);
	state_save_register_item("cem3394", sndindex, chip->step);
	state_save_register_item("cem3394", sndindex, chip->filter_position);
	state_save_register_item("cem3394", sndindex, chip->filter_step);
	state_save_register_item("cem3394", sndindex, chip->modulation_depth);
	state_save_register_item("cem3394", sndindex, chip->last_ext);
	state_save_register_item("cem3394", sndindex, chip->pulse_width);

	return chip;
}


INLINE double compute_db(double voltage)
{
	/* assumes 0.0 == full off, 4.0 == full on, with linear taper, as described in the datasheet */

	/* above 4.0, maximum volume */
	if (voltage >= 4.0)
		return 0.0;

	/* below 0.0, minimum volume */
	else if (voltage <= 0.0)
		return 90.0;

	/* between 2.5 and 4.0, linear from 20dB to 0dB */
	else if (voltage >= 2.5)
		return (4.0 - voltage) * (1.0 / 1.5) * 20.0;

	/* between 0.0 and 2.5, exponential to 20dB */
	else
	{
		double temp = 20.0 * pow(2.0, 2.5 - voltage);
		if (temp < 90.0) return 90.0;
		else return temp;
	}
}


INLINE UINT32 compute_db_volume(double voltage)
{
	double temp;

	/* assumes 0.0 == full off, 4.0 == full on, with linear taper, as described in the datasheet */

	/* above 4.0, maximum volume */
	if (voltage >= 4.0)
		return 256;

	/* below 0.0, minimum volume */
	else if (voltage <= 0.0)
		return 0;

	/* between 2.5 and 4.0, linear from 20dB to 0dB */
	else if (voltage >= 2.5)
		temp = (4.0 - voltage) * (1.0 / 1.5) * 20.0;

	/* between 0.0 and 2.5, exponential to 20dB */
	else
	{
		temp = 20.0 * pow(2.0, 2.5 - voltage);
		if (temp < 50.0) return 0;
	}

	/* convert from dB to volume and return */
	return (UINT32)(256.0 * pow(0.891251, temp));
}


void cem3394_set_voltage(int chipnum, int input, double voltage)
{
	sound_chip *chip = sndti_token(SOUND_CEM3394, chipnum);
	double temp;

	/* don't do anything if no change */
	if (voltage == chip->values[input])
		return;
	chip->values[input] = voltage;

	/* update the stream first */
	stream_update(chip->stream);

	/* switch off the input */
	switch (input)
	{
		/* frequency varies from -4.0 to +4.0, at 0.75V/octave */
		case CEM3394_VCO_FREQUENCY:
			temp = chip->vco_zero_freq * pow(2.0, -voltage * (1.0 / 0.75));
			chip->step = (UINT32)(temp * chip->inv_sample_rate * FRACTION_ONE_D);
			break;

		/* wave select determines triangle/sawtooth enable */
		case CEM3394_WAVE_SELECT:
			chip->wave_select &= ~(WAVE_TRIANGLE | WAVE_SAWTOOTH);
			if (voltage >= -0.5 && voltage <= -0.2)
				chip->wave_select |= WAVE_TRIANGLE;
			else if (voltage >=  0.9 && voltage <=  1.5)
				chip->wave_select |= WAVE_TRIANGLE | WAVE_SAWTOOTH;
			else if (voltage >=  2.3 && voltage <=  3.9)
				chip->wave_select |= WAVE_SAWTOOTH;
			break;

		/* pulse width determines duty cycle; 0.0 means 0%, 2.0 means 100% */
		case CEM3394_PULSE_WIDTH:
			if (voltage < 0.0)
			{
				chip->pulse_width = 0;
				chip->wave_select &= ~WAVE_PULSE;
			}
			else
			{
				temp = voltage * 0.5;
				if (LIMIT_WIDTH)
					temp = MINIMUM_WIDTH + (MAXIMUM_WIDTH - MINIMUM_WIDTH) * temp;
				chip->pulse_width = (UINT32)(temp * FRACTION_ONE_D);
				chip->wave_select |= WAVE_PULSE;
			}
			break;

		/* final gain is pretty self-explanatory; 0.0 means ~90dB, 4.0 means 0dB */
		case CEM3394_FINAL_GAIN:
			chip->volume = compute_db_volume(voltage);
			break;

		/* mixer balance is a pan between the external input and the internal input */
		/* 0.0 is equal parts of both; positive values favor external, negative favor internal */
		case CEM3394_MIXER_BALANCE:
			if (voltage >= 0.0)
			{
				chip->mixer_internal = compute_db_volume(3.55 - voltage);
				chip->mixer_external = compute_db_volume(3.55 + 0.45 * (voltage * 0.25));
			}
			else
			{
				chip->mixer_internal = compute_db_volume(3.55 - 0.45 * (voltage * 0.25));
				chip->mixer_external = compute_db_volume(3.55 + voltage);
			}
			break;

		/* filter frequency varies from -4.0 to +4.0, at 0.375V/octave */
		case CEM3394_FILTER_FREQENCY:
			temp = chip->filter_zero_freq * pow(2.0, -voltage * (1.0 / 0.375));
			chip->filter_step = (UINT32)(temp * chip->inv_sample_rate * FRACTION_ONE_D);
			break;

		/* modulation depth is 0.01 at 0V and 2.0 at 3.5V; how it grows from one to the other */
		/* is still unclear at this point */
		case CEM3394_MODULATION_AMOUNT:
			if (voltage < 0.0)
				chip->modulation_depth = (UINT32)(0.01 * FRACTION_ONE_D);
			else if (voltage > 3.5)
				chip->modulation_depth = (UINT32)(2.00 * FRACTION_ONE_D);
			else
				chip->modulation_depth = (UINT32)(((voltage * (1.0 / 3.5)) * 1.99 + 0.01) * FRACTION_ONE_D);
			break;

		/* this is not yet implemented */
		case CEM3394_FILTER_RESONANCE:
			break;
	}
}


double cem3394_get_parameter(int chipnum, int input)
{
	sound_chip *chip = sndti_token(SOUND_CEM3394, chipnum);
	double voltage = chip->values[input];

	switch (input)
	{
		case CEM3394_VCO_FREQUENCY:
			return chip->vco_zero_freq * pow(2.0, -voltage * (1.0 / 0.75));

		case CEM3394_WAVE_SELECT:
			return voltage;

		case CEM3394_PULSE_WIDTH:
			if (voltage <= 0.0)
				return 0.0;
			else if (voltage >= 2.0)
				return 1.0;
			else
				return voltage * 0.5;

		case CEM3394_FINAL_GAIN:
			return compute_db(voltage);

		case CEM3394_MIXER_BALANCE:
			return voltage * 0.25;

		case CEM3394_MODULATION_AMOUNT:
			if (voltage < 0.0)
				return 0.01;
			else if (voltage > 3.5)
				return 2.0;
			else
				return (voltage * (1.0 / 3.5)) * 1.99 + 0.01;

		case CEM3394_FILTER_RESONANCE:
			if (voltage < 0.0)
				return 0.0;
			else if (voltage > 2.5)
				return 1.0;
			else
				return voltage * (1.0 / 2.5);

		case CEM3394_FILTER_FREQENCY:
			return chip->filter_zero_freq * pow(2.0, -voltage * (1.0 / 0.375));
	}
	return 0.0;
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void cem3394_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void cem3394_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = cem3394_set_info;		break;
		case SNDINFO_PTR_START:							info->start = cem3394_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "CEM3394";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Analog Synth";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

