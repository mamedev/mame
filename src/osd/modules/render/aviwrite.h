// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  aviwrite.h - AVI screenshot writer class
//
//============================================================

#ifndef MAME_RENDER_AVIWRITE_H
#define MAME_RENDER_AVIWRITE_H

#pragma once

// emu
#include "attotime.h"

// lib/util
#include "aviio.h"

#include <string_view>


class running_machine;

class avi_write
{
public:
	avi_write(running_machine& machine, uint32_t width, uint32_t height);
	~avi_write();

	void record(std::string_view name);
	void stop();
	void audio_frame(const int16_t *buffer, int samples_this_frame);
	void video_frame(bitmap_rgb32& snap);

	// Getters
	bool recording() const { return m_recording; }

private:
	void begin_avi_recording(std::string_view name);
	void end_avi_recording();

	running_machine&        m_machine;

	bool                    m_recording;

	uint32_t                m_width;
	uint32_t                m_height;

	avi_file::ptr           m_output_file;

	int                     m_frame;
	attotime                m_frame_period;
	attotime                m_next_frame_time;
};

#endif // MAME_RENDER_AVIWRITE_H
