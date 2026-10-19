// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "cem3372.h"

#include "machine/rescap.h"
#include "sound/flt_rc.h"
#include "sound/va_ops.h"

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


// *** CEM3372_VCF ***

cem3372_vcf_device::cem3372_vcf_device(const machine_config &mconfig, const char *tag, device_t *owner, const stream_inputs &inputs)
	: device_t(mconfig, CEM3372_VCF, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_inputs(inputs)
	, m_stream(nullptr)
	, m_input_a_vca(*this, "input_a_vca")
	, m_input_b_vca(*this, "input_b_vca")
	, m_vcf(*this, "vcf")
	, m_c({-1, -1, -1, -1})
	, m_level_a_cv(0)
	, m_level_b_cv(0)
	, m_freq_cv(0)
	, m_res_cv(0)
{
}

cem3372_vcf_device::cem3372_vcf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cem3372_vcf_device(mconfig, tag, owner, stream_inputs())
{
}

void cem3372_vcf_device::device_add_mconfig(machine_config &config)
{
	if (m_inputs.channel_a)
		m_inputs.channel_a->add_route(0, m_input_a_vca, 1.0);
	else
		VA_CONST(config, "zero_a").set_value(0).add_route(0, m_input_a_vca, 1.0);

	if (m_inputs.channel_b)
		m_inputs.channel_b->add_route(0, m_input_b_vca, 1.0);
	else
		VA_CONST(config, "zero_b").set_value(0).add_route(0, m_input_b_vca, 1.0);

	// From the datasheet:
	// - Last stage gain: 100x
	// - Input VCA typical gain: 35x
	// - Input signal for 5% THD: 75mV peak-to-peak.
	// - Output signal for 5% THD: 2.5V peak-to-peak.
	//   - Output/Input for 5% THD: ~33.3 ~= 35.
	// - The output of the input VCA is scaled down by some amount.
	//
	// Given all the above, the output of the input VCA is scaled by ~1/100.
	//
	// That scaling matches the Xpander filter circuit, which applies ~100x gain
	// to the output of the first 3 stages, before mixing them with the VCF output.

	VA_VCA(config, m_input_a_vca).add_route(0, m_vcf, 1.0 / 100.0);
	VA_VCA(config, m_input_b_vca).add_route(0, m_vcf, 1.0 / 100.0);

	VA_LPF4(config, m_vcf)
		// According to the datasheet, the passband gain remains constant with varying resonance.
		.configure_bass_gain_comp(1)
		.configure_drive(25)  // TODO: figure out a more accurate value.
		.add_route(va_lpf4_device::OUTPUT_LPF1, *this, 1.0, OUTPUT_CAP1)
		.add_route(va_lpf4_device::OUTPUT_LPF2, *this, 1.0, OUTPUT_CAP2)
		.add_route(va_lpf4_device::OUTPUT_LPF3, *this, 1.0, OUTPUT_CAP3)
		.add_route(va_lpf4_device::OUTPUT_LPF4, *this, 1.0, OUTPUT_CAP4)
		.add_route(va_lpf4_device::OUTPUT_LPF4, *this, 100.0, OUTPUT_VCFOUT);
}

void cem3372_vcf_device::device_start()
{
	m_stream = stream_alloc(5, 5, machine().sample_rate());
	save_item(NAME(m_c));
	save_item(NAME(m_level_a_cv));
	save_item(NAME(m_level_b_cv));
	save_item(NAME(m_freq_cv));
	save_item(NAME(m_res_cv));
}

void cem3372_vcf_device::device_reset()
{
	update_freq_scale();
	update_freq();
}

void cem3372_vcf_device::sound_stream_update(sound_stream &stream)
{
	const u8 outputs_mask = get_sound_requested_outputs_mask();
	const int output_count = get_sound_requested_outputs();

	for (int i = 0; i < output_count; ++i)
	{
		if (BIT(outputs_mask, i))
			stream.copy(i, i);
	}
}

void cem3372_vcf_device::set_level_a_cv(float cv)
{
	if (cv == m_level_a_cv)
		return;

	m_level_a_cv = cv;
	const float gain = input_gain(cv);
	m_input_a_vca->set_fixed_gain_cv(gain);

	LOG("%s: Level A - cv: %f, gain: %f\n", tag(), cv, gain);
}

void cem3372_vcf_device::set_level_b_cv(float cv)
{
	if (cv == m_level_b_cv)
		return;

	m_level_b_cv = cv;
	const float gain = input_gain(cv);
	m_input_b_vca->set_fixed_gain_cv(gain);

	LOG("%s: Level B - cv: %f, gain: %f\n", tag(), cv, gain);
}

void cem3372_vcf_device::set_freq_cv(float cv)
{
	if (cv == m_freq_cv)
		return;

	m_freq_cv = cv;
	update_freq();
}

void cem3372_vcf_device::set_res_cv(float cv)
{
	if (cv == m_res_cv)
		return;

	m_res_cv = cv;

	// TODO: response curve is unknown. Treating it as linear for now.
	// Typical voltage required for self-oscillation: 2.8
	const float res_gain = 4.0F * std::clamp(cv, 0.0F, 5.0F) / 2.8F;
	m_vcf->set_fixed_res_cv(res_gain);

	LOG("%s: Res - cv: %f, gain: %f\n", tag(), cv, res_gain);
}

float cem3372_vcf_device::input_gain(float cv)
{
	// 0-5V maps to 0-35x gain (typical).
	// The two input VCAs use audio taper control scales.
	return 35.0F * float(RES_AUDIO_POT_LAW(std::clamp(cv, 0.0F, 5.0F) / 5.0F));
}

cem3372_vcf_device &cem3372_vcf_device::set_c(int stage, float c)
{
	if (stage < 0 || stage >= m_c.size())
		fatalerror("%s: Invalidate stage number in set_c: %d\n", tag(), stage);

	if (c == m_c[stage])
		return *this;

	m_c[stage] = c;

	if (m_stream)
		update_freq_scale();

	return *this;
}

void cem3372_vcf_device::update_freq()
{
	// Typical frequency at CV = 0 when Ca=Cb=Cc=0.03uF and Cd=300pF: 1KHz.
	// Determine frequency at CV = 0 with the configured capacitor instead.
	const float freq0 = 1000.0F * (float(CAP_U(0.03)) / m_c[0]);

	// Typical frequency control scale: 19.0 mV/octave.
	const float freq = freq0 * powf(2.0F, m_freq_cv / 0.019F);

	m_vcf->set_fixed_freq_cv(freq);
	LOG("%s: Freq - cv: %f, freq: %f\n", tag(), m_freq_cv, freq);
}

void cem3372_vcf_device::update_freq_scale()
{
	// The last stage has a capacitor that should be 1% of the rest, for cutoff
	// frequencies to match. C_SCALE is used to convert all capacitors to the
	// same scale for the calculations below.
	constexpr float C_SCALE[4] = { 1.0F, 1.0F, 1.0F, 100.0F };

	for (int i = 0; i < 4; ++i)
	{
		if (m_c[i] <= 0.0F)
			fatalerror("%s: Uninitialized or invalid capacitor value: C%d: %f\n", tag(), i, m_c[i]);

		// Compute stage i's frequency scale, relative to the first stage frequency.
		// Frequency is inversely proportional to capacitance.
		const float freq_scale = 1.0F / (C_SCALE[i] * m_c[i] / m_c[0]);
		m_vcf->set_freq_scale(i, freq_scale);

		LOG("%s: Freq scale: %d, %f\n", tag(), i, freq_scale);
	}
	update_freq();
}


// *** CEM3372_VCA ***

cem3372_vca_device::cem3372_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: va_vca_device(mconfig, CEM3372_VCA, tag, owner, clock)
	, m_output_scale(1.0F)
{
}

cem3372_vca_device &cem3372_vca_device::configure_inv_opamp_voltage_output(float r_feedback)
{
	m_output_scale = -r_feedback;
	return *this;
}

void cem3372_vca_device::device_start()
{
	va_vca_device::device_start();

	const u64 accepted_inputs_mask = (1 << INPUT_AUDIO) | (1 << INPUT_GAIN);
	if (get_sound_requested_inputs_mask() & ~accepted_inputs_mask)
		fatalerror("%s: cem3372_vca - invalid input(s) connected\n", tag());
}

float cem3372_vca_device::cv_to_gain(float cv) const
{
	// The control scale is "very linear". Maximum gain is 0.93.
	const float gain = 0.93F * std::clamp(cv, 0.0F, 5.0F) / 5.0F;
	LOG("%s: Final VCA - cv: %f, output_scale: %f, gain: %f\n",
		tag(), cv, m_output_scale, gain);
	return gain * m_output_scale;
}


DEFINE_DEVICE_TYPE(CEM3372_VCF, cem3372_vcf_device, "cem3372_vcf", "CEM3372 signal processor, VCF section")
DEFINE_DEVICE_TYPE(CEM3372_VCA, cem3372_vca_device, "cem3372_vca", "CEM3372 signal processor, VCA section")
