// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  aviwrite.h - AVI screenshot writer class
//
//============================================================

#pragma once

#ifndef __RENDER_AVIWRITE__
#define __RENDER_AVIWRITE__

#include "emu.h"
#include "aviio.h"

class running_machine;

class avi_write
{
public:
	avi_write(running_machine& machine, uint32_t width, uint32_t height);
	~avi_write();

	void record(std::string name);
	void stop();
	void audio_frame(const INT16 *buffer, int samples_this_frame);
	void video_frame(bitmap_rgb32& snap);

	// Getters
	bool recording() const { return m_recording; }

private:
	void begin_avi_recording(std::string name);
	void end_avi_recording();

	running_machine&		m_machine;

	bool					m_recording;

	uint32_t				m_width;
	uint32_t				m_height;

	avi_file::ptr           m_output_file;

	int                     m_frame;
	attotime                m_frame_period;
	attotime                m_next_frame_time;
};

#endif // __RENDER_AVIWRITE__