// license:BSD-3-Clause
// copyright-holders:O. Galibert

#include "sound_module.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <numeric>
#include <utility>


sound_module::~sound_module()
{
	// implementing this here forces the vtable and inline virtual member functions to be instantiated
}

sound_module::abuffer::abuffer(uint32_t channels, uint32_t rate) noexcept :
	m_channels(channels),
	m_rate(rate),
	m_used_buffers(0),
	m_max_buffers(8),
	m_hindex(0),
	m_last_sample(channels, 0)
{
	clear();
}

void sound_module::abuffer::set_latency(float latency)
{
	// set maximum buffers from latency in 20ms steps (the default of 8 is 0.16s)
	m_max_buffers = std::max<uint32_t>(latency + latency / 2.0f, latency + 3);
	m_max_buffers = std::clamp(m_max_buffers, 4U, 50U);
}

void sound_module::abuffer::clear()
{
	get(m_last_sample.data(), 1);
	m_used_buffers = 0;
	m_used_buffers_prev = 0;
	m_overrun = false;
	m_history.fill(0);

	m_delta = 0;
	m_delta2 = 0;
	m_underruns = 0;
	m_overruns = 0;
}

void sound_module::abuffer::get(int16_t *data, uint32_t samples) noexcept
{
	m_delta -= samples;
	m_delta2 -= samples;
	uint32_t pos = 0;
	while(pos != samples) {
		// on underrun, fill buffer with last sample to prevent audible pop
		if(!m_used_buffers) {
			m_delta2 += samples - pos;
			m_underruns++;
			while(pos != samples) {
				std::memmove(data, m_last_sample.data(), m_channels * sizeof(int16_t));
				data += m_channels;
				pos++;
			}
			break;
		}

		auto &buf = m_buffers.front();
		if(buf.data.empty()) {
			pop_buffer();
			continue;
		}

		uint32_t avail = (buf.data.size() / m_channels) - buf.cpos;
		if(avail > (samples - pos)) {
			avail = samples - pos;
			std::copy_n(buf.data.data() + (buf.cpos * m_channels), avail * m_channels, data);
			buf.cpos += avail;
			break;
		}

		std::copy_n(buf.data.data() + (buf.cpos * m_channels), avail * m_channels, data);
		pop_buffer();
		pos += avail;
		data += avail * m_channels;
	}
	m_internal_get = false;
	//printf("%d -%d +%d # %d %d\n", m_used_buffers, m_underruns, m_overruns, m_delta, m_delta2);
}

void sound_module::abuffer::flush_buffers(uint32_t remain)
{
	assert(remain);

	for(uint32_t i = 0; i != m_used_buffers - remain; i++)
		m_delta2 -= (m_buffers[i].data.size() / m_channels - m_buffers[i].cpos);

	// number of samples to crossfade (eg. 128 when samplerate is 48000)
	const uint32_t samples = std::max(m_rate / 375, 2U);

	// get crossfade source chunk
	if(!m_internal_get || m_last_fade.size() != samples * m_channels) {
		m_last_fade.resize(samples * m_channels);
		get(m_last_fade.data(), samples);
		m_internal_get = true;
		m_delta += samples;
		m_delta2 += samples;
	}

	// flush buffers until [remain] are left
	if(m_used_buffers > remain) {
		for(uint32_t i = 0; i < remain; i++) {
			using std::swap;
			swap(m_buffers[i], m_buffers[m_used_buffers + i - remain]);
		}
		m_used_buffers = remain;
	}

	if(!m_used_buffers || samples > (m_buffers[0].data.size() / m_channels - m_buffers[0].cpos))
		return;
	int16_t *dest = &m_buffers[0].data[m_channels * m_buffers[0].cpos];

	// crossfade into front of buffer to reduce clicks
	for(uint32_t i = 0; i < samples; i++) {
		int32_t gain_b = (i << 15) / (samples - 1);
		int32_t gain_a = 32768 - gain_b;

		for(uint32_t ch = 0; ch < m_channels; ch++) {
			uint32_t j = i * m_channels + ch;

			int32_t mix = (m_last_fade[j] * gain_a + dest[j] * gain_b) >> 15;
			dest[j] = static_cast<int16_t>(mix);
		}
	}
}

void sound_module::abuffer::push(const int16_t *data, uint32_t samples)
{
	m_delta += samples;
	m_delta2 += samples;
	auto &buf = push_buffer();
	buf.cpos = 0;
	buf.data.resize(samples * m_channels);
	std::copy_n(data, samples * m_channels, buf.data.data());
	std::copy_n(data + ((samples - 1) * m_channels), m_channels, m_last_sample.data());

	// maximum number of buffers relative to samples
	// unless -speed or -refreshspeed is used, this is same as m_max_buffers
	const uint32_t max_buffers = std::max(m_max_buffers * m_rate / samples / 50, 4U);

	// minimum number of buffers after overrun
	// lower limit of 2 prevents buffer underruns with push(this), get, get, push
	const uint32_t min_buffers = std::max(max_buffers / 3, 2U);

	m_history[m_hindex] = m_used_buffers_prev - m_used_buffers;
	m_hindex = (m_hindex + 1) % m_history.size();

	if(m_overrun && std::accumulate(m_history.begin(), m_history.end(), 0) >= -2) {
		if(m_used_buffers > min_buffers) {
			// once it's stabilized after an overrun, reduce buffers to minimum latency
			flush_buffers(min_buffers);
			m_history.fill(0);
		}
		m_overrun = false;
	} else if(m_used_buffers > max_buffers) {
		// if there are too many buffers, drop some and mark this event as an overrun
		flush_buffers(max_buffers);
		m_overrun = true;
		m_overruns++;
	}

	m_used_buffers_prev = m_used_buffers;
	//printf("%d -%d +%d # %d %d\n", m_used_buffers, m_underruns, m_overruns, m_delta, m_delta2);
}

uint32_t sound_module::abuffer::available() const noexcept
{
	uint32_t result = 0;
	for(uint32_t i = 0; m_used_buffers > i; ++i)
		result += (m_buffers[i].data.size() / m_channels) - m_buffers[i].cpos;
	return result;
}

inline void sound_module::abuffer::pop_buffer() noexcept
{
	assert(m_used_buffers);
	if(--m_used_buffers) {
		auto temp(std::move(m_buffers.front()));
		for(uint32_t i = 0; m_used_buffers > i; ++i)
			m_buffers[i] = std::move(m_buffers[i + 1]);
		m_buffers[m_used_buffers] = std::move(temp);
	}
}

inline sound_module::abuffer::buffer &sound_module::abuffer::push_buffer()
{
	if(m_buffers.size() > m_used_buffers) {
		return m_buffers[m_used_buffers++];
	} else {
		assert(m_buffers.size() == m_used_buffers);
		++m_used_buffers;
		return m_buffers.emplace_back();
	}
}
