// license:BSD-3-Clause
// copyright-holders:m1macrophage,Olivier Galibert

#include "emu.h"
#include "va_vcf.h"

#include <cfloat>

va_lpf4_device::va_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_lpf4_device(mconfig, VA_LPF4, tag, owner, clock)
{
}

va_lpf4_device::va_lpf4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_streamless_sample_rate(0)
	, m_input_gain(1)
	, m_gain_comp(0)
	, m_drive(1)
	, m_fc(0)
	, m_res(0)
	, m_stages()
	, m_alpha0(1)
	, m_G4(1)
	, m_gain_comp_scale(1)
{
}

va_lpf4_device &va_lpf4_device::configure_streamless(u32 sample_rate)
{
	m_streamless_sample_rate = sample_rate;
	return *this;
}

va_lpf4_device &va_lpf4_device::configure_input_gain(float gain)
{
	m_input_gain = gain;
	return *this;
}

va_lpf4_device &va_lpf4_device::configure_bass_gain_comp(float comp)
{
	m_gain_comp = comp;
	return *this;
}

va_lpf4_device &va_lpf4_device::va_lpf4_device::configure_drive(float drive)
{
	m_drive = drive;
	return *this;
}

void va_lpf4_device::set_fixed_freq_cv(float freq_cv)
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_FREQ))
		fatalerror("%s: Cannot set a fixed frequency CV when streaming it.\n", tag());

	const float fc = cv_to_freq(freq_cv);
	if (fc == m_fc)
		return;

	if (m_stream)
		m_stream->update();

	m_fc = fc;
	recalc_filter();
}

void va_lpf4_device::set_fixed_res_cv(float res_cv)
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_RES))
		fatalerror("%s: Cannot set a fixed resonance CV when streaming it.\n", tag());

	const float res = cv_to_res(res_cv);
	if (res == m_res)
		return;

	if (m_stream)
		m_stream->update();

	m_res = res;
	recalc_res();
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
	if (get_sound_requested_outputs() > 0)
	{
		if (!BIT(get_sound_requested_inputs_mask(), INPUT_AUDIO))
			fatalerror("%s: requires input 0 to be connected.\n", tag());
		if (get_sound_requested_inputs_mask() & ~u64(7))
			fatalerror("%s: can only have inputs 0-2 connected.\n", tag());
		if (m_streamless_sample_rate > 0)
			fatalerror("%s: configured as streamless, but the output stream is connected.\n", tag());

		// Using a minimum of 96KHz to reduce aliasing due to distortion.
		m_stream = stream_alloc(get_sound_requested_inputs(), 1, std::max(96000, machine().sample_rate()));
	}
	else if (m_streamless_sample_rate == 0)
	{
		fatalerror(
				"%s: not configured properly. Should either have streams connected, "
				"or be configured as streamless.\n",
				tag());
	}

	save_item(NAME(m_fc));
	save_item(NAME(m_res));
	save_item(STRUCT_MEMBER(m_stages, alpha));
	save_item(STRUCT_MEMBER(m_stages, beta));
	save_item(STRUCT_MEMBER(m_stages, state));
	save_item(NAME(m_alpha0));
	save_item(NAME(m_G4));
	save_item(NAME(m_gain_comp_scale));

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

    The implementation here is based on [1], which is based on the SynthLab SDK
    [2], which itself is based on Zavalishin's ladder filter "TPT"
    discretization in [3].

    [1] https://github.com/ddiakopoulos/MoogLadders/blob/main/src/OberheimVariationModel.h
    [2] https://www.willpirkle.com/synthlab/docs/html/index.html (also described
        in his book: "Designing Software Synthesizer Plugins in C++")
    [3] "The Art of VA Filter Design", V Zavalishin, Chapter 5.3.
*/
sound_stream::sample_t va_lpf4_device::process_sample_internal(sound_stream::sample_t s)
{
	// The chapter references below are for the book "The Art of VA Filter Design".
	// Most of the implementation below is based on Chapter 5.3.

	float sigma = 0;
	for (const filter_stage &stage : m_stages)
		sigma += stage.beta * stage.state;

	float x = s * m_input_gain;

	// Adding a tiny amount of noise to the input signal, to ensure the filter
	// can self-oscillate even when there is no input. See chapter 6, footnote 4.
	const float noise = 2 * (float(machine().rand()) / std::numeric_limits<u32>::max() - 0.5F);  // [-1, 1]
	x += 0.000001F * noise;

	// 'drive' will scale the signal before entering the filter. The scaling
	// will be undone at the output. This is used to fine-tune the balance
	// between self-oscillation and input signal. See chapter 6.3, section
	// "Effects of transient response", including the summary in the last paragraph.
	x *= m_drive;

	// Apply low frequency gain compensation. See first paragraph in chapter 5.4
	// ("feedback shaping"). But instead of scaling by (1 + k), we scale by
	// (1 + a * k) (stored in m_gain_comp_scale) to make the compensation
	// configurable by changing `a`, as per the W. Pirkle book in the function
	// comments above.
	x *= m_gain_comp_scale;

	float u = (x - m_res * sigma) * m_alpha0;

	// Saturation is required for stability at high resonance settings. As a
	// bonus, it better matches analog filters. See intro to chapter 6, and
	// chapter 6.3. Here, we implement "feedforward path saturation" (chapter 6.3).
	// Applying saturation accurately is expensive (chapters 6.4 and 6.5), so we
	// use the "linearization at zero" approximation (chapter 6.6), for now.
	u = tanhf(u);

	for (filter_stage &stage : m_stages)
	{
		const float vn = (u - stage.state) * stage.alpha;
		u = vn + stage.state;
		stage.state = vn + u;
	}

	return u / m_drive;
}

sound_stream::sample_t va_lpf4_device::process_sample(sound_stream::sample_t s)
{
	if (get_sound_requested_outputs() > 0)
		fatalerror("%s: process_sample() can only be used when in streamless mode.\n", tag());
	return process_sample_internal(s);
}

void va_lpf4_device::sound_stream_update(sound_stream &stream)
{
	const bool streaming_freq = BIT(get_sound_requested_inputs_mask(), INPUT_FREQ);
	const bool streaming_res = BIT(get_sound_requested_inputs_mask(), INPUT_RES);

	const int n = stream.samples();
	for(int i = 0; i < n; ++i)
	{
		if (streaming_freq)
		{
			const float fc = cv_to_freq(stream.get(INPUT_FREQ, i));
			if (fc != m_fc)
			{
				m_fc = fc;
				recalc_filter();
			}
		}
		if (streaming_res)
		{
			const float res = cv_to_res(stream.get(INPUT_RES, i));
			if (res != m_res)
			{
				m_res = res;
				recalc_res();
			}
		}
		stream.put(0, i, process_sample_internal(stream.get(INPUT_AUDIO, i)));
	}
}

u32 va_lpf4_device::sample_rate() const
{
	if (m_stream)
		return m_stream->sample_rate();
	else
		return m_streamless_sample_rate;
}

void va_lpf4_device::recalc_res()
{
	m_alpha0 = 1.0F / (1.0F + m_res * m_G4);
	m_gain_comp_scale = 1.0F + m_gain_comp * m_res;
}

void va_lpf4_device::recalc_filter()
{
	const float T = 1.0F / sample_rate();
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
	const float w_max = 2 * float(M_PI) * std::min(0.75F * sample_rate() / 2, 16'000.0F);
	float g = 0;
	if (w <= w_max)
		g = tanf(w * T / 2);
	else
		g = tanf(w_max * T / 2) / w_max * w;

	const float gp1 = 1 + g;
	const float G = g / gp1;
	const float G2 = G * G;
	m_G4 = G2 * G2;
	recalc_res();

	for (filter_stage &stage : m_stages)
		stage.alpha = G;

	m_stages[0].beta = G2 * G / gp1;
	m_stages[1].beta = G2 / gp1;
	m_stages[2].beta = G / gp1;
	m_stages[3].beta = 1.0F / gp1;
}

DEFINE_DEVICE_TYPE(VA_LPF4, va_lpf4_device, "va_lpf4", "4th order LPF")
