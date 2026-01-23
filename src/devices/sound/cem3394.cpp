// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Curtis Electromusic Specialties CEM3394 µP-Controllable Synthesizer Voice

    This driver handles CEM-3394 analog synth chip.

***************************************************************************/

#include "emu.h"
#include "cem3394.h"

#include <algorithm>


// various filter implementations to play with; currently SVTRAP works best
#define FILTER_TYPE_NONE   (0)
#define FILTER_TYPE_SVTRAP (1)
#define FILTER_TYPE_ESQ1   (2)

#define FILTER_TYPE FILTER_TYPE_SVTRAP

#define ENABLE_AC_COUPLING  1


// logging
#define LOG_CONTROL_CHANGES (1U << 1)
#define LOG_NANS            (1U << 2)
#define LOG_VALUES          (1U << 3)
#define LOG_CONFIG          (1U << 4)
#define VERBOSE (LOG_NANS)
#include "logmacro.h"


// use 0.25 as the base volume for pulses
static constexpr double PULSE_VOLUME = 0.25;

// sawtooth is 27% larger than pulses
static constexpr double SAWTOOTH_VOLUME = PULSE_VOLUME * 1.27f;

// triangle is 27% larger than sawtooth
static constexpr double TRIANGLE_VOLUME = SAWTOOTH_VOLUME * 1.27f;

// external input is unknown but let's make it the same as the pulse
static constexpr double EXTERNAL_VOLUME = PULSE_VOLUME;


// waveform generation parameters
#define ENABLE_PULSE        1
#define ENABLE_TRIANGLE     1
#define ENABLE_SAWTOOTH     1
#define ENABLE_EXTERNAL     1


// pulse shaping parameters
// can be enabled with configure_limit_pw(true)
// examples:
//    hat trick - skidding ice sounds too loud if minimum width is too big
//    snake pit - melody during first level too soft if minimum width is too small
//    snake pit - bonus counter at the end of level
//    snacks'n jaxson - laugh at end of level is too soft if minimum width is too small

#define LIMIT_WIDTH         1
#define MINIMUM_WIDTH       0.2
#define MAXIMUM_WIDTH       0.8


/********************************************************************************

    From the datasheet:

    VCO_FREQUENCY:
        -4.0 ... +4.0
        -0.75 V/octave
        f = exp(V) * 431.894

    MODULATION_AMOUNT
         0.0 ... +3.5
         0.0 == 0.01 x frequency
         3.5 == 2.00 x frequency

    WAVE_SELECT
        -0.5 ... -0.2 == triangle
        +0.9 ... +1.5 == triangle + sawtooth
        +2.3 ... +3.9 == sawtooth

    PULSE_WIDTH
         0.0 ... +2.0
         0.0 ==   0% duty cycle
        +2.0 == 100% duty cycle

    MIXER_BALANCE
        -4.0 ... +4.0
         0.0 both at -6dB
         -20 dB/V

    FILTER_RESONANCE
         0.0 ... +2.5
         0.0 == no resonance
        +2.5 == oscillation

    FILTER_FREQUENCY
        -3.0 ... +4.0
        -0.375 V/octave
         0.0 == 1300Hz

    FINAL_GAIN
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


// device type definition
DEFINE_DEVICE_TYPE(CEM3394, cem3394_device, "cem3394", "CEM3394 Synthesizer Voice")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cem3394_device - constructor and configuration
//-------------------------------------------------

cem3394_device::cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CEM3394, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_inv_sample_rate(1.0 / 48000.0),
	m_vco_zero_freq(500.0),
	m_filter_zero_freq(1300.0),
	m_hpf_k(0),
	m_limit_pw(false),
	m_values{-1}, // will be initialized in device_start()
	m_wave_select(0),
	m_volume(0),
	m_mixer_internal(0),
	m_mixer_external(0),
	m_vco_position(0),
	m_vco_step(0),
	m_filter_frequency(1300),
	m_filter_modulation(0),
	m_filter_resonance(0),
	m_filter_in{0},
	m_filter_out{0},
	m_pulse_width(0),
	m_hpf_mem(0)
{
	// configuring with the example values in the datasheet
	configure(270E3, 2E-9, 33E-9, 4.7E-6);
}

cem3394_device &cem3394_device::configure(double r_vco, double c_vco, double c_vcf, double c_ac)
{
	// datasheet equation for Fout at CV = 0
	m_vco_zero_freq = 1.3 / (5.0 * r_vco * c_vco);

	// VCO can range up to pow(2, 4.0/.75) = ~40.3 * zero-voltage-freq
	const double sample_rate = m_vco_zero_freq * pow(2, 4.0 / 0.75) * 5;
	m_inv_sample_rate = 1.0 / sample_rate;

	// datasheet equation for Pzcv
	// Note that "4.3 x 10E-5" in the equation should be 4.3E-5. The surrounding
	// text and the example walkthrough use the correct value.
	m_filter_zero_freq = 4.3E-5 / c_vcf;

	// See hpf() for more info
	constexpr double R_AC = 11E3;  // internal AC coupling resistor
	m_hpf_k = 1.0 - exp((-1 / (R_AC * c_ac)) * m_inv_sample_rate);

	LOGMASKED(LOG_CONFIG, "CEM3394 config - vco zero freq: %f, filter zero freq: %f, sample rate: %d\n",
			  m_vco_zero_freq, m_filter_zero_freq, int(sample_rate));
	return *this;
}

cem3394_device &cem3394_device::configure_limit_pw(bool limit_pw)
{
	m_limit_pw = limit_pw;
	return *this;
}

//-------------------------------------------------
//  filter - apply the lowpass filter at the given
//  cutoff frequency
//-------------------------------------------------

#if (FILTER_TYPE == FILTER_TYPE_NONE)

double cem3394_device::filter(double input, double cutoff)
{
	return input;
}

#elif (FILTER_TYPE == FILTER_TYPE_SVTRAP)

double cem3394_device::filter(double input, double cutoff)
{
	// clamp cutoff to useful range, 50Hz-20kHz
	cutoff = std::clamp(cutoff, 50.0, 20000.0);

	// clamp resonance to below 1.0 to prevent runaway behavior; when clamping,
	// also apply an (arbitrary) scale factor to the output since we're close
	// to resonance and the datasheet indicates there is an amplitude correction
	// in this case
	double outscale = 1.0;
	double res = m_filter_resonance;
	if (res > 0.99)
	{
		// Don't clamp if there is no input, to allow self-oscillation to occur.
		if (m_wave_select)
			res = 0.99;
		outscale = 0.5;
	}

	// core filter implementation
	double g = tan(M_PI * cutoff * m_inv_sample_rate);
	double k = 2.0 - 2.0 * res;
	double a1 = 1.0 / (1.0 + g * (g + k));
	double a2 = g * a1;
	double a3 = g * a2;
	double v3 = input - m_filter_out[1];
	double v1 = a1 * m_filter_out[0] + a2 * v3;
	double v2 = m_filter_out[1] + a2 * m_filter_out[0] + a3 * v3;
	m_filter_out[0] = 2 * v1 - m_filter_out[0];
	m_filter_out[1] = 2 * v2 - m_filter_out[1];

	// lowpass output is equal to v2
	double output = v2 * outscale;

	// catch any NaNs
	if (std::isnan(output))
	{
		LOGMASKED(LOG_NANS, "NAN - vco: %6.0f cutoff: %6.0f res: %.5f output: %.5f\n", m_vco_step / m_inv_sample_rate, cutoff, m_filter_resonance, output);
		output = 0;
		m_filter_out[0] = m_filter_out[1] = 0;
	}

	// if we go out of range, scale down to 1.0 and also scale our
	// feedback terms to help us stay in control
	else if (fabs(output) > 1.0)
	{
		double scale = 1.0 / fabs(output);
		output *= scale;
		m_filter_out[0] *= scale;
		m_filter_out[1] *= scale;
	}
	return output;
}

#elif (FILTER_TYPE == FILTER_TYPE_ESQ1)

double cem3394_device::filter(double input, double cutoff)
{
	// clamp cutoff to useful range, 50Hz-20kHz
	cutoff = std::clamp(cutoff, 50.0, 20000.0);

	// clamp resonance to 0.95 to prevent infinite gain
	double r = 4.0 * std::min(m_filter_resonance, 0.95);

	// core filter implementation
	double g = 2 * M_PI * cutoff;
	double zc = g / tan(g/2 * m_inv_sample_rate);
	double gzc = zc / g;
	double gzc2 = gzc * gzc;
	double gzc3 = gzc2 * gzc;
	double gzc4 = gzc3 * gzc;
	double r1 = 1 + r;
	double a0 = r1;
	double a1 = 4 * r1;
	double a2 = 6 * r1;
	double a3 = 4 * r1;
	double a4 = r1;
	double b0 =      r1 + 4 * gzc + 6 * gzc2 + 4 * gzc3 + gzc4;
	double b1 = 4 * (r1 + 2 * gzc            - 2 * gzc3 - gzc4);
	double b2 = 6 * (r1           - 2 * gzc2            + gzc4);
	double b3 = 4 * (r1 - 2 * gzc            + 2 * gzc3 - gzc4);
	double b4 =      r1 - 4 * gzc + 6 * gzc2 - 4 * gzc3 + gzc4;

	double output = (input * a0
					+  m_filter_in[0] * a1 +  m_filter_in[1] * a2 +  m_filter_in[2] * a3 +  m_filter_in[3] * a4
					- m_filter_out[0] * b1 - m_filter_out[1] * b2 - m_filter_out[2] * b3 - m_filter_out[3] * b4) / b0;

	// catch NaNs
	if (std::isnan(output))
	{
		LOGMASKED(LOG_NANS, "NAN - vco: %6.0f cutoff: %6.0f res: %.5f output: %.5f\n", m_vco_step / m_inv_sample_rate, cutoff, m_filter_resonance, output);
		output = 0;
	}

	// if output goes significantly out of range, scale it down
	else if (fabs(output) > 10.0)
		output = 10.0;

	// update memories
	m_filter_in[3] = m_filter_in[2];
	m_filter_in[2] = m_filter_in[1];
	m_filter_in[1] = m_filter_in[0];
	m_filter_in[0] = input;

	m_filter_out[3] = m_filter_out[2];
	m_filter_out[2] = m_filter_out[1];
	m_filter_out[1] = m_filter_out[0];
	m_filter_out[0] = output;

	// clamp to range and return
	if (output < -1.0)
		output = -1.0;
	else if (output > 1.0)
		output = 1.0;
	return output;
}

#else

#error Unknown FILTER_TYPE

#endif


//-------------------------------------------------
//  hpf - apply AC coupling to the output of the
//  filter, before the signal gets routed to the VCA.
//-------------------------------------------------

double cem3394_device::hpf(double input)
{
	// The filter's output is AC-coupled to the VCA input.

	// Based on the block diagram in the datasheet, the AC coupling (high-pass
	// filtering) is implemented by subtracting a low-pass-filtered signal
	// from the original signal.

	// The capacitor of the LPF RC is attached to pin 17, whereas the resistor
	// (11 KOhm) is internal to the chip.

	// The LPF code was obtained from sound/flt_rc.cpp.

	m_hpf_mem += (input - m_hpf_mem) * m_hpf_k; // low-pass filtered signal
	return input - m_hpf_mem; // HPFed signal = signal - LPFed signal
}


//-------------------------------------------------
//  sound_stream_update - generate sound to the mix
//  buffer in mono
//-------------------------------------------------

void cem3394_device::sound_stream_update(sound_stream &stream)
{
	if (m_wave_select == 0 && m_mixer_external == 0)
		LOGMASKED(LOG_VALUES, "%f V didn't cut it\n", m_values[WAVE_SELECT]);

	const u64 input_mask = get_sound_requested_inputs_mask();
	const bool streaming_cv = input_mask & 0x1fe;

	// loop over samples
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		// take into account any streaming voltage inputs
		if (streaming_cv)
		{
			for (int i = 1; i < INPUT_COUNT; i++)
			{
				if (BIT(input_mask, i))
					set_voltage_internal(i, stream.get(i, sampindex));
			}
		}

		// get the current VCO position and step it forward
		double vco_position = m_vco_position;
		m_vco_position += m_vco_step;

		// clamp VCO position to a fraction
		if (m_vco_position >= 1.0)
			m_vco_position -= floor(m_vco_position);

		// handle the pulse component
		double result = 0;
		if (ENABLE_PULSE && (m_wave_select & WAVE_PULSE))
		{
			// The datasheet mentions a "unique circuit [...] that keeps the
			// average DC level [...] constant regardless of duty cycle". The
			// block diagram shows the pulse signal being subtracted from the
			// pulse width. Here, the pulse width is subtracted from the signal
			// instead, to ensure a phase consistent with the rest of the signal.
			if (vco_position < m_pulse_width)
				result += (1 - m_pulse_width) * PULSE_VOLUME * m_mixer_internal;
			else
				result += (0 - m_pulse_width) * PULSE_VOLUME * m_mixer_internal;
		}

		// handle the sawtooth component
		if (ENABLE_SAWTOOTH && (m_wave_select & WAVE_SAWTOOTH))
			result += SAWTOOTH_VOLUME * m_mixer_internal * (vco_position - 0.5);

		// always compute the triangle waveform which is also used for filter modulation
		double triangle = 2.0 * vco_position;
		if (triangle > 1.0)
			triangle = 2.0 - triangle;
		triangle -= 0.5;

		// handle the triangle component
		if (ENABLE_TRIANGLE && (m_wave_select & WAVE_TRIANGLE))
			result += TRIANGLE_VOLUME * m_mixer_internal * triangle;

		// convert from [-0.5, 0.5] to [-1, 1]
		result *= 2;

		// compute extension input (for Bally/Sente this is the noise)
		if (ENABLE_EXTERNAL && BIT(input_mask, AUDIO_INPUT))
			result += EXTERNAL_VOLUME * m_mixer_external * stream.get(AUDIO_INPUT, sampindex);

		// compute the modulated filter frequency and apply the filter
		// modulation tracks the VCO triangle
		double filter_freq = m_filter_frequency * (1 + m_filter_modulation * triangle);
		result = filter(result, filter_freq);

		// apply AC coupling
		if (ENABLE_AC_COUPLING)
			result = hpf(result);

		// write the sample
		stream.put(0, sampindex, result * m_volume);
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cem3394_device::device_start()
{
	// compute a sample rate
	const int sample_rate = int(round(1.0 / m_inv_sample_rate));

	// allocate stream channels
	m_stream = stream_alloc(get_sound_requested_inputs(), 1, sample_rate);

	save_item(NAME(m_values));
	save_item(NAME(m_wave_select));

	save_item(NAME(m_volume));
	save_item(NAME(m_mixer_internal));
	save_item(NAME(m_mixer_external));

	save_item(NAME(m_vco_position));
	save_item(NAME(m_vco_step));

	save_item(NAME(m_filter_frequency));
	save_item(NAME(m_filter_modulation));
	save_item(NAME(m_filter_resonance));
	save_item(NAME(m_filter_in));
	save_item(NAME(m_filter_out));

	save_item(NAME(m_pulse_width));

	save_item(NAME(m_hpf_mem));

	// Ensures that m_values, and member variables derived from m_values, are
	// properly initialized. Index 0 is unused.
	for (int i = 1; i < INPUT_COUNT; i++)
		set_voltage_internal(i, 0);
}


double cem3394_device::compute_db(double voltage)
{
	// assumes 0.0 == full off, 4.0 == full on, with linear taper, as described in the datasheet

	// above 4.0, maximum volume
	if (voltage >= 4.0)
		return 0.0;

	// below 0.0, minimum volume
	else if (voltage <= 0.0)
		return 90.0;

	// between 2.5 and 4.0, linear from 20dB to 0dB
	else if (voltage >= 2.5)
		return (4.0 - voltage) * (1.0 / 1.5) * 20.0;

	// between 0.0 and 2.5, exponential to 20dB
	else
	{
		double temp = 20.0 * pow(2.0, 2.5 - voltage);
		if (temp < 90.0) return 90.0;
		else return temp;
	}
}


sound_stream::sample_t cem3394_device::compute_db_volume(double voltage)
{
	double temp;

	// assumes 0.0 == full off, 4.0 == full on, with linear taper, as described in the datasheet

	// above 4.0, maximum volume
	if (voltage >= 4.0)
		return 1.0;

	// below 0.0, minimum volume
	else if (voltage <= 0.0)
		return 0;

	// between 2.5 and 4.0, linear from 20dB to 0dB
	else if (voltage >= 2.5)
		temp = (4.0 - voltage) * (1.0 / 1.5) * 20.0;

	// between 0.0 and 2.5, exponential to 20dB
	else
	{
		temp = 20.0 * pow(2.0, 2.5 - voltage);
		if (temp < 50.0) return 0;
	}

	// convert from dB to volume and return
	return powf(0.891251f, temp);
}


void cem3394_device::set_voltage_internal(int input, double voltage)
{
	double temp;

	// don't do anything if no change
	if (voltage == m_values[input])
		return;
	m_values[input] = voltage;

	// switch off the input
	switch (input)
	{
		// frequency varies from -4.0 to +4.0, at 0.75V/octave
		case VCO_FREQUENCY:
			temp = m_vco_zero_freq * pow(2.0, -voltage * (1.0 / 0.75));
			m_vco_step = temp * m_inv_sample_rate;
			LOGMASKED(LOG_CONTROL_CHANGES, "VCO_FREQ=%6.3fV -> freq=%f\n", voltage, temp);
			break;

		// Wave select chooses between triangle, sawtooth, both, or neither.
		// The waveform selection voltages, as specified in the datasheet, are:
		// - none:                 less than -0.5
		// - triangle:            -0.5  - -0.2
		// - triangle + sawtooth:  0.9  -  1.5
		// - sawtooth:             2.3  -  3.9
		// However, some systems (such as the Six-Trak) use voltages outside
		// those ranges. The logic below uses the midpoint of two boundaries as
		// the transition point.
		case WAVE_SELECT:
			m_wave_select &= ~(WAVE_TRIANGLE | WAVE_SAWTOOTH);
			if (voltage >= -0.5 && voltage < 0.35)
				m_wave_select |= WAVE_TRIANGLE;
			else if (voltage >= 0.35 && voltage < 1.9)
				m_wave_select |= WAVE_TRIANGLE | WAVE_SAWTOOTH;
			else if (voltage >= 1.9)
				m_wave_select |= WAVE_SAWTOOTH;
			LOGMASKED(LOG_CONTROL_CHANGES, "WAVE_SEL=%6.3fV -> tri=%d saw=%d\n", voltage, (m_wave_select & WAVE_TRIANGLE) ? 1 : 0, (m_wave_select & WAVE_SAWTOOTH) ? 1 : 0);
			break;

		// pulse width determines duty cycle; 0.0 means 0%, 2.0 means 100%
		case PULSE_WIDTH:
			if (voltage < 0.0)
			{
				m_pulse_width = 0;
				m_wave_select &= ~WAVE_PULSE;
			}
			else if (voltage > 2.0)
			{
				m_pulse_width = 1;
				m_wave_select &= ~WAVE_PULSE;
			}
			else
			{
				m_pulse_width = voltage * 0.5;
				if (LIMIT_WIDTH && m_limit_pw)
					m_pulse_width = MINIMUM_WIDTH + (MAXIMUM_WIDTH - MINIMUM_WIDTH) * m_pulse_width;
				m_wave_select |= WAVE_PULSE;
			}
			LOGMASKED(LOG_CONTROL_CHANGES, "PULSE_WI=%6.3fV -> raw=%f adj=%f\n", voltage, voltage * 0.5, m_pulse_width);
			break;

		// final gain is pretty self-explanatory; 0.0 means ~90dB, 4.0 means 0dB
		case FINAL_GAIN:
			m_volume = compute_db_volume(voltage);
			LOGMASKED(LOG_CONTROL_CHANGES, "TOT_GAIN=%6.3fV -> vol=%f\n", voltage, m_volume);
			break;

		// mixer balance is a pan between the external input and the internal input
		// 0.0 is equal parts of both; positive values favor external, negative favor internal
		case MIXER_BALANCE:
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
			LOGMASKED(LOG_CONTROL_CHANGES, " BALANCE=%6.3fV -> int=%f ext=%f\n", voltage, m_mixer_internal, m_mixer_external);
			break;

		// filter frequency varies from -3.0 to +4.0, at 0.375V/octave
		case FILTER_FREQUENCY:
			m_filter_frequency = m_filter_zero_freq * pow(2.0, -voltage * (1.0 / 0.375));
			LOGMASKED(LOG_CONTROL_CHANGES, "FLT_FREQ=%6.3fV -> freq=%f\n", voltage, m_filter_frequency);
			break;

		// modulation depth is 0.01*freq at 0V and 2.0*freq at 3.5V
		case MODULATION_AMOUNT:
			if (voltage < 0.0)
				m_filter_modulation = 0.01;
			else if (voltage > 3.5)
				m_filter_modulation = 1.99;
			else
				m_filter_modulation = (voltage * (1.0 / 3.5)) * 1.98 + 0.01;
			LOGMASKED(LOG_CONTROL_CHANGES, "FLT_MODU=%6.3fV -> mod=%f\n", voltage, m_filter_modulation);
			break;

		// this is not yet implemented
		case FILTER_RESONANCE:
			if (voltage < 0.0)
				m_filter_resonance = 0.0;
			else if (voltage > 2.5)
				m_filter_resonance = 1.0;
			else
				m_filter_resonance = voltage * (1.0 / 2.5);
			LOGMASKED(LOG_CONTROL_CHANGES, "FLT_RESO=%6.3fV -> mod=%f\n", voltage, m_filter_resonance);
			break;

		default:
			fatalerror("%s - unrecognized input: %d\n", tag(), input);
			break;
	}
}


void cem3394_device::set_voltage(int input, double voltage)
{
	if (input < 1 || input >= INPUT_COUNT)
		fatalerror("%s - Invalid input to set_voltage(): %d\n", tag(), input);

	if (BIT(get_sound_requested_inputs_mask(), input))
		fatalerror("%s - Cannot call set_voltage(%d, ...). %d is a streaming input.\n", tag(), input, input);

	if (voltage == m_values[input])
		return;

	m_stream->update();
	set_voltage_internal(input, voltage);
}

double cem3394_device::get_voltage(int input)
{
	if (input < 1 || input >= INPUT_COUNT)
		fatalerror("%s - Invalid input to get_voltage(): %d\n", tag(), input);

	if (BIT(get_sound_requested_inputs_mask(), input))
		m_stream->update();

	return m_values[input];
}

double cem3394_device::get_parameter(int input)
{
	const double voltage = get_voltage(input);

	switch (input)
	{
		case VCO_FREQUENCY:
			return m_vco_zero_freq * pow(2.0, -voltage * (1.0 / 0.75));

		case WAVE_SELECT:
			return voltage;

		case PULSE_WIDTH:
			if (voltage <= 0.0)
				return 0.0;
			else if (voltage >= 2.0)
				return 1.0;
			else
				return voltage * 0.5;

		case FINAL_GAIN:
			return compute_db(voltage);

		case MIXER_BALANCE:
			return voltage * 0.25;

		case MODULATION_AMOUNT:
			if (voltage < 0.0)
				return 0.01;
			else if (voltage > 3.5)
				return 1.99;
			else
				return (voltage * (1.0 / 3.5)) * 1.98 + 0.01;

		case FILTER_RESONANCE:
			if (voltage < 0.0)
				return 0.0;
			else if (voltage > 2.5)
				return 1.0;
			else
				return voltage * (1.0 / 2.5);

		case FILTER_FREQUENCY:
			return m_filter_zero_freq * pow(2.0, -voltage * (1.0 / 0.375));
	}
	return 0.0;
}
