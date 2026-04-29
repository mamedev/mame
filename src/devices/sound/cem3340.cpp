// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "cem3340.h"

namespace {

constexpr float VT = 25.7E-3F;  // Thermal voltage constant at 25 deg C.

// Recommended voltage supply and component values in the datasheet. If a
// design deviates meaningfully from these, they can be made configurable.

constexpr float VCC = 15.0F;  // Positive supply voltage.
constexpr float RS = 1.8E3F;  // Resistor between pin 18 and GND.
constexpr float RT = 5.6E3F;  // Resistor between pin 2 and pin 3.

// According to the datasheet, RZ should be trimmed so that:
// pin 2 current = pin 1 current. Or: 22 * VT / RT = 3 / RZ
constexpr float RZ = 3 * RT / (22 * VT);  // Total resistance between pin 1 and pin 3.

constexpr float PW_MAX = VCC / 3.0F;

// See "supplies" section in the datasheet. The minimum for both triangle and
// ramp is always 0.
constexpr float TRIANGLE_MAX = VCC / 3.0F;
constexpr float TRIANGLE_MIN = 0.0F;
constexpr float RAMP_MAX = 2.0F * VCC / 3.0F;
constexpr float RAMP_MIN = 0.0F;

// Computing the exact PULSE max is somewhat involved, and depends on both:
// the pulldown resistor at the PULSE output, and the pulldown voltage (see
// "waveform outputs" section). Using the nominal value provided in the
// "supplies" section instead.
constexpr float PULSE_MAX = VCC - 1.5F;

// Typical configurations pull down to GND. See info in the "waveform outputs"
// section of the datasheet, if you need to make this configurable.
constexpr float PULSE_MIN = 0;

}  // anonymous namespace


cem3340_device::cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, float cf, float rr)
	: device_t(mconfig, CEM3340, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_cf(cf)
	, m_rr(rr)
	, m_stream(nullptr)
	, m_freq_cc(-1)
	, m_freq(10.0F)
	, m_pw_cv(-1)
	, m_pw(0.5F)
	, m_step(0.0F)
	, m_phase(0.0F)
{
}

cem3340_device::cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cem3340_device(mconfig, tag, owner, 1000E-12F, 1.5E6F)  // Values from the datasheet.
{
}

void cem3340_device::set_freq_cc_internal(float freq_cc)
{
	if (freq_cc == m_freq_cc)
		return;

	// Equations shown and/or described in the datasheet.
	const float iom = (22.0F * VT / RT) * (1.0F - freq_cc * RZ / 3.0F);  // Output current of the multiplier.
	const float vb = iom * RS;  // Voltage at the base of the exponential converter.
	const float iref = VCC / m_rr;  // Reference input current at pin 13.
	const float ieg = iref * expf(-vb / VT);  // Output current of the exponential converter.
	m_freq = 3.0F * ieg / (2.0F * VCC * m_cf);  // Oscillation frequency.
	m_step = m_freq / float(m_stream->sample_rate());
	m_freq_cc = freq_cc;
}

void cem3340_device::set_pw_cv_internal(float pw_cv)
{
	if (pw_cv == m_pw_cv)
		return;
	m_pw = std::clamp(pw_cv, 0.0F, PW_MAX) / PW_MAX;
	m_pw_cv = pw_cv;
}

cem3340_device &cem3340_device::set_freq_cc(float freq_cc)
{
	if (!m_stream)  // Need to know the sample rate.
		fatalerror("%s: set_freq_cc() cannot be called before device_start().\n", tag());
	if (freq_cc == m_freq_cc)
		return *this;
	m_stream->update();
	set_freq_cc_internal(freq_cc);
	return *this;
}

cem3340_device &cem3340_device::set_pw_cv(float pw_cv)
{
	if (pw_cv == m_pw_cv)
		return *this;
	if (m_stream)
		m_stream->update();
	set_pw_cv_internal(pw_cv);
	return *this;
}

float cem3340_device::get_freq()
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_FREQ))
		m_stream->update();
	return m_freq;
}

void cem3340_device::device_start()
{
	m_stream = stream_alloc(get_sound_requested_inputs(), get_sound_requested_outputs(), machine().sample_rate());
	save_item(NAME(m_freq_cc));
	save_item(NAME(m_freq));
	save_item(NAME(m_pw_cv));
	save_item(NAME(m_pw));
	save_item(NAME(m_step));
	save_item(NAME(m_phase));
}

// Implementation is based on:
// https://www.martin-finke.de/articles/audio-plugins-018-polyblep-oscillator/
float cem3340_device::poly_blep(float phase) const
{
	float val = 0;
	if (phase < m_step)
	{
		const float t = phase / m_step;
		val = t + t - t * t - 1.0F;
	}
	else if (phase > 1.0F - m_step)
	{
		const float t = (phase - 1.0F) / m_step;
		val = t + t + t * t + 1.0F;
	}
	return val;
}

// Implementation is based on:
// https://dsp.stackexchange.com/questions/54790/polyblamp-anti-aliasing-in-c
float cem3340_device::poly_blamp(float phase) const
{
	float y = 0.0F;
	if (0.0F <= phase && phase < 2.0F * m_step)
	{
		const float x = phase / m_step;
		const float u = 2.0F - x;
		const float u2 = u * u;
		y -= u * u2 * u2;
		if (phase < m_step)
		{
			const float v = 1.0F - x;
			const float v2 = v * v;
			y += 4.0F * v * v2 * v2;
		}
	}
	return y * m_step / 15.0F;
}

// Converts from [-1, 1] to [min_value, max_value]
static inline float transform(float x, float min_value, float max_value)
{
	return (max_value - min_value) * (x + 1.0F) / 2.0F + min_value;
}

// A faster way to do fmod(x, 1.0F). At the time of this writing, the "-bench"
// speed on the prophet5 (11 CEM3340s) improves from ~1480% to ~1680% when
// switching from fmodf(x, 1.0F) to fmodf1(x).
static inline float fmodf1(float x)
{
	// No need to worry about a negative `x` in this application.
	if (x >= 1.0F)
		x -= floorf(x);
	return x;
}

void cem3340_device::sound_stream_update(sound_stream &stream)
{
	const bool streaming_freq = BIT(get_sound_requested_inputs_mask(), INPUT_FREQ);
	const bool streaming_pw = BIT(get_sound_requested_inputs_mask(), INPUT_PW);

	const bool tri_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_TRIANGLE);
	const bool ramp_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_RAMP);
	const bool pulse_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_PULSE);

	const int n = stream.samples();

	for (int i = 0; i < n; ++i)
	{
		if (streaming_freq)
			set_freq_cc_internal(stream.get(INPUT_FREQ, i));
		if (streaming_pw)
			set_pw_cv_internal(stream.get(INPUT_PW, i));

		// Uses the PolyBLEP (for ramp and pulse) and PolyBLAMP (for triangle)
		// algorithms to generate anti-aliased waveforms. Those algorithms start
		// with the "naive" versions of the waveforms, and then apply
		// corrections at waveform discontinuities.
		// See references in poly_blep() and poly_blamp().

		// Needed for both the ramp and triangle waveforms.
		const float naive_ramp = 2.0F * m_phase - 1.0F;  // [-1, 1]

		if (ramp_out)
		{
			const float ramp = naive_ramp - poly_blep(m_phase);
			stream.put(OUTPUT_RAMP, i, transform(ramp, RAMP_MIN, RAMP_MAX));
		}

		if (pulse_out)
		{
			// The pulse waveform is generated by a comparator on the ramp
			// waveform, wired such that it goes high when the PW CV is larger
			// than the (scaled) ramp waveform.
			float pulse = (m_phase < m_pw) ? 1.0F : -1.0F;
			pulse += poly_blep(m_phase);
			pulse -= poly_blep(fmodf1(m_phase + (1.0F - m_pw)));
			stream.put(OUTPUT_PULSE, i, transform(pulse, PULSE_MIN, PULSE_MAX));
		}

		if (tri_out)
		{
			// See reference in poly_blamp(). Note that the signs of the
			// corrections below are opposite of those in the reference, because
			// the triangle wave in the reference is inverted.

			float triangle = 1.0F - 2.0F * fabsf(naive_ramp);

			// Correction at the bottom corner of the triangle.
			triangle -= poly_blamp(m_phase);
			triangle -= poly_blamp(1.0F - m_phase);

			// Correction at the top corner of the triangle.
			const float peak_phase = fmodf1(m_phase + 0.5F);
			triangle += poly_blamp(peak_phase);
			triangle += poly_blamp(1.0F - peak_phase);

			stream.put(OUTPUT_TRIANGLE, i, transform(triangle, TRIANGLE_MIN, TRIANGLE_MAX));
		}

		m_phase = fmodf1(m_phase + m_step);
	}
}

DEFINE_DEVICE_TYPE(CEM3340, cem3340_device, "cem3340", "CEM3340 Voltage Controlled Oscillator")
