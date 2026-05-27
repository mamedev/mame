// license:BSD-3-Clause
// copyright-holders:O. Galibert

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

	virtual uint32_t get_generation() = 0;
	virtual osd::audio_info get_information() = 0;
	virtual bool external_per_channel_volume() { return false; }
	virtual bool split_streams_per_source() { return false; }

	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) = 0;
	virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) { return 0; }
	virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) {}
	virtual void stream_close(uint32_t id) = 0;
	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) = 0;
	virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) {}

	virtual void begin_update() {}
	virtual void end_update() {}

protected:
	class abuffer {
	public:
		abuffer(uint32_t channels, uint32_t rate) noexcept;
		void set_latency(float latency);
		void clear();
		void get(int16_t *data, uint32_t samples) noexcept;
		void push(const int16_t *data, uint32_t samples);
		uint32_t channels() const noexcept { return m_channels; }
		uint32_t available() const noexcept;

	private:
		struct buffer {
			uint32_t cpos;
			std::vector<int16_t> data;

			buffer() noexcept = default;
			buffer(const buffer &) = default;
			buffer(buffer &&) noexcept = default;
			buffer &operator=(const buffer &) = default;
			buffer &operator=(buffer &&) noexcept = default;
		};

		void pop_buffer() noexcept;
		void flush_buffers(uint32_t remain);
		buffer &push_buffer();

		uint32_t m_channels;
		uint32_t m_rate;
		uint32_t m_used_buffers;
		uint32_t m_used_buffers_prev;
		uint32_t m_max_buffers;
		std::array<int32_t, 8> m_history;
		uint8_t m_hindex;
		bool m_overrun;
		bool m_internal_get;
		std::vector<int16_t> m_last_fade;
		std::vector<int16_t> m_last_sample;
		std::vector<buffer> m_buffers;

		// statistics
		int32_t m_delta, m_delta2;
		uint32_t m_underruns, m_overruns;
	};
};

#endif // MAME_OSD_SOUND_SOUND_MODULE_H
