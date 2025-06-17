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

	virtual void begin_update() {}
	virtual void end_update() {}

protected:
	class abuffer {
	public:
		abuffer(uint32_t channels) noexcept;
		void get(int16_t *data, uint32_t samples) noexcept;
		void push(const int16_t *data, uint32_t samples);
		void clear() noexcept { m_used_buffers = 0; }
		uint32_t channels() const noexcept { return m_channels; }
		uint32_t available() const noexcept;

	private:
		struct buffer {
			uint32_t m_cpos;
			std::vector<int16_t> m_data;

			buffer() noexcept = default;
			buffer(const buffer &) = default;
			buffer(buffer &&) noexcept = default;
			buffer &operator=(const buffer &) = default;
			buffer &operator=(buffer &&) noexcept = default;
		};

		void pop_buffer() noexcept;
		buffer &push_buffer();

		int32_t m_delta, m_delta2;
		uint32_t m_channels;
		uint32_t m_used_buffers;
		std::vector<int16_t> m_last_sample;
		std::vector<buffer> m_buffers;
	};
};

#endif // MAME_OSD_SOUND_SOUND_MODULE_H
