// license:BSD-3-Clause
// copyright-holders:O. Galibert


#include "emu.h"
#include "sound_module.h"

void sound_module::abuffer::get(int16_t *data, uint32_t samples)
{
	uint32_t pos = 0;
	while(pos != samples) {
		if(m_buffers.empty()) {
			while(pos != samples) {
				memcpy(data, m_last_sample.data(), m_channels*2);
				data += m_channels;
				pos ++;				
			}
			break;
		}

		auto &buf = m_buffers.front();
		if(buf.m_data.empty()) {
			m_buffers.erase(m_buffers.begin());
			continue;
		}

		uint32_t avail = buf.m_data.size() / m_channels - buf.m_cpos;
		if(avail > samples - pos) {
			avail = samples - pos;
			memcpy(data, buf.m_data.data() + buf.m_cpos * m_channels, avail * 2 * m_channels);
			buf.m_cpos += avail;
			break;
		}

		memcpy(data, buf.m_data.data() + buf.m_cpos * m_channels, avail * 2 * m_channels);
		m_buffers.erase(m_buffers.begin());
		pos += avail;
		data += avail * m_channels;
	}			
}

void sound_module::abuffer::push(const int16_t *data, uint32_t samples)
{
	m_buffers.resize(m_buffers.size() + 1);
	auto &buf = m_buffers.back();
	buf.m_cpos = 0;
	buf.m_data.resize(samples * m_channels);
	memcpy(buf.m_data.data(), data, samples * 2 * m_channels);
	memcpy(m_last_sample.data(), data + (samples-1) * m_channels, 2 * m_channels);

	if(m_buffers.size() > 10)
		// If there are way too many buffers, drop some so only 10 are left (roughly 0.2s)
		m_buffers.erase(m_buffers.begin(), m_buffers.begin() + m_buffers.size() - 10);

	else if(m_buffers.size() >= 5)
		// If there are too many buffers, remove five samples per buffer
		// to slowly resync to reduce latency (4 seconds to
		// compensate one buffer, roughly)
		buf.m_cpos = 5;
}

