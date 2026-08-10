// license:BSD-3-Clause
// copyright-holders:Aaron Giles,m1macrophage
/***************************************************************************

    Curtis Electromusic Specialties CEM3394 µP-Controllable Synthesizer Voice

    This driver handles CEM-3394 analog synth chip.

***************************************************************************/

#include "emu.h"
#include "cem3394.h"
#include "sound/flt_rc.h"

#include <limits>


// logging
#define LOG_CONTROL_CHANGES (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"


// use 0.25 as the base volume for pulses
static constexpr double PULSE_VOLUME = 0.25;

// sawtooth is 27% larger than pulses
static constexpr double SAWTOOTH_VOLUME = PULSE_VOLUME * 1.27f;

// triangle is 27% larger than sawtooth
static constexpr double TRIANGLE_VOLUME = SAWTOOTH_VOLUME * 1.27f;

// external input is unknown but let's make it the same as the pulse
static constexpr double EXTERNAL_VOLUME = PULSE_VOLUME;


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


// device type definition
DEFINE_DEVICE_TYPE(CEM3394, cem3394_device, "cem3394", "CEM3394 Synthesizer Voice")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


cem3394_device::cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cem3394_device(mconfig, tag, owner, components(), stream_inputs())
{
}

cem3394_device::cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, const components &comps, const stream_inputs &inputs) :
	device_t(mconfig, CEM3394, tag, owner, 0),
	device_sound_interface(mconfig, *this),
	m_stream_inputs(inputs),
	m_components(comps),
	// datasheet equation for Fout at CV = 0
	m_vco_zero_freq(1.3 / (5.0 * comps.r_vco * comps.c_vco)),
	// datasheet equation for Pzcv
	// Note that "4.3 x 10E-5" in the equation should be 4.3E-5. The surrounding
	// text and the example walkthrough use the correct value.
	m_filter_zero_freq(4.3E-5 / comps.c_vcf),
	m_initialized(false),
	m_stream(nullptr),
	m_vco(*this, "vco"),
	m_filt_freq(*this, "filt_freq"),
	m_filt_fm(*this, "filt_fm"),
	m_vcf(*this, "vcf"),
	m_vca(*this, "vca"),
	m_vco_freq_cv(std::numeric_limits<double>::quiet_NaN()),
	m_mod_amount_cv(std::numeric_limits<double>::quiet_NaN()),
	m_wave_select_cv(std::numeric_limits<double>::quiet_NaN()),
	m_pulse_width_cv(std::numeric_limits<double>::quiet_NaN()),
	m_mixer_balance_cv(std::numeric_limits<double>::quiet_NaN()),
	m_filt_res_cv(std::numeric_limits<double>::quiet_NaN()),
	m_filt_freq_cv(std::numeric_limits<double>::quiet_NaN()),
	m_final_gain_cv(std::numeric_limits<double>::quiet_NaN()),
	m_tri(false),
	m_saw(false),
	m_vco_frequency(500.0),
	m_pulse_width(0),
	m_volume(0),
	m_mixer_internal(0),
	m_mixer_external(0),
	m_filter_frequency(1300),
	m_filter_modulation(0),
	m_filter_resonance(0)
{
}

void cem3394_device::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
}

void cem3394_device::device_add_mconfig(machine_config &config)
{
	// Set up the VCO.
	// Route gains will be adjusted when relevant CVs are updated.
	VA_VCO(config, m_vco)
		.configure_pulse_from_tri(true)
		.configure_pulse_dc_comp(true)
		.add_route(va_vco_device::OUTPUT_RAMP, m_vcf, 0.0, va_lpf4_device::INPUT_AUDIO)
		.add_route(va_vco_device::OUTPUT_PULSE, m_vcf, 0.0, va_lpf4_device::INPUT_AUDIO)
		.add_route(va_vco_device::OUTPUT_TRIANGLE, m_vcf, 0.0, va_lpf4_device::INPUT_AUDIO)
		.add_route(va_vco_device::OUTPUT_TRIANGLE, m_filt_fm, 0.0, va_vca_device::INPUT_AUDIO);

	// Set up the external input.
	if (m_stream_inputs.ext_input != nullptr)
	{
		// Route gain will be adjusted in set_mixer_balance_cv().
		m_stream_inputs.ext_input->add_route(0, m_vcf, 0.0, va_lpf4_device::INPUT_AUDIO);
	}

	// Set up filter frequency control.
	device_sound_interface *filt_freq = nullptr;
	if (m_stream_inputs.filt_freq_cv != nullptr)
	{
		m_stream_inputs.filt_freq_cv->add_route(0, "cv2filtfreq", 1.0);
		filt_freq = &VA_LAMBDA(config, "cv2filtfreq", [this] (float cv) { return stream_op_filter_freq(cv); });
	}
	else
	{
		filt_freq = &VA_CONST(config, m_filt_freq);
	}

	// When the filter's cutoff frequency is modulated, its frequency is:
	//   filt_freq * (1 + filt_mod_amount * triangle) -- triangle's range: [-0.5, 0.5]
	//   = filt_freq + filt_freq * filt_mod_amount * 0.5 * triangle  -- triangle's range: [-1, 1].
	// The two terms in the equation above are summed in m_vcf's INPUT_FREQ node.

	// First term:
	filt_freq->add_route(0, m_vcf, 1.0, va_lpf4_device::INPUT_FREQ);

	// Second term, computed by the m_filt_fm VCA:
	// - The triangle wave (range: [-1, 1]) is applied to m_filt_fm's INPUT_AUDIO
	//   (see VCO configuration above).
	// - The route gain of the above is set to filt_mod_amount * 0.5
	//   (see set_mod_amount_cv()).
	// - filt_freq is applied to INPUT_GAIN (see right below).
	// The above make m_filt_fm's output: filt_freq * filt_mod_amount * 0.5 * triangle
	filt_freq->add_route(0, m_filt_fm, 1.0, va_vca_device::INPUT_GAIN);
	VA_VCA(config, m_filt_fm).add_route(0, m_vcf, 1.0, va_lpf4_device::INPUT_FREQ);

	// Set up the VCF.
	VA_LPF4(config, m_vcf)
		.configure_drive(1.0)
		// According to the datasheet, the filter maintains the apparent loudness
		// constant as resonance increases. The value below was selected by trial
		// and error, to qualitatively match the preceding statement.
		.configure_bass_gain_comp(0.2)
		.add_route(0, "ac_couple", 1.0);

	// Set up AC coupling.
	constexpr double R_AC = 11E3;  // internal AC coupling resistor
	FILTER_RC(config, "ac_couple")
		.set_rc(filter_rc_device::HIGHPASS, R_AC, 0, 0, m_components.c_ac)
		.add_route(0, m_vca, 1.0, va_vca_device::INPUT_AUDIO);

	// Set up the VCA.
	if (m_stream_inputs.final_gain_cv != nullptr)
	{
		m_stream_inputs.final_gain_cv->add_route(0, "cv2ampgain", 1.0);
		VA_LAMBDA(config, "cv2ampgain", [this] (float cv) { return stream_op_amp_gain(cv); })
			.add_route(0, m_vca, 1.0, va_vca_device::INPUT_GAIN);
	}
	VA_VCA(config, m_vca).add_route(0, *this, 1.0);
}

void cem3394_device::device_start()
{
	// allocate stream channels
	m_stream = stream_alloc(1, 1, machine().sample_rate());

	save_item(NAME(m_vco_freq_cv));
	save_item(NAME(m_mod_amount_cv));
	save_item(NAME(m_wave_select_cv));
	save_item(NAME(m_pulse_width_cv));
	save_item(NAME(m_mixer_balance_cv));
	save_item(NAME(m_filt_res_cv));
	save_item(NAME(m_filt_freq_cv));
	save_item(NAME(m_final_gain_cv));

	save_item(NAME(m_tri));
	save_item(NAME(m_saw));
	save_item(NAME(m_vco_frequency));
	save_item(NAME(m_pulse_width));
	save_item(NAME(m_volume));
	save_item(NAME(m_mixer_internal));
	save_item(NAME(m_mixer_external));
	save_item(NAME(m_filter_frequency));
	save_item(NAME(m_filter_modulation));
	save_item(NAME(m_filter_resonance));
}

void cem3394_device::device_reset()
{
	if (!m_initialized)
	{
		// This initialization is only required on startup, not on reset. But it
		// can't be done in device_start() because it requires the owned devices
		// to be running. Using m_initialized to ensure this is only done once.

		set_vco_freq_cv(0.0);
		set_mod_amount_cv(0.0);
		set_wave_select_cv(0.0);
		set_pulse_width_cv(0.0);
		set_mixer_balance_cv(0.0);
		set_filt_res_cv(0.0);

		if (m_stream_inputs.filt_freq_cv != nullptr)
			set_filt_freq_cv_internal(0.0);
		else
			set_filt_freq_cv(0.0);

		if (m_stream_inputs.final_gain_cv != nullptr)
			set_final_gain_cv_internal(0.0);
		else
			set_final_gain_cv(0.0);

		m_initialized = true;
	}
}


void cem3394_device::set_vco_freq_cv(double cv)
{
	if (cv == m_vco_freq_cv)
		return;

	m_vco_freq_cv = cv;

	// frequency varies from -4.0 to +4.0, at 0.75V/octave
	m_vco_frequency = m_vco_zero_freq * pow(2.0, -cv * (1.0 / 0.75));
	LOGMASKED(LOG_CONTROL_CHANGES, "VCO_FREQ=%6.3fV -> freq=%f\n", cv, m_vco_frequency);

	m_vco->set_freq_ctrl(m_vco_frequency);
}

void cem3394_device::set_mod_amount_cv(double cv)
{
	if (cv == m_mod_amount_cv)
		return;

	m_mod_amount_cv = cv;

	// At max depth, the frequency is modulated from 0.01x to 2.0x. This
	// implementation modulates from 0.01x to 1.99x, for simpler math. 0%
	// modulation is achieved when the CV is below -0.3 - +0.1 V. Using a
	// threshold of 0.01 here, to ensure the min CV set by the sixtrak results
	// in 0% modulation. 100% modulation is achieved when the CV is above 3 - 4 V.
	// Using the midpoint (3.5) as the threshold here.
	if (cv < 0.01)
		m_filter_modulation = 0;
	else if (cv > 3.5)
		m_filter_modulation = 1.98;
	else
		m_filter_modulation = 1.98 * (cv - 0.01) / (3.5 - 0.01);
	LOGMASKED(LOG_CONTROL_CHANGES, "FLT_MODU=%6.3fV -> mod=%f\n", cv, m_filter_modulation);

	// See m_filt_fm setup in device_add_mconfig().
	const double mod_gain = 0.5 * m_filter_modulation;
	m_vco->set_route_gain(va_vco_device::OUTPUT_TRIANGLE, m_filt_fm, va_vca_device::INPUT_AUDIO, mod_gain);
}

void cem3394_device::set_wave_select_cv(double cv)
{
	if (cv == m_wave_select_cv)
		return;

	m_wave_select_cv = cv;

	// Wave select chooses between triangle, sawtooth, both, or neither.
	// The waveform selection voltages, as specified in the datasheet, are:
	// - none:                 less than -0.5
	// - triangle:            -0.5  - -0.2
	// - triangle + sawtooth:  0.9  -  1.5
	// - sawtooth:             2.3  -  3.9
	// However, some systems (such as the Six-Trak) use voltages outside those
	// ranges. The logic below uses the midpoint of two boundaries as the
	// transition point.
	m_tri = (cv >= -0.5 && cv < 1.9);
	m_saw = (cv >= 0.35);
	LOGMASKED(LOG_CONTROL_CHANGES, "WAVE_SEL=%6.3fV -> tri=%d saw=%d\n", cv, m_tri, m_saw);

	update_osc_mix();
}

void cem3394_device::set_pulse_width_cv(double cv)
{
	if (cv == m_pulse_width_cv)
		return;

	m_pulse_width_cv = cv;

	// pulse width determines duty cycle; 0.0 means 0%, 2.0 means 100%
	// A pulse width of 0 or 1 will result in the waveform being 0 for its
	// entire cycle, which means it is essentially disabled.
	m_pulse_width = std::clamp(0.5 * cv, 0.0, 1.0);
	LOGMASKED(LOG_CONTROL_CHANGES, "PULSE_WI=%6.3fV -> adj=%f\n", cv, m_pulse_width);

	m_vco->set_pw_ctrl(m_pulse_width);
}

void cem3394_device::set_mixer_balance_cv(double cv)
{
	if (cv == m_mixer_balance_cv)
		return;

	m_mixer_balance_cv = cv;

	// mixer balance is a pan between the external input and the internal input
	// 0.0 is equal parts of both; positive values favor external, negative favor internal
	if (cv >= 0.0)
	{
		m_mixer_internal = compute_db_volume(3.55 - cv);
		m_mixer_external = compute_db_volume(3.55 + 0.45 * (cv * 0.25));
	}
	else
	{
		m_mixer_internal = compute_db_volume(3.55 - 0.45 * (cv * 0.25));
		m_mixer_external = compute_db_volume(3.55 + cv);
	}
	LOGMASKED(LOG_CONTROL_CHANGES, " BALANCE=%6.3fV -> int=%f ext=%f\n", cv, m_mixer_internal, m_mixer_external);

	const double pulse_gain = PULSE_VOLUME * m_mixer_internal;
	m_vco->set_route_gain(va_vco_device::OUTPUT_PULSE, m_vcf, va_lpf4_device::INPUT_AUDIO, pulse_gain);
	update_osc_mix();

	if (m_stream_inputs.ext_input != nullptr)
	{
		const double ext_gain = EXTERNAL_VOLUME * m_mixer_external;
		m_stream_inputs.ext_input->set_route_gain(0, m_vcf, va_lpf4_device::INPUT_AUDIO, ext_gain);
	}
}

void cem3394_device::set_filt_res_cv(double cv)
{
	if (cv == m_filt_res_cv)
		return;

	m_filt_res_cv = cv;

	// According to the datasheet, the "no resonance" CV threshold is between
	// 0V and 0.3V, and oscillation starts between 2.0 and 3.0 V, with a typical
	// threshold of 2.5V. So 2.5V will map to a resonance gain of 4.
	if (cv < 0.0)
		m_filter_resonance = 0.0;
	else
		m_filter_resonance = 4.0 * cv / 2.5;
	LOGMASKED(LOG_CONTROL_CHANGES, "FLT_RESO=%6.3fV -> mod=%f\n", cv, m_filter_resonance);

	m_vcf->set_fixed_res_cv(m_filter_resonance);
}

void cem3394_device::set_filt_freq_cv_internal(double cv)
{
	if (cv == m_filt_freq_cv)
		return;

	m_filt_freq_cv = cv;

	// filter frequency varies from -3.0 to +4.0, at 0.375V/octave
	m_filter_frequency = m_filter_zero_freq * pow(2.0, -cv * (1.0 / 0.375));
	LOGMASKED(LOG_CONTROL_CHANGES, "FLT_FREQ=%6.3fV -> freq=%f\n", cv, m_filter_frequency);
}

void cem3394_device::set_filt_freq_cv(double cv)
{
	if (m_stream_inputs.filt_freq_cv != nullptr)
		fatalerror("%s - Cannot call set_fill_freq_cv() when filt_freq is a streaming input.\n", tag());
	if (cv == m_filt_freq_cv)
		return;

	set_filt_freq_cv_internal(cv);
	m_filt_freq->set_value(m_filter_frequency);
}

void cem3394_device::set_final_gain_cv_internal(double cv)
{
	if (cv == m_final_gain_cv)
		return;

	m_final_gain_cv = cv;

	// final gain is pretty self-explanatory; 0.0 means ~90dB, 4.0 means 0dB
	m_volume = compute_db_volume(cv);
	LOGMASKED(LOG_CONTROL_CHANGES, "TOT_GAIN=%6.3fV -> vol=%f\n", cv, m_volume);
}

void cem3394_device::set_final_gain_cv(double cv)
{
	if (m_stream_inputs.final_gain_cv != nullptr)
		fatalerror("%s - Cannot call set_final_gain_cv() when final_gain is a streaming input.\n", tag());
	if (cv == m_final_gain_cv)
		return;

	set_final_gain_cv_internal(cv);
	m_vca->set_fixed_gain_cv(m_volume);
}


double cem3394_device::vco_freq()
{
	return m_vco_frequency;
}

double cem3394_device::filt_res()
{
	return m_filter_resonance;
}

double cem3394_device::filt_freq()
{
	if (m_stream_inputs.filt_freq_cv != nullptr)
		m_stream->update();
	return m_filter_frequency;
}

double cem3394_device::final_gain()
{
	if (m_stream_inputs.final_gain_cv != nullptr)
		m_stream->update();
	return m_volume;
}


double cem3394_device::compute_db(double voltage)
{
	// assumes 0.0 == full off, 4.0 == full on, with linear taper, as described in the datasheet
	// the typical max attenuation is 90dB, according to the datasheet

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
		if (temp > 90.0) return 90.0;
		else return temp;
	}
}

sound_stream::sample_t cem3394_device::compute_db_volume(double voltage)
{
	// convert from dB to volume and return
	return powf(0.891251f, compute_db(voltage));
}

void cem3394_device::update_osc_mix()
{
	const double tri_gain = m_tri ? (TRIANGLE_VOLUME * m_mixer_internal) : 0;
	m_vco->set_route_gain(va_vco_device::OUTPUT_TRIANGLE, m_vcf, va_lpf4_device::INPUT_AUDIO, tri_gain);

	const double saw_gain = m_saw ? (SAWTOOTH_VOLUME * m_mixer_internal) : 0;
	m_vco->set_route_gain(va_vco_device::OUTPUT_RAMP, m_vcf, va_lpf4_device::INPUT_AUDIO, saw_gain);
}

float cem3394_device::stream_op_filter_freq(float voltage)
{
	set_filt_freq_cv_internal(voltage);
	return m_filter_frequency;
}

float cem3394_device::stream_op_amp_gain(float voltage)
{
	set_final_gain_cv_internal(voltage);
	return m_volume;
}

