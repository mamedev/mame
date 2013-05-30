//============================================================
//
//  video.c - Generic video system
//
//============================================================
//
//  Copyright Nicola Salmoria and the MAME Team.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

#include "emu/emu.h"
#include "emu/emuopts.h"
#include "osd/osdepend.h"
#include "emu/video/vector.h"
#include "emu/render.h"
#include "emu/rendutil.h"
#include "emu/ui.h"
#include "emu/uiinput.h"

#include "render/video.h"

namespace render
{

video_system::video_system(running_machine &machine) :
	m_machine(machine)
{
	// ensure we get called on the way out
	m_machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(video_system::exit), this));

	// extract data from the options
	extract_video_config();

	// set up monitors first
	init_monitors();

	// initialize the window system so we can make windows
	m_window = global_alloc_clear(window_system(machine, this));

	// create the windows
	for (int index = 0; index < m_video_config.numscreens; index++)
	{
		m_window->window_create(index, pick_monitor(index), &m_video_config.window[index]);
	}

	// set up the window list
	m_last_window_ptr = m_window->window_list_ptr();
}

void video_system::extract_video_config()
{
	m_video_config.windowed     = m_machine.options().window();
	m_video_config.keepaspect   = m_machine.options().keep_aspect();
	m_video_config.numscreens	= m_machine.options().numscreens();

	if (m_machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
	{
		m_video_config.windowed = true;
	}
}

void video_system::exit()
{
}

//============================================================
//  get_aspect
//============================================================

float video_system::get_aspect(const char *defdata, const char *data, int report_error)
{
	int num = 0, den = 1;

	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		mame_printf_error("Illegal aspect ratio value = %s\n", data);
	return (float)num / (float)den;
}



//============================================================
//  get_resolution
//============================================================

void video_system::get_resolution(const char *defdata, const char *data, window_config *config, int report_error)
{
	config->width = config->height = config->refresh = 0;
	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return;
		data = defdata;
	}
	if (sscanf(data, "%dx%d@%d", &config->width, &config->height, &config->refresh) < 2 && report_error)
		mame_printf_error("Illegal resolution value = %s\n", data);
}


//============================================================
//  last_monitor
//============================================================

void video_system::set_last_monitor(monitor_info *last_monitor)
{
	monitor_info **tailptr = (monitor_info **)&m_monitor_list;
	monitor_info ***tailptrptr = (monitor_info ***)&tailptr;
	**tailptrptr = last_monitor;
	*tailptrptr = last_monitor->next_ptr();
}


//============================================================
//  pick_monitor
//============================================================

monitor_info *video_system::pick_monitor(int index)
{
	// get the screen option
	const char *scrname = m_machine.options().screen();
	const char *scrname2 = m_machine.options().screen(index);

	// decide which one we want to use
	if (strcmp(scrname2, "auto") != 0)
		scrname = scrname2;

	// get the aspect ratio
	float aspect = get_aspect(m_machine.options().aspect(), m_machine.options().aspect(index), true);

	// look for a match in the name first
	monitor_info *monitor;
	int moncount = 0;
	if (scrname != NULL)
	{
		for (monitor = m_monitor_list; monitor != NULL; monitor = monitor->next())
		{
			moncount++;

			char *monitor_name = monitor->device_name();
			if (monitor_name == NULL)
			{
				if (aspect != 0.0f)
					monitor->set_aspect(aspect);
				return monitor;
			}

			int match = strcmp(scrname, monitor_name);
			osd_free(monitor_name);
			if (match == 0)
			{
				if (aspect != 0.0f)
					monitor->set_aspect(aspect);
				return monitor;
			}
		}
	}

	// didn't find it; alternate monitors until we hit the jackpot
	index %= moncount;
	bool found = false;
	for (monitor = m_monitor_list; monitor != NULL; monitor = monitor->next())
	{
		if (index == 0)
		{
			found = true;
			break;
		}
		index--;
	}

	// return the primary just in case all else fails
	if (!found)
	{
		monitor = m_primary_monitor;
	}

	if (aspect != 0.0f)
	{
		monitor->set_aspect(aspect);
	}
	return monitor;
}

};