// license:BSD-3-Clause
// copyright-holders:m1macrophage,Olivier Galibert

#include "emu.h"
#include "va_vcf.h"
#include "machine/rescap.h"

#include <cfloat>

va_lpf4_device::va_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_lpf4_device(mconfig, VA_LPF4, tag, owner, clock)
{
}

va_lpf4_device::va_lpf4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_input_gain(1)
	, m_fc(0)
	, m_res(0)
{
	std::fill(m_a.begin(), m_a.end(), 0);
	std::fill(m_b.begin(), m_b.end(), 0);
	std::fill(m_x.begin(), m_x.end(), 0);
	std::fill(m_y.begin(), m_y.end(), 0);
}

va_lpf4_device& va_lpf4_device::configure_input_gain(float gain)
{
	m_input_gain = gain;
	return *this;
}

void va_lpf4_device::set_fixed_freq_cv(float freq_cv)
{
	if (!m_stream)
		fatalerror("%s: set_fixed_freq_cv() cannot be called before device_start()\n", tag());
	if (BIT(get_sound_requested_inputs_mask(), INPUT_FREQ))
		fatalerror("%s: Cannot set a fixed frequency CV when streaming it.\n", tag());

	const float fc = cv_to_freq(freq_cv);
	if (fc == m_fc)
		return;

	m_stream->update();
	m_fc = fc;
	recalc_filter();
}

void va_lpf4_device::set_fixed_res_cv(float res_cv)
{
	if (!m_stream)
		fatalerror("%s: set_fixed_res_cv() cannot be called before device_start()\n", tag());
	if (BIT(get_sound_requested_inputs_mask(), INPUT_RES))
		fatalerror("%s: Cannot set a fixed resonance CV when streaming it.\n", tag());

	const float res = cv_to_res(res_cv);
	if (res == m_res)
		return;

	m_stream->update();
	m_res = res;
	recalc_filter();
}

float va_lpf4_device::get_freq()
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_FREQ))
		m_stream->update();
	return m_fc;
}

float va_lpf4_device::get_res()
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_RES))
		m_stream->update();
	return m_res;
}

float va_lpf4_device::cv_to_freq(float freq_cv) const
{
	return freq_cv;
}

float va_lpf4_device::cv_to_res(float res_cv) const
{
	return res_cv;
}

void va_lpf4_device::device_start()
{
	if (!BIT(get_sound_requested_inputs_mask(), INPUT_AUDIO))
		fatalerror("%s: requires input 0 to be connected.\n", tag());
	if (get_sound_requested_inputs_mask() & ~u64(7))
		fatalerror("%s: can only have inputs 0-2 connected.\n", tag());

	save_item(NAME(m_fc));
	save_item(NAME(m_res));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_x));
	save_item(NAME(m_y));

	m_stream = stream_alloc(get_sound_requested_inputs(), 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	recalc_filter();
}

/*
    A 4-level lowpass filter with a loopback:


             +-[+]-<-[*-1]--------------------------+
             |  |                                   |
             ^ [*r]                                 |
             |  |                                   |
             |  v                                   ^
    input ---+-[+]--[LPF]---[LPF]---[LPF]---[LPF]---+--- output

    All 4 LPFs are identical, with a transconductance G:

    output = 1/(1+s/G)^4 * ( (1+r)*input - r*output)

    or

    output = input * (1+r)/((1+s/G)^4+r)

    to which the usual z-transform can be applied (see votrax.c)
*/
void va_lpf4_device::sound_stream_update(sound_stream &stream)
{
	const bool streaming_freq = BIT(get_sound_requested_inputs_mask(), INPUT_FREQ);
	const bool streaming_res = BIT(get_sound_requested_inputs_mask(), INPUT_RES);
	const bool streaming_cv = streaming_freq || streaming_res;

	const int n = stream.samples();
	for(int i = 0; i < n; ++i)
	{
		if (streaming_cv)
		{
			bool recalc = false;
			if (streaming_freq)
			{
				const float fc = cv_to_freq(stream.get(INPUT_FREQ, i));
				if (fc != m_fc)
				{
					m_fc = fc;
					recalc = true;
				}
			}
			if (streaming_res)
			{
				const float res = cv_to_res(stream.get(INPUT_RES, i));
				if (res != m_res)
				{
					m_res = res;
					recalc = true;
				}
			}
			if (recalc)
				recalc_filter();
		}

		const float x = stream.get(INPUT_AUDIO, i) * m_input_gain;
		const float y = (x * m_a[0]
						+ m_x[0] * m_a[1] + m_x[1] * m_a[2] + m_x[2] * m_a[3] + m_x[3] * m_a[4]
						- m_y[0] * m_b[1] - m_y[1] * m_b[2] - m_y[2] * m_b[3] - m_y[3] * m_b[4]) / m_b[0];
		memmove(&m_x[1], &m_x[0], 3 * sizeof(float));
		memmove(&m_y[1], &m_y[0], 3 * sizeof(float));
		m_x[0] = x;
		m_y[0] = y;
		stream.put(0, i, y);

		// When the input goes quiet, the filter can oscillate continuously at
		// very low volume and, in some cases, eventually "explode". Detect low
		// volume states and stop any oscillations.
		bool quiet = true;
		for (const auto my : m_y)
		{
			if (fabsf(my) > 1e-20)
			{
				quiet = false;
				break;
			}
		}
		if (quiet)
			std::fill(m_y.begin(), m_y.end(), 0);
	}
}

void va_lpf4_device::recalc_filter()
{
	const float T = 1.0F / m_stream->sample_rate();
	const float w = 2 * float(M_PI) * m_fc;

	// Using the "bounded cutoff prewarping" strategy described in Zavalishin's
	// "The art of VA filter design": if the cutoff is larger than some bound
	// w_max (16KHz in the book), then use w_max instead of the cutoff as the
	// prewarp point. The argument is that there is no point improving the filter
	// response accuracy at inaudible frequencies, at the expense of accuracy at
	// audible ones. This is more relevant to HPFs, but a useful side-effect for
	// LPFs is that the cutoff frequency can be near or beyond Nyquist, which
	// does not work well with standard cutoff prewarping.
	// Here, we set the max at 16KHz (same as in the book). But for low sample
	// rates, we use a fraction of Nyquist instead.
	const float w_max = 2 * float(M_PI) * std::min(0.75F * m_stream->sample_rate() / 2, 16'000.0F);
	float g = 0;
	if (w <= w_max)
		g = tanf(w * T / 2);
	else
		g = tanf(w_max * T / 2) / w_max * w;

	const float gzc = 1 / g;
	const float gzc2 = gzc * gzc;
	const float gzc3 = gzc2 * gzc;
	const float gzc4 = gzc3 * gzc;
	const float r1 = 1 + m_res;

	m_a[0] = r1;
	m_a[1] = 4 * r1;
	m_a[2] = 6 * r1;
	m_a[3] = 4 * r1;
	m_a[4] = r1;

	m_b[0] =      r1 + 4 * gzc + 6 * gzc2 + 4 * gzc3 + gzc4;
	m_b[1] = 4 * (r1 + 2 * gzc            - 2 * gzc3 - gzc4);
	m_b[2] = 6 * (r1           - 2 * gzc2            + gzc4);
	m_b[3] = 4 * (r1 - 2 * gzc            + 2 * gzc3 - gzc4);
	m_b[4] =      r1 - 4 * gzc + 6 * gzc2 - 4 * gzc3 + gzc4;
}

// Parallel combination of the external feedback resistor (recommended value is
// 100K) and impedance of each gain cell. This affects a lot of calculations.
// See datasheet.
const float cem3320_lpf4_device::R_EQ = RES_2_PARALLEL(RES_K(100), RES_M(1));

cem3320_lpf4_device::cem3320_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, double c_p)
	: va_lpf4_device(mconfig, CEM3320_LPF4, tag, owner, 0)
	, m_cv2freq(1)
	, m_res_enabled(false)
	, m_r_rc(1)
	, m_res_a(1)
{
	// See cem3320_lpf4_device::cv_to_freq() for info on these equations.
	constexpr float AI0 = 0.9F;  // From the datasheet.
	m_cv2freq = AI0 / (2 * float(M_PI) * R_EQ * float(c_p));
	configure_input_gain(R_EQ);
}

cem3320_lpf4_device::cem3320_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cem3320_lpf4_device(mconfig, tag, owner, CAP_P(300))  // Arbitrarily choosing the example value in the datasheet.
{
}

cem3320_lpf4_device &cem3320_lpf4_device::configure_voltage_input(float r_i)
{
	configure_input_gain((1.0 / r_i) * R_EQ);
	return *this;
}

cem3320_lpf4_device &cem3320_lpf4_device::configure_resonance(float r_rc, float r_ri)
{
	return configure_resonance(r_rc, r_ri, -1, 1);
}

cem3320_lpf4_device &cem3320_lpf4_device::configure_resonance(float r_rc, float r_ri, float r_ri_gnd, float external_gain)
{
	// See cv_to_res() for details on the equations here.
	constexpr float Z_RI = RES_K(3.6);  // Nominal input impedance of pin 8.
	const float z_input = (r_ri_gnd > 0) ? RES_2_PARALLEL(Z_RI, r_ri_gnd) : Z_RI;
	m_res_a = external_gain * z_input / r_ri;

	m_r_rc = r_rc;
	m_res_enabled = true;
	return *this;
}

float cem3320_lpf4_device::cv_to_freq(float freq_cv) const
{
	// From the datasheet, the pole frequency is given by:
	// f_p = AI0 / (2 * PI * R_EQ * C_P) * exp(-V_C / V_T), where:
	// - V_C ~ Frequency control voltage at pin 12.
	// - V_T ~ Thermal voltage.
	// - AI0 ~ Gain when V_C = 0. Typically 0.9, can range from 0.7 to 1.3.
	// - R_EQ ~ Parallel combination of R_F and 1MOhm.
	// - R_F ~ External feedback resistor. Usually 100K.
	// - C_P ~ External capacitor.
	constexpr float VT = 0.0252F;  // Thermal voltage at 20C.
	// m_cv2freq caches: AI0 / (2 * PI * R_EQ * C_P).
	return m_cv2freq * expf(-freq_cv / VT);
}

float cem3320_lpf4_device::cv_to_res(float res_cv) const
{
	if (!m_res_enabled)
		fatalerror("%s: Attempting to use resonance, but configure_resonance() was never called.\n", tag());

	// Resonance is applied by having the output of the filter (pin 10) feed
	// back into the resonance input (pin 8), which is routed to the filter's
	// input via an OTA. The control current for the OTA is provided to pin 9.

	// Compute resonance control current.
	const float i_rc = res_cv / m_r_rc;

	// Compute mapping from resonance control current to the OTA's
	// transconductance.

	// The datasheet provides a graph (figure 6) of that mapping but no
	// equations. It calls it a "modified linear scale". The equations below
	// transition smoothly between lines (A[0], B[0]) and (A[1], B[1]), by
	// blending with a 3rd line (A[2], B[2]). Line 0 is the tangent line near
	// X = 0uA, line 1 is the tangent line near X = 300uA, and line 2 connects
	// the Y points of line 1 and 2 at X = 0uA and 300uA respectively.
	// The values below were determined by eyeballing the graph. The result
	// matches the graph decently well, but note that the graph has a max X of
	// 300 uA. Not sure what happens beyond that. The equation below treat that
	// part as (almost) linear.

	constexpr float A[3] = { 500E-6F / 30E-6F, (1600E-6F - 1200E-6F) / 300E-6F, 1600E-6F / 300E-6F };
	constexpr float B[3] = { 0, 1200E-6F, 0 };
	constexpr float C = B[1] / (A[0] - A[1]);  // X at which lines 0 and 1 intersect.
	constexpr float K = 0.015E6F;  // Smoothing factor.
	constexpr float MAX_G_M = 2250E-6F;  // From figure 6.

	const float y = (i_rc <= C) ? (A[0] * i_rc + B[0]) : (A[1] * i_rc + B[1]);
	const float blend = 1.0F / (1.0F + expf(-K * fabsf(i_rc - C)));
	const float g_m = std::min(MAX_G_M, blend * y + (1.0F - blend) * (A[2] * i_rc + B[2]));

	// Convert the transconductance to a gain.

	// This is done by rearranging the datasheet equation for determining R_RI
	// (the signal resistor at pin 8), while also taking into account signal
	// gain that might be applied externally, and any external resistors from
	// pin 8 to ground.

	// With the above in mind, we have:
	// GAIN = (EXTERNAL_GAIN * INPUT_Z / R_RI) * (G * R_EQ - 1), where:
	// - EXTERNAL_GAIN ~ Gain applied to the filter output (pin 10), before
	//   routing it to the resonance input (pin 8).
	// - INPUT_Z ~ Input impedance at pin 8. This is 3.6 KOhm nominal, unless a
	//   resistor to ground is connected externally.
	// - R_RI ~ External resistor between the input signal and pin 8.
	// - G ~ transconductance of the resonance OTA.
	// - R_EQ ~ (R_F || 1MOhm). See datasheet.
	// - R_F ~ external feedback resistor.

	// The (EXTERNAL_GAIN * INPUT_Z / R_RI) factor is computed in
	// configure_resonance() and stored in m_res_a.
	const float gain = m_res_a * (g_m * R_EQ - 1.0F);

	// The equations in the datasheet can result in slightly negative gain
	// values. Clamp those to 0.
	// The CEM3320 supports some gain above 4, which results in an increased
	// self-oscillation amplitude. But the implementation here does not support
	// gain >= 4, so clamp it.
	return std::clamp(gain, 0.0F, 3.99F);
}

DEFINE_DEVICE_TYPE(VA_LPF4, va_lpf4_device, "va_lpf4", "4th order LPF")
DEFINE_DEVICE_TYPE(CEM3320_LPF4, cem3320_lpf4_device, "cem3320_lpf4", "CEM3320-based 4th order LPF")
