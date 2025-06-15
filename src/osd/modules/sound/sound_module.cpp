// license:BSD-3-Clause
// copyright-holders:O. Galibert


#include "sound_module.h"

#include <algorithm>
#include <cassert>
#include <utility>


sound_module::~sound_module()
{
}

osd::audio_info sound_module::get_information()
{
	osd::audio_info result;
	result.m_generation = 1;
	result.m_default_sink = 1;
	result.m_default_source = 0;
	result.m_nodes.resize(1);
	result.m_nodes[0].m_name = "-";
	result.m_nodes[0].m_display_name = "fallthrough";
	result.m_nodes[0].m_id = 1;
	result.m_nodes[0].m_rate.m_default_rate = 0; // Magic value meaning "use configured sample rate"
	result.m_nodes[0].m_rate.m_min_rate = 0;
	result.m_nodes[0].m_rate.m_max_rate = 0;
	result.m_nodes[0].m_sinks = 2;
	result.m_nodes[0].m_sources = 0;
	result.m_nodes[0].m_port_names.emplace_back("L");
	result.m_nodes[0].m_port_names.emplace_back("R");
	result.m_nodes[0].m_port_positions.emplace_back(std::array<double, 3>({ -0.2, 0.0, 1.0 }));
	result.m_nodes[0].m_port_positions.emplace_back(std::array<double, 3>({  0.2, 0.0, 1.0 }));
	result.m_streams.resize(1);
	result.m_streams[0].m_id = 1;
	result.m_streams[0].m_node = 1;
	return result;
}

sound_module::abuffer::abuffer(uint32_t channels) noexcept : m_channels(channels), m_used_buffers(0), m_last_sample(channels, 0)
{
}

void sound_module::abuffer::get(int16_t *data, uint32_t samples) noexcept
{
	uint32_t pos = 0;
	while(pos != samples) {
		if(!m_used_buffers) {
			while(pos != samples) {
				std::copy_n(m_last_sample.data(), m_channels, data);
				data += m_channels;
				pos++;
			}
			break;
		}

		auto &buf = m_buffers.front();
		if(buf.m_data.empty()) {
			pop_buffer();
			continue;
		}

		uint32_t avail = (buf.m_data.size() / m_channels) - buf.m_cpos;
		if(avail > (samples - pos)) {
			avail = samples - pos;
			std::copy_n(buf.m_data.data() + (buf.m_cpos * m_channels), avail * m_channels, data);
			buf.m_cpos += avail;
			break;
		}

		std::copy_n(buf.m_data.data() + (buf.m_cpos * m_channels), avail * m_channels, data);
		pop_buffer();
		pos += avail;
		data += avail * m_channels;
	}
}

void sound_module::abuffer::push(const int16_t *data, uint32_t samples)
{
	auto &buf = push_buffer();
	buf.m_cpos = 0;
	buf.m_data.resize(samples * m_channels);
	std::copy_n(data, samples * m_channels, buf.m_data.data());
	std::copy_n(data + ((samples - 1) * m_channels), m_channels, m_last_sample.data());

	if(m_used_buffers > 10) {
		// If there are way too many buffers, drop some so only 10 are left (roughly 0.2s)
		for(unsigned i = 0; 10 > i; ++i) {
			using std::swap;
			swap(m_buffers[i], m_buffers[m_used_buffers + i - 10]);
		}
		m_used_buffers = 10;
	} else if(m_used_buffers >= 5) {
		// If there are too many buffers, remove five samples per buffer
		// to slowly resync to reduce latency (4 seconds to
		// compensate one buffer, roughly)
		buf.m_cpos = std::max<uint32_t>(samples / 200, 1);
	}
}

uint32_t sound_module::abuffer::available() const noexcept
{
	uint32_t result = 0;
	for(uint32_t i = 0; m_used_buffers > i; ++i)
		result += (m_buffers[i].m_data.size() / m_channels) - m_buffers[i].m_cpos;
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
