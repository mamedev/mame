// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  aviwrite.cpp - AVI screenshot writer class
//
//============================================================

#include "emu.h"
#include "aviwrite.h"

#include "modules/lib/osdobj_common.h"
#include "fileio.h"
#include "screen.h"


avi_write::avi_write(running_machine& machine, uint32_t width, uint32_t height)
	: m_machine(machine)
	, m_recording(false)
	, m_width(width)
	, m_height(height)
	, m_output_file(nullptr)
	, m_frame(0)
{
}

avi_write::~avi_write()
{
	if (m_recording)
	{
		stop();
	}
}

void avi_write::record(std::string_view name)
{
	if (m_recording)
	{
		end_avi_recording();
	}

	begin_avi_recording(name);
}

void avi_write::stop()
{
	osd_printf_info("Stopping AVI recording after %d frames.\n", m_frame);
	end_avi_recording();
}

void avi_write::begin_avi_recording(std::string_view name)
{
	// stop any existing recording
	end_avi_recording();

	// reset the state
	m_frame = 0;
	m_next_frame_time = m_machine.time();

	const screen_device *primary_screen = screen_device_enumerator(m_machine.root_device()).first();
	// build up information about this new movie
	avi_file::movie_info info;
	info.video_format = 0;
	info.video_timescale = 1000 * (primary_screen ? ATTOSECONDS_TO_HZ(primary_screen->frame_period().m_attoseconds) : screen_device::DEFAULT_FRAME_RATE);
	info.video_sampletime = 1000;
	info.video_numsamples = 0;
	info.video_width = m_width;
	info.video_height = m_height;
	info.video_depth = 24;

	info.audio_format = 0;
	info.audio_timescale = m_machine.sample_rate();
	info.audio_sampletime = 1;
	info.audio_numsamples = 0;
	info.audio_channels = m_machine.sound().outputs_count();
	info.audio_samplebits = 16;
	info.audio_samplerate = m_machine.sample_rate();

	// compute the frame time
	m_frame_period = attotime::from_seconds(1000) / info.video_timescale;

	// create a new temporary movie file
	emu_file tempfile(m_machine.options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	const std::error_condition filerr = (name.empty() || name == OSDOPTVAL_AUTO)
			? m_machine.video().open_next(tempfile, "avi")
			: tempfile.open(name);

	// if we succeeded, make a copy of the name and create the real file over top
	if (!filerr)
	{
		const std::string fullpath = tempfile.fullpath();
		tempfile.close();

		// create the file and free the string
		avi_file::error avierr = avi_file::create(fullpath, info, m_output_file);
		if (avierr != avi_file::error::NONE)
		{
			osd_printf_error("Error creating AVI: %s\n", avi_file::error_string(avierr));
		}
		else
		{
			m_recording = true;
		}
	}
}

void avi_write::end_avi_recording()
{
	m_recording = false;
	m_output_file.reset();
	m_frame = 0;
}

void avi_write::video_frame(bitmap_rgb32& snap)
{
	// get the current time
	attotime curtime = m_machine.time();

	// loop until we hit the right time
	while (m_next_frame_time <= curtime)
	{
		// handle an AVI recording
		// write the next frame
		avi_file::error avierr = m_output_file->append_video_frame(snap);
		if (avierr != avi_file::error::NONE)
		{
			osd_printf_error("Error while logging AVI video frame: %s\n", avi_file::error_string(avierr));
			osd_printf_error("Stopping AVI recording.\n");
			end_avi_recording();
			return;
		}

		// advance time
		m_next_frame_time += m_frame_period;
		m_frame++;
	}
}

void avi_write::audio_frame(const int16_t *buffer, int samples_this_frame)
{
	// only record if we have a file
	if (m_output_file != nullptr)
	{
		// write the next frame
		int channels = m_machine.sound().outputs_count();
		avi_file::error avierr = avi_file::error::NONE;
		for (int channel = 0; channel != channels && avierr == avi_file::error::NONE; channel ++)
			avierr = m_output_file->append_sound_samples(channel, buffer + channel, samples_this_frame, channels-1);
		if (avierr != avi_file::error::NONE)
		{
			osd_printf_error("Error while logging AVI audio frame: %s\n", avi_file::error_string(avierr));
			osd_printf_error("Stopping AVI recording.\n");
			end_avi_recording();
		}
	}
}
