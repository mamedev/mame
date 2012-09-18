/*****************************************************************************

    Harris HC-55516 (and related) emulator

    Copyright Nicola Salmoria and the MAME Team

*****************************************************************************/

#include "emu.h"
#include "hc55516.h"


/* 4x oversampling */
#define SAMPLE_RATE 			(48000 * 4)

#define	INTEGRATOR_LEAK_TC		0.001
#define	FILTER_DECAY_TC			0.004
#define	FILTER_CHARGE_TC		0.004
#define	FILTER_MIN				0.0416
#define	FILTER_MAX				1.0954
#define	SAMPLE_GAIN				10000.0


struct hc55516_state
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

	double	filter;
	double	integrator;
};


static double charge, decay, leak;


static STREAM_UPDATE( hc55516_update );



INLINE hc55516_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == HC55516 ||
		   device->type() == MC3417 ||
		   device->type() == MC3418);
	return (hc55516_state *)downcast<hc55516_device *>(device)->token();
}


static void start_common(device_t *device, UINT8 _shiftreg_mask, int _active_clock_hi)
{
	hc55516_state *chip = get_safe_token(device);

	/* compute the fixed charge, decay, and leak time constants */
	charge = pow(exp(-1.0), 1.0 / (FILTER_CHARGE_TC * 16000.0));
	decay = pow(exp(-1.0), 1.0 / (FILTER_DECAY_TC * 16000.0));
	leak = pow(exp(-1.0), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0));

	chip->clock = device->clock();
	chip->shiftreg_mask = _shiftreg_mask;
	chip->active_clock_hi = _active_clock_hi;
	chip->last_clock_state = 0;

	/* create the stream */
	chip->channel = device->machine().sound().stream_alloc(*device, 0, 1, SAMPLE_RATE, chip, hc55516_update);

	device->save_item(NAME(chip->last_clock_state));
	device->save_item(NAME(chip->digit));
	device->save_item(NAME(chip->new_digit));
	device->save_item(NAME(chip->shiftreg));
	device->save_item(NAME(chip->curr_sample));
	device->save_item(NAME(chip->next_sample));
	device->save_item(NAME(chip->update_count));
	device->save_item(NAME(chip->filter));
	device->save_item(NAME(chip->integrator));
}


static DEVICE_START( hc55516 )
{
	start_common(device, 0x07, TRUE);
}


static DEVICE_START( mc3417 )
{
	start_common(device, 0x07, FALSE);
}


static DEVICE_START( mc3418 )
{
	start_common(device, 0x0f, FALSE);
}



static DEVICE_RESET( hc55516 )
{
	hc55516_state *chip = get_safe_token(device);
	chip->last_clock_state = 0;
}



INLINE int is_external_osciallator(hc55516_state *chip)
{
	return chip->clock != 0;
}


INLINE int is_active_clock_transition(hc55516_state *chip, int clock_state)
{
	return (( chip->active_clock_hi && !chip->last_clock_state &&  clock_state) ||
			(!chip->active_clock_hi &&  chip->last_clock_state && !clock_state));
}


INLINE int current_clock_state(hc55516_state *chip)
{
	return ((UINT64)chip->update_count * chip->clock * 2 / SAMPLE_RATE) & 0x01;
}


static void process_digit(hc55516_state *chip)
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
	hc55516_state *chip = (hc55516_state *)param;
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


void hc55516_clock_w(device_t *device, int state)
{
	hc55516_state *chip = get_safe_token(device);
	UINT8 clock_state = state ? TRUE : FALSE;

	/* only makes sense for setups with a software driven clock */
	assert(!is_external_osciallator(chip));

	/* speech clock changing? */
	if (is_active_clock_transition(chip, clock_state))
	{
		/* update the output buffer before changing the registers */
		chip->channel->update();

		/* clear the update count */
		chip->update_count = 0;

		process_digit(chip);
	}

	/* update the clock */
	chip->last_clock_state = clock_state;
}


void hc55516_digit_w(device_t *device, int digit)
{
	hc55516_state *chip = get_safe_token(device);

	if (is_external_osciallator(chip))
	{
		chip->channel->update();
		chip->new_digit = digit & 1;
	}
	else
		chip->digit = digit & 1;
}


int hc55516_clock_state_r(device_t *device)
{
	hc55516_state *chip = get_safe_token(device);

	/* only makes sense for setups with an external oscillator */
	assert(is_external_osciallator(chip));

	chip->channel->update();

	return current_clock_state(chip);
}


const device_type HC55516 = &device_creator<hc55516_device>;

hc55516_device::hc55516_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HC55516, "HC-55516", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(hc55516_state);
}
hc55516_device::hc55516_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(hc55516_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void hc55516_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hc55516_device::device_start()
{
	DEVICE_START_NAME( hc55516 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hc55516_device::device_reset()
{
	DEVICE_RESET_NAME( hc55516 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void hc55516_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type MC3417 = &device_creator<mc3417_device>;

mc3417_device::mc3417_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hc55516_device(mconfig, MC3417, "MC3417", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc3417_device::device_start()
{
	DEVICE_START_NAME( mc3417 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mc3417_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type MC3418 = &device_creator<mc3418_device>;

mc3418_device::mc3418_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hc55516_device(mconfig, MC3418, "MC3418", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc3418_device::device_start()
{
	DEVICE_START_NAME( mc3418 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mc3418_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


