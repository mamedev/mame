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

#include "render/video.h"

namespace render
{

video_system::video_system(running_machine &machine) :
	m_machine(machine)
{
	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(exit), &machine));

	// extract data from the options
	extract_video_config();

	// set up monitors first
	init_monitors();

	// initialize the window system so we can make windows
	m_window = global_alloc_clear(window_system(machine));

	// create the windows
	for (int index = 0; index < m_video_config.numscreens; index++)
	{
		m_window->window_create(index, pick_monitor(index), &m_video_config.window[index]);
	}

	// possibly create the debug window, but don't show it yet
	if (m_machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		debugwin_init_windows(m_machine);

	// set up the window list
	m_last_window_ptr = &m_window_list;
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
	float aspect = get_aspect(m_machine.options().aspect(), m_machine.options().aspect(index), TRUE);

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
				goto finishit;
			}

			int match = strcmp(scrname, monitor_name);
			osd_free(monitor_name);
			if (match == 0)
				goto finishit;
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

	if (aspect != 0)
	{
		monitor->set_aspect(aspect);
	}
	return monitor;
}

};