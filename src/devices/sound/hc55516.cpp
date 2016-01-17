// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*****************************************************************************

    Harris HC-55516 (and related) emulator

*****************************************************************************/

#include "emu.h"
#include "hc55516.h"


/* 4x oversampling */
#define SAMPLE_RATE             (48000 * 4)

#define INTEGRATOR_LEAK_TC      0.001
#define FILTER_DECAY_TC         0.004
#define FILTER_CHARGE_TC        0.004
#define FILTER_MIN              0.0416
#define FILTER_MAX              1.0954
#define SAMPLE_GAIN             10000.0




const device_type HC55516 = &device_creator<hc55516_device>;

hc55516_device::hc55516_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HC55516, "HC-55516", tag, owner, clock, "hc55516", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_active_clock_hi(0),
		m_shiftreg_mask(0),
		m_last_clock_state(0),
		m_digit(0),
		m_new_digit(0),
		m_shiftreg(0),
		m_curr_sample(0),
		m_next_sample(0),
		m_update_count(0),
		m_filter(0),
		m_integrator(0),
		m_charge(0),
		m_decay(0),
		m_leak(0)
{
}
hc55516_device::hc55516_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_active_clock_hi(0),
		m_shiftreg_mask(0),
		m_last_clock_state(0),
		m_digit(0),
		m_new_digit(0),
		m_shiftreg(0),
		m_curr_sample(0),
		m_next_sample(0),
		m_update_count(0),
		m_filter(0),
		m_integrator(0),
		m_charge(0),
		m_decay(0),
		m_leak(0)
{
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
	start_common(0x07, TRUE);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hc55516_device::device_reset()
{
	m_last_clock_state = 0;
}

const device_type MC3417 = &device_creator<mc3417_device>;

mc3417_device::mc3417_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: hc55516_device(mconfig, MC3417, "MC3417", tag, owner, clock, "mc3417", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc3417_device::device_start()
{
	start_common(0x07, FALSE);
}


const device_type MC3418 = &device_creator<mc3418_device>;

mc3418_device::mc3418_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: hc55516_device(mconfig, MC3418, "MC3418", tag, owner, clock, "mc3418", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc3418_device::device_start()
{
	start_common(0x0f, FALSE);
}


void hc55516_device::start_common(UINT8 _shiftreg_mask, int _active_clock_hi)
{
	/* compute the fixed charge, decay, and leak time constants */
	m_charge = pow(exp(-1.0), 1.0 / (FILTER_CHARGE_TC * 16000.0));
	m_decay = pow(exp(-1.0), 1.0 / (FILTER_DECAY_TC * 16000.0));
	m_leak = pow(exp(-1.0), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0));

	m_shiftreg_mask = _shiftreg_mask;
	m_active_clock_hi = _active_clock_hi;
	m_last_clock_state = 0;

	/* create the stream */
	m_channel = machine().sound().stream_alloc(*this, 0, 1, SAMPLE_RATE);

	save_item(NAME(m_last_clock_state));
	save_item(NAME(m_digit));
	save_item(NAME(m_new_digit));
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_curr_sample));
	save_item(NAME(m_next_sample));
	save_item(NAME(m_update_count));
	save_item(NAME(m_filter));
	save_item(NAME(m_integrator));
}

inline int hc55516_device::is_external_oscillator()
{
	return clock() != 0;
}


inline int hc55516_device::is_active_clock_transition(int clock_state)
{
	return (( m_active_clock_hi && !m_last_clock_state &&  clock_state) ||
			(!m_active_clock_hi &&  m_last_clock_state && !clock_state));
}


inline int hc55516_device::current_clock_state()
{
	return ((UINT64)m_update_count * clock() * 2 / SAMPLE_RATE) & 0x01;
}


void hc55516_device::process_digit()
{
	double integrator = m_integrator, temp;

	/* shift the bit into the shift register */
	m_shiftreg = (m_shiftreg << 1) | m_digit;

	/* move the estimator up or down a step based on the bit */
	if (m_digit)
		integrator += m_filter;
	else
		integrator -= m_filter;

	/* simulate leakage */
	integrator *= m_leak;

	/* if we got all 0's or all 1's in the last n bits, bump the step up */
	if (((m_shiftreg & m_shiftreg_mask) == 0) ||
		((m_shiftreg & m_shiftreg_mask) == m_shiftreg_mask))
	{
		m_filter = FILTER_MAX - ((FILTER_MAX - m_filter) * m_charge);

		if (m_filter > FILTER_MAX)
			m_filter = FILTER_MAX;
	}

	/* simulate decay */
	else
	{
		m_filter *= m_decay;

		if (m_filter < FILTER_MIN)
			m_filter = FILTER_MIN;
	}

	/* compute the sample as a 32-bit word */
	temp = integrator * SAMPLE_GAIN;
	m_integrator = integrator;

	/* compress the sample range to fit better in a 16-bit word */
	if (temp < 0)
		m_next_sample = (int)(temp / (-temp * (1.0 / 32768.0) + 1.0));
	else
		m_next_sample = (int)(temp / (temp * (1.0 / 32768.0) + 1.0));
}

void hc55516_device::clock_w(int state)
{
	UINT8 clock_state = state ? TRUE : FALSE;

	/* only makes sense for setups with a software driven clock */
	assert(!is_external_oscillator());

	/* speech clock changing? */
	if (is_active_clock_transition(clock_state))
	{
		/* update the output buffer before changing the registers */
		m_channel->update();

		/* clear the update count */
		m_update_count = 0;

		process_digit();
	}

	/* update the clock */
	m_last_clock_state = clock_state;
}


void hc55516_device::digit_w(int digit)
{
	if (is_external_oscillator())
	{
		m_channel->update();
		m_new_digit = digit & 1;
	}
	else
		m_digit = digit & 1;
}


int hc55516_device::clock_state_r()
{
	/* only makes sense for setups with an external oscillator */
	assert(is_external_oscillator());

	m_channel->update();

	return current_clock_state();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void hc55516_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];
	int i;
	INT32 sample, slope;

	/* zero-length? bail */
	if (samples == 0)
		return;

	if (!is_external_oscillator())
	{
		/* track how many samples we've updated without a clock */
		m_update_count += samples;
		if (m_update_count > SAMPLE_RATE / 32)
		{
			m_update_count = SAMPLE_RATE;
			m_next_sample = 0;
		}
	}

	/* compute the interpolation slope */
	sample = m_curr_sample;
	slope = ((INT32)m_next_sample - sample) / samples;
	m_curr_sample = m_next_sample;

	if (is_external_oscillator())
	{
		/* external oscillator */
		for (i = 0; i < samples; i++, sample += slope)
		{
			UINT8 clock_state;

			*buffer++ = sample;

			m_update_count++;

			clock_state = current_clock_state();

			/* pull in next digit on the appropriate edge of the clock */
			if (is_active_clock_transition(clock_state))
			{
				m_digit = m_new_digit;

				process_digit();
			}

			m_last_clock_state = clock_state;
		}
	}

	/* software driven clock */
	else
		for (i = 0; i < samples; i++, sample += slope)
			*buffer++ = sample;
}

void mc3417_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	hc55516_device::sound_stream_update(stream, inputs, outputs, samples);
}

void mc3418_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	hc55516_device::sound_stream_update(stream, inputs, outputs, samples);
}
