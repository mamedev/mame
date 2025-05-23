// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * sound_module.h
 *
 */
#ifndef MAME_OSD_SOUND_SOUND_MODULE_H
#define MAME_OSD_SOUND_SOUND_MODULE_H

#pragma once

#include <osdepend.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#define OSD_SOUND_PROVIDER   "sound"

class sound_module
{
public:
	virtual ~sound_module();

	virtual uint32_t get_generation() { return 1; }
	virtual osd::audio_info get_information();
	virtual bool external_per_channel_volume() { return false; }
	virtual bool split_streams_per_source() { return false; }

	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) { return 1; }
	virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) { return 0; }
	virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) {}
	virtual void stream_close(uint32_t id) {}
	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) {}
	virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) {}

protected:
	class abuffer {
	public:
		abuffer(uint32_t channels) : m_channels(channels), m_last_sample(channels, 0) {}
		void get(int16_t *data, uint32_t samples);
		void push(const int16_t *data, uint32_t samples);
		uint32_t channels() const { return m_channels; }

	private:
		struct buffer {
			uint32_t m_cpos;
			std::vector<int16_t> m_data;
		};

		uint32_t m_channels;
		std::vector<int16_t> m_last_sample;
		std::vector<buffer> m_buffers;
	};
};

#endif // MAME_OSD_SOUND_SOUND_MODULE_H
