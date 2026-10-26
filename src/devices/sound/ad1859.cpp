// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Analog Devices AD1859 16/18-bit stereo DAC

    The chip's interpolation filters are unpublished, so a windowed sinc stands in for them.
*/

#include "emu.h"
#include "ad1859.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

DEFINE_DEVICE_TYPE(AD1859, ad1859_device, "ad1859", "Analog Devices AD1859 stereo DAC")

const float *ad1859_device::filter()
{
	static const std::array<float, FILTER_TAPS * (FILTER_PHASES + 1)> table = [] ()
	{
		std::array<float, FILTER_TAPS * (FILTER_PHASES + 1)> result{};
		for (u32 phase = 0; phase <= FILTER_PHASES; phase++)
		{
			for (u32 tap = 0; tap < FILTER_TAPS; tap++)
			{
				const double x = double(tap) + double(phase) / FILTER_PHASES - FILTER_TAPS / 2;
				const double a = 2 * std::numbers::pi * x / FILTER_TAPS;
				const double window = 0.42 + 0.5 * std::cos(a) + 0.08 * std::cos(2 * a);
				result[phase * FILTER_TAPS + tap] = window * ((x == 0) ? 1.0 : std::sin(std::numbers::pi * x) / (std::numbers::pi * x));
			}
		}
		return result;
	}();

	return table.data();
}

ad1859_device::ad1859_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AD1859, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_format(RIGHT_JUSTIFIED)
	, m_word_length(16)
	, m_sdata(false)
	, m_bclk(false)
	, m_lrclk(false)
	, m_cdata(false)
	, m_cclk(false)
	, m_clatch(false)
	, m_mute(false)
	, m_pd_rst(true)
	, m_shift(0)
	, m_bits(0)
	, m_channel(0)
	, m_sync(false)
	, m_skip(false)
	, m_receiving(false)
	, m_control_shift(0)
	, m_control_latch(0)
	, m_control_pending(false)
	, m_control{}
	, m_gain_pending{}
	, m_gain_samples{}
	, m_gain{}
	, m_previous{}
	, m_history{}
	, m_history_pos{}
	, m_sample_period{}
{
}

void ad1859_device::device_validity_check(validity_checker &valid) const
{
	if (m_word_length != 16 && m_word_length != 18)
	{
		osd_printf_error("Word length must be 16 or 18 bits\n");
	}
}

void ad1859_device::device_start()
{
	m_stream = stream_alloc(0, 2, OUTPUT_RATE);

	save_item(NAME(m_sdata));
	save_item(NAME(m_bclk));
	save_item(NAME(m_lrclk));
	save_item(NAME(m_cdata));
	save_item(NAME(m_cclk));
	save_item(NAME(m_clatch));
	save_item(NAME(m_mute));
	save_item(NAME(m_pd_rst));
	save_item(NAME(m_shift));
	save_item(NAME(m_bits));
	save_item(NAME(m_channel));
	save_item(NAME(m_sync));
	save_item(NAME(m_skip));
	save_item(NAME(m_receiving));
	save_item(NAME(m_control_shift));
	save_item(NAME(m_control_latch));
	save_item(NAME(m_control_pending));
	save_item(NAME(m_control));
	save_item(NAME(m_gain_pending));
	save_item(NAME(m_gain_samples));
	save_item(NAME(m_gain));
	save_item(NAME(m_previous));
	save_item(NAME(m_history));
	save_item(NAME(m_history_pos));
	save_item(NAME(m_sample_time));
	save_item(NAME(m_sample_period));
}

void ad1859_device::device_reset()
{
	m_stream->update();
	reset_registers();
}

void ad1859_device::reset_registers()
{
	m_shift = 0;
	m_bits = 0;
	m_channel = 0;
	m_sync = m_skip = m_receiving = false;
	m_control_shift = m_control_latch = 0;
	m_control_pending = false;

	for (u32 ch = 0; ch < 2; ch++)
	{
		m_control[ch] = 0;
		m_gain_pending[ch] = false;
		m_gain_samples[ch] = 0;
		m_gain[ch] = target_gain(ch);
		m_previous[ch] = 0;
		std::fill(std::begin(m_history[ch]), std::end(m_history[ch]), 0);
		m_history_pos[ch] = 0;
		m_sample_time[ch] = attotime::never;
		m_sample_period[ch] = 0;
	}
}

void ad1859_device::pd_rst_w(int state)
{
	if (m_pd_rst != bool(state))
	{
		m_stream->update();
		m_pd_rst = bool(state);
		if (m_pd_rst)
		{
			reset_registers();
		}
	}
}

void ad1859_device::mute_w(int state)
{
	if (m_mute != bool(state))
	{
		m_stream->update();
		m_mute = bool(state);
		for (u32 ch = 0; ch < 2; ch++)
		{
			update_gain(ch);
		}
	}
}

void ad1859_device::cclk_w(int state)
{
	if (!m_cclk && state && m_pd_rst)
	{
		m_control_shift = (m_control_shift << 1) | u8(m_cdata);
	}
	m_cclk = bool(state);
}

void ad1859_device::clatch_w(int state)
{
	if (!m_clatch && state && m_pd_rst)
	{
		m_control_latch = m_control_shift;
		m_control_pending = true;
	}
	m_clatch = bool(state);
}

void ad1859_device::lrclk_w(int state)
{
	if (m_lrclk == bool(state))
	{
		return;
	}

	if (m_pd_rst)
	{
		if (m_control_pending)
		{
			m_stream->update();
			const u32 ch = BIT(m_control_latch, 7);
			m_control[ch] = m_control_latch & 0x7f;
			m_control_pending = false;
			update_gain(ch);
		}

		if (m_format == DSP)
		{
			m_sync = bool(state);
		}
		else
		{
			if (m_format == RIGHT_JUSTIFIED && m_receiving && m_bits >= m_word_length)
			{
				sample_w(m_channel, m_shift);
			}
			m_channel = (m_format == I2S) ? bool(state) : !state;
			m_shift = 0;
			m_bits = 0;
			m_receiving = true;
			m_skip = (m_format == I2S);
		}
	}
	m_lrclk = bool(state);
}

void ad1859_device::bclk_w(int state)
{
	const bool edge = (m_bclk != bool(state)) && (m_format == DSP ? !state : bool(state));
	m_bclk = bool(state);
	if (!edge || !m_pd_rst)
	{
		return;
	}

	if (m_format == DSP && m_sync)
	{
		m_sync = false;
		m_receiving = true;
		m_shift = 0;
		m_bits = 0;
		return; // frame sync comes one BCLK before the MSB
	}

	if (!m_receiving)
	{
		return;
	}

	if (m_skip)
	{
		m_skip = false;
		return;
	}

	m_shift = (m_shift << 1) | u32(m_sdata);
	if (m_bits < 32)
	{
		m_bits++;
	}

	if (m_format != RIGHT_JUSTIFIED && m_bits == m_word_length)
	{
		sample_w(m_channel, m_shift);
		m_receiving = false;
		if (m_format == DSP)
		{
			m_channel ^= 1;
		}
	}
}

void ad1859_device::sample_w(u32 channel, u32 data)
{
	m_stream->update();

	const attotime now = machine().time();
	if (m_sample_time[channel] != attotime::never && now > m_sample_time[channel])
	{
		m_sample_period[channel] = (now - m_sample_time[channel]).as_double();
	}
	m_sample_time[channel] = now;

	// the history is stored twice so the filter can read it without wrapping
	m_history_pos[channel] = (m_history_pos[channel] + FILTER_TAPS - 1) % FILTER_TAPS;
	const float sample = double(util::sext(data, m_word_length)) / (1U << (m_word_length - 1));
	m_history[channel][m_history_pos[channel]] = sample;
	m_history[channel][m_history_pos[channel] + FILTER_TAPS] = sample;

	if (m_gain_pending[channel] && m_gain_samples[channel] < 1024)
	{
		m_gain_samples[channel]++;
	}
}

float ad1859_device::target_gain(u32 channel) const
{
	return std::pow(10.0, (m_mute || BIT(m_control[channel], 6)) ? -74.2 / 20 : -double(m_control[channel] & 63) / 20);
}

void ad1859_device::update_gain(u32 channel)
{
	m_gain_pending[channel] = (m_gain[channel] != target_gain(channel));
	m_gain_samples[channel] = 0;
}

void ad1859_device::sound_stream_update(sound_stream &stream)
{
	const float *const table = filter();

	for (u32 ch = 0; ch < 2; ch++)
	{
		if (!m_pd_rst || m_sample_period[ch] <= 0)
		{
			stream.fill(ch, 0);
			continue;
		}

		double age = (stream.start_time() - m_sample_time[ch]).as_double() / m_sample_period[ch];
		const double step = 1.0 / (OUTPUT_RATE * m_sample_period[ch]);
		for (int i = 0; i < stream.samples(); i++, age += step)
		{
			float sample = 0;
			if (age >= 0 && age < FILTER_TAPS)
			{
				const u32 offset = u32(age);
				const double phase = (age - offset) * FILTER_PHASES;
				const u32 index = u32(phase);
				const float fraction = phase - index;
				const float *const h0 = &table[index * FILTER_TAPS + offset];
				const float *const h1 = h0 + FILTER_TAPS;
				const float *const history = &m_history[ch][m_history_pos[ch]];
				for (u32 tap = 0; tap < FILTER_TAPS - offset; tap++)
				{
					const float h = h0[tap] + fraction * (h1[tap] - h0[tap]);
					sample += history[tap] * h;
				}
			}

			// gain changes wait for a zero crossing, or 1024 samples at most
			if (m_gain_pending[ch] && (sample == 0 || (sample < 0) != (m_previous[ch] < 0) || m_gain_samples[ch] >= 1024))
			{
				m_gain[ch] = target_gain(ch);
				m_gain_pending[ch] = false;
			}
			m_previous[ch] = sample;
			stream.put(ch, i, sample * m_gain[ch]);
		}
	}
}
