// license:BSD-3-Clause
// copyright-holders:O. Galibert


#include "emu.h"
#include "sound_module.h"

#include <algorithm>


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
	result.m_nodes[0].m_name = "";
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

void sound_module::abuffer::get(int16_t *data, uint32_t samples)
{
	uint32_t pos = 0;
	while(pos != samples) {
		if(m_buffers.empty()) {
			while(pos != samples) {
				std::copy_n(m_last_sample.data(), m_channels, data);
				data += m_channels;
				pos++;
			}
			break;
		}

		auto &buf = m_buffers.front();
		if(buf.m_data.empty()) {
			m_buffers.erase(m_buffers.begin());
			continue;
		}

		uint32_t avail = (buf.m_data.size() / m_channels) - buf.m_cpos;
		if(avail > samples - pos) {
			avail = samples - pos;
			std::copy_n(buf.m_data.data() + (buf.m_cpos * m_channels), avail * m_channels, data);
			buf.m_cpos += avail;
			break;
		}

		std::copy_n(buf.m_data.data() + (buf.m_cpos * m_channels), avail * m_channels, data);
		m_buffers.erase(m_buffers.begin());
		pos += avail;
		data += avail * m_channels;
	}			
}

void sound_module::abuffer::push(const int16_t *data, uint32_t samples)
{
	auto &buf = m_buffers.emplace_back();
	buf.m_cpos = 0;
	buf.m_data.resize(samples * m_channels);
	std::copy_n(data, samples * m_channels, buf.m_data.data());
	std::copy_n(data + ((samples - 1) * m_channels), m_channels, m_last_sample.data());

	if(m_buffers.size() > 10) {
		// If there are way too many buffers, drop some so only 10 are left (roughly 0.2s)
		m_buffers.erase(m_buffers.begin(), m_buffers.begin() + m_buffers.size() - 10);
	} else if(m_buffers.size() >= 5) {
		// If there are too many buffers, remove five samples per buffer
		// to slowly resync to reduce latency (4 seconds to
		// compensate one buffer, roughly)
		buf.m_cpos = 5;
	}
}
