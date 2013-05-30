//============================================================
//
//  window.cpp - Generic window management classes
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

#include "emu.h"
#include "emuopts.h"

#include "render/video.h"
#include "render/window.h"
#include "osdepend.h"

namespace render
{

window_system::window_system(running_machine &machine, video_system *video) :
	m_machine(machine),
	m_video(video)
{
	// determine if we are using multithreading or not
	m_multithreading_enabled = machine.options().multithreading();
}

window_info::~window_info()
{
	//remove_from_list();

	// free the render target
	m_machine.render().target_free(m_target);

	// free the lock
	osd_lock_free(m_render_lock);
}

window_info *window_system::window_alloc(monitor_info *monitor, int index)
{
	return global_alloc_clear(window_info(m_machine, index, m_main_threadid, m_window_threadid, this, monitor));
}


//============================================================
//  window_create
//============================================================

void window_system::window_create(int index, monitor_info *monitor, const window_config *config)
{
	assert(GetCurrentThreadId() == m_main_threadid);

	// allocate a new window object

	window_info *window = window_alloc(monitor, index);
	window->set_maxdims(config->width, config->height);
	window->set_refresh(config->refresh);
	window->set_fullscreen(!m_video->get_video_config().windowed);

	// see if we are safe for fullscreen
	window->set_fullscreen_safe(TRUE);
	for (window_info *win = m_window_list; win != NULL; win = win->next())
		if (win->monitor() == monitor)
			window->set_fullscreen_safe(FALSE);

	// add us to the list
	//*m_last_window_ptr = window;
	//m_last_window_ptr = &window->next();

	// make the window title
	if (m_video->get_video_config().numscreens == 1)
		sprintf(window->title(), "%s: %s [%s]", emulator_info::get_appname(), m_machine.system().description, m_machine.system().name);
	else
		sprintf(window->title(), "%s: %s [%s] - Screen %d", emulator_info::get_appname(), m_machine.system().description, m_machine.system().name, index);

	window->wait_for_ready();
}

window_info::window_info(running_machine &machine, int index, UINT64 main_threadid, UINT64 window_threadid,
	window_system *system, monitor_info *monitor)
	: m_monitor(monitor),
	  m_machine(machine),
	  m_system(system)
{
	// create a lock that we can use to skip blitting
	m_render_lock = osd_lock_alloc();

	m_main_threadid = main_threadid;
	m_window_threadid = window_threadid;

	// load the layout
	m_target = machine.render().target_alloc();

	// remember the current values in case they change
	m_targetview = m_target->view();
	m_targetorient = m_target->orientation();
	m_targetlayerconfig = m_target->layer_config();

	set_starting_view(index, m_machine.options().view(), m_machine.options().view(index));

	m_init_state = 0;

	// set the initial maximized state
	m_startmaximized = m_machine.options().maximize();

	m_multithreading_enabled = m_machine.options().multithreading();
}

render_primitive_list *window_info::get_primitives()
{
	m_hal->update_bounds();
	return &(m_target->get_primitives());
}

void window_info::set_starting_view(int index, const char *defview, const char *view)
{
	// choose non-auto over auto
	if (strcmp(view, "auto") == 0 && strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	int viewindex = m_target->configured_view(view, index, m_system->video()->get_video_config().numscreens);

	// set the view
	m_target->set_view(viewindex);
}


//============================================================
//  update_minmax_state
//  (window thread)
//============================================================

void window_info::update_minmax_state()
{
	assert(GetCurrentThreadId() == m_window_threadid);

	if (!m_fullscreen)
	{
		// compare the maximum bounds versus the current bounds
		math::rectf minbounds, maxbounds, bounds;
		get_min_bounds(&minbounds, m_system->video()->get_video_config().keepaspect);
		get_max_bounds(&maxbounds, m_system->video()->get_video_config().keepaspect);
		get_window_rect(&bounds);

		// if either the width or height matches, we were maximized
		m_isminimized = (bounds.width() == minbounds.width() ||
						bounds.height() == minbounds.height());
		m_ismaximized = (bounds.width() == maxbounds.width() ||
						bounds.height() == maxbounds.height());
	}
	else
	{
		m_isminimized = false;
		m_ismaximized = true;
	}
}

}; // render