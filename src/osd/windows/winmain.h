// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmain.h - Win32 main program and core headers
//
//============================================================
#ifndef MAME_OSD_WINDOWS_WINMAIN_H
#define MAME_OSD_WINDOWS_WINMAIN_H

#pragma once

#include "winopts.h"

#include "modules/lib/osdobj_common.h"
#include "osdepend.h"

#include <chrono>
#include <vector>


//============================================================
//  TYPE DEFINITIONS
//============================================================

enum input_event
{
	INPUT_EVENT_KEYDOWN,
	INPUT_EVENT_KEYUP,
	INPUT_EVENT_RAWINPUT,
	INPUT_EVENT_ARRIVAL,
	INPUT_EVENT_REMOVAL,
	INPUT_EVENT_MOUSE_BUTTON
};

struct KeyPressEventArgs
{
	input_event event_id;
	uint8_t vkey;
	uint8_t scancode;
};

struct MouseButtonEventArgs
{
	int button;
	int keydown;
	int xpos;
	int ypos;
};


class windows_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	windows_osd_interface(windows_options &options);
	virtual ~windows_osd_interface();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;
	virtual void input_update(bool relative_reset) override;
	virtual void check_osd_inputs() override;

	// input overrideables
	virtual void customize_input_type_list(std::vector<input_type_entry> &typelist) override;

	// video overridables
	virtual void add_audio_to_recording(const int16_t *buffer, int samples_this_frame) override;

	virtual bool video_init() override;
	virtual bool window_init() override;

	virtual void video_exit() override;
	virtual void window_exit() override;

	void extract_video_config();

	// windows OSD specific
	bool handle_input_event(input_event eventid, void *eventdata) const;
	bool should_hide_mouse() const;

	virtual bool has_focus() const override;
	virtual void process_events() override;

	virtual windows_options &options() override { return m_options; }

	int window_count();

	using osd_common_t::poll_input_modules; // Win32 debugger calls this directly, which it shouldn't

protected:
	virtual void build_slider_list() override;
	virtual void update_slider_list() override;

private:
	void process_events(bool ingame, bool nodispatch);
	virtual void osd_exit() override;
	static void output_oslog(const char *buffer);

	windows_options &m_options;
	bool const m_com_status;

	std::chrono::steady_clock::time_point m_last_event_check;

	static inline constexpr int DEFAULT_FONT_HEIGHT = 200;
};


//============================================================
//  GLOBAL VARIABLES
//============================================================

// defined in winwork.c
extern int osd_num_processors;

#endif // MAME_OSD_WINDOWS_WINMAIN_H
