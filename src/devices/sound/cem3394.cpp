// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    CEM3394 sound driver.

    This driver handles CEM-3394 analog synth chip. Very crudely.

    Still to do:
        - adjust the overall volume when multiple waves are being generated
        - filter internal sound
        - support resonance (don't understand how it works)

***************************************************************************/

#include "emu.h"
#include "cem3394.h"



/* waveform generation parameters */
#define ENABLE_PULSE        1
#define ENABLE_TRIANGLE     1
#define ENABLE_SAWTOOTH     1
#define ENABLE_EXTERNAL     1


/* pulse shaping parameters */
/* examples: */
/*    hat trick - skidding ice sounds too loud if minimum width is too big */
/*    snake pit - melody during first level too soft if minimum width is too small */
/*    snake pit - bonus counter at the end of level */
/*    snacks'n jaxson - laugh at end of level is too soft if minimum width is too small */

#define LIMIT_WIDTH         1
#define MINIMUM_WIDTH       0.25
#define MAXIMUM_WIDTH       0.75


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


// various waveforms
#define WAVE_TRIANGLE       1
#define WAVE_SAWTOOTH       2
#define WAVE_PULSE          4

// keep lots of fractional bits
#define FRACTION_BITS       28
#define FRACTION_ONE        (1 << FRACTION_BITS)
#define FRACTION_ONE_D      ((double)(1 << FRACTION_BITS))
#define FRACTION_MASK       (FRACTION_ONE - 1)
#define FRACTION_MULT(a,b)  (((a) >> (FRACTION_BITS / 2)) * ((b) >> (FRACTION_BITS - FRACTION_BITS / 2)))


// device type definition
const device_type CEM3394 = &device_creator<cem3394_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cem3394_device - constructor
//-------------------------------------------------

cem3394_device::cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CEM3394, "CEM3394", tag, owner, clock, "cem3394", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_vco_zero_freq(0.0),
		m_filter_zero_freq(0.0),
		m_wave_select(0),
		m_volume(0),
		m_mixer_internal(0),
		m_mixer_external(0),
		m_position(0),
		m_step(0),
		m_filter_position(0),
		m_filter_step(0),
		m_modulation_depth(0),
		m_last_ext(0),
		m_pulse_width(0),
		m_inv_sample_rate(0.0),
		m_sample_rate(0),
		m_mixer_buffer(nullptr),
		m_external_buffer(nullptr)
{
	memset(m_values, 0, 8*sizeof(double));
}


//-------------------------------------------------
//  sound_stream_update - generate sound to the mix buffer in mono
//-------------------------------------------------

void cem3394_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int int_volume = (m_volume * m_mixer_internal) / 256;
	int ext_volume = (m_volume * m_mixer_external) / 256;
	UINT32 step = m_step, position, end_position = 0;
	stream_sample_t *buffer = outputs[0];
	INT16 *mix, *ext;
	int i;

	/* external volume is effectively 0 if no external function */
	if (m_ext_cb.isnull() || !ENABLE_EXTERNAL)
		ext_volume = 0;

	/* adjust the volume for the filter */
	if (step > m_filter_step)
		int_volume /= step - m_filter_step;

	/* bail if nothing's going on */
	if (int_volume == 0 && ext_volume == 0)
	{
		memset(buffer, 0, sizeof(*buffer) * samples);
		return;
	}

	/* if there's external stuff, fetch and process it now */
	if (ext_volume != 0)
	{
		UINT32 fposition = m_filter_position, fstep = m_filter_step, depth;
		INT16 last_ext = m_last_ext;

		/* fetch the external data */
		m_ext_cb(samples, m_external_buffer);

		/* compute the modulation depth, and adjust fstep to the maximum frequency */
		/* we lop off 13 bits of depth so that we can multiply by stepadjust, below, */
		/* which has 13 bits of precision */
		depth = FRACTION_MULT(fstep, m_modulation_depth);
		fstep += depth;
		depth >>= 13;

		/* "apply" the filter: note this is pretty cheesy; it basically just downsamples the
		   external sample to filter_freq by allowing only 2 transitions for every cycle */
		for (i = 0, ext = m_external_buffer, position = m_position; i < samples; i++, ext++)
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
		m_filter_position = fposition;
		m_last_ext = last_ext;
	}

	/* if there's internal stuff, generate it */
	if (int_volume != 0)
	{
		if (m_wave_select == 0 && !ext_volume)
			logerror("%f V didn't cut it\n", m_values[CEM3394_WAVE_SELECT]);

		/* handle the pulse component; it maxes out at 0x1932, which is 27% smaller than */
		/* the sawtooth (since the value is constant, this is the best place to have an */
		/* odd value for volume) */
		if (ENABLE_PULSE && (m_wave_select & WAVE_PULSE))
		{
			UINT32 pulse_width = m_pulse_width;

			/* if the width is wider than the step, we're guaranteed to hit it once per cycle */
			if (pulse_width >= step)
			{
				for (i = 0, mix = m_mixer_buffer, position = m_position; i < samples; i++, mix++)
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
				for (i = 0, mix = m_mixer_buffer, position = m_position; i < samples; i++, mix++)
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
			memset(m_mixer_buffer, 0, sizeof(INT16) * samples);

		/* handle the sawtooth component; it maxes out at 0x2000, which is 27% larger */
		/* than the pulse */
		if (ENABLE_SAWTOOTH && (m_wave_select & WAVE_SAWTOOTH))
		{
			for (i = 0, mix = m_mixer_buffer, position = m_position; i < samples; i++, mix++)
			{
				*mix += ((position >> (FRACTION_BITS - 14)) & 0x3fff) - 0x2000;
				position += step;
			}
			end_position = position & FRACTION_MASK;
		}

		/* handle the triangle component; it maxes out at 0x2800, which is 25% larger */
		/* than the sawtooth (should be 27% according to the specs, but 25% saves us */
		/* a multiplication) */
		if (ENABLE_TRIANGLE && (m_wave_select & WAVE_TRIANGLE))
		{
			for (i = 0, mix = m_mixer_buffer, position = m_position; i < samples; i++, mix++)
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
		m_position = end_position;
	}

	/* mix it down */
	mix = m_mixer_buffer;
	ext = m_external_buffer;
	{
		/* internal + external */
		if (ext_volume != 0 && int_volume != 0)
		{
			for (i = 0; i < samples; i++, mix++, ext++)
				*buffer++ = (*mix * int_volume + *ext * ext_volume) / 128;
		}
		/* internal only */
		else if (int_volume != 0)
		{
			for (i = 0; i < samples; i++, mix++)
				*buffer++ = *mix * int_volume / 128;
		}
		/* external only */
		else
		{
			for (i = 0; i < samples; i++, ext++)
				*buffer++ = *ext * ext_volume / 128;
		}
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cem3394_device::device_start()
{
	/* copy global parameters */
	m_sample_rate = CEM3394_SAMPLE_RATE;
	m_inv_sample_rate = 1.0 / (double)m_sample_rate;

	/* allocate stream channels, 1 per chip */
	m_stream = stream_alloc(0, 1, m_sample_rate);

	m_ext_cb.bind_relative_to(*owner());

	/* allocate memory for a mixer buffer and external buffer (1 second should do it!) */
	m_mixer_buffer = auto_alloc_array(machine(), INT16, m_sample_rate);
	m_external_buffer = auto_alloc_array(machine(), INT16, m_sample_rate);

	save_item(NAME(m_values));
	save_item(NAME(m_wave_select));
	save_item(NAME(m_volume));
	save_item(NAME(m_mixer_internal));
	save_item(NAME(m_mixer_external));
	save_item(NAME(m_position));
	save_item(NAME(m_step));
	save_item(NAME(m_filter_position));
	save_item(NAME(m_filter_step));
	save_item(NAME(m_modulation_depth));
	save_item(NAME(m_last_ext));
	save_item(NAME(m_pulse_width));
}


double cem3394_device::compute_db(double voltage)
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


UINT32 cem3394_device::compute_db_volume(double voltage)
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


void cem3394_device::set_voltage(int input, double voltage)
{
	double temp;

	/* don't do anything if no change */
	if (voltage == m_values[input])
		return;
	m_values[input] = voltage;

	/* update the stream first */
	m_stream->update();

	/* switch off the input */
	switch (input)
	{
		/* frequency varies from -4.0 to +4.0, at 0.75V/octave */
		case CEM3394_VCO_FREQUENCY:
			temp = m_vco_zero_freq * pow(2.0, -voltage * (1.0 / 0.75));
			m_step = (UINT32)(temp * m_inv_sample_rate * FRACTION_ONE_D);
			break;

		/* wave select determines triangle/sawtooth enable */
		case CEM3394_WAVE_SELECT:
			m_wave_select &= ~(WAVE_TRIANGLE | WAVE_SAWTOOTH);
			if (voltage >= -0.5 && voltage <= -0.2)
				m_wave_select |= WAVE_TRIANGLE;
			else if (voltage >=  0.9 && voltage <=  1.5)
				m_wave_select |= WAVE_TRIANGLE | WAVE_SAWTOOTH;
			else if (voltage >=  2.3 && voltage <=  3.9)
				m_wave_select |= WAVE_SAWTOOTH;
			break;

		/* pulse width determines duty cycle; 0.0 means 0%, 2.0 means 100% */
		case CEM3394_PULSE_WIDTH:
			if (voltage < 0.0)
			{
				m_pulse_width = 0;
				m_wave_select &= ~WAVE_PULSE;
			}
			else
			{
				temp = voltage * 0.5;
				if (LIMIT_WIDTH)
					temp = MINIMUM_WIDTH + (MAXIMUM_WIDTH - MINIMUM_WIDTH) * temp;
				m_pulse_width = (UINT32)(temp * FRACTION_ONE_D);
				m_wave_select |= WAVE_PULSE;
			}
			break;

		/* final gain is pretty self-explanatory; 0.0 means ~90dB, 4.0 means 0dB */
		case CEM3394_FINAL_GAIN:
			m_volume = compute_db_volume(voltage);
			break;

		/* mixer balance is a pan between the external input and the internal input */
		/* 0.0 is equal parts of both; positive values favor external, negative favor internal */
		case CEM3394_MIXER_BALANCE:
			if (voltage >= 0.0)
			{
				m_mixer_internal = compute_db_volume(3.55 - voltage);
				m_mixer_external = compute_db_volume(3.55 + 0.45 * (voltage * 0.25));
			}
			else
			{
				m_mixer_internal = compute_db_volume(3.55 - 0.45 * (voltage * 0.25));
				m_mixer_external = compute_db_volume(3.55 + voltage);
			}
			break;

		/* filter frequency varies from -4.0 to +4.0, at 0.375V/octave */
		case CEM3394_FILTER_FREQENCY:
			temp = m_filter_zero_freq * pow(2.0, -voltage * (1.0 / 0.375));
			m_filter_step = (UINT32)(temp * m_inv_sample_rate * FRACTION_ONE_D);
			break;

		/* modulation depth is 0.01 at 0V and 2.0 at 3.5V; how it grows from one to the other */
		/* is still unclear at this point */
		case CEM3394_MODULATION_AMOUNT:
			if (voltage < 0.0)
				m_modulation_depth = (UINT32)(0.01 * FRACTION_ONE_D);
			else if (voltage > 3.5)
				m_modulation_depth = (UINT32)(2.00 * FRACTION_ONE_D);
			else
				m_modulation_depth = (UINT32)(((voltage * (1.0 / 3.5)) * 1.99 + 0.01) * FRACTION_ONE_D);
			break;

		/* this is not yet implemented */
		case CEM3394_FILTER_RESONANCE:
			break;
	}
}


double cem3394_device::get_parameter(int input)
{
	double voltage = m_values[input];

	switch (input)
	{
		case CEM3394_VCO_FREQUENCY:
			return m_vco_zero_freq * pow(2.0, -voltage * (1.0 / 0.75));

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
			return m_filter_zero_freq * pow(2.0, -voltage * (1.0 / 0.375));
	}
	return 0.0;
}
