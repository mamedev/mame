//============================================================
//
//  drawhal.cpp - Generic render abstraction layer
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

#include "drawhal.h"
#include "osdepend.h"
#include "winutf8.h"

namespace render
{

window_info::~window_info()
{
	remove_from_list();

	// free the render target
	window->machine().render().target_free(window->target);

	// free the lock
	osd_lock_free(m_render_lock);
}

window_info::window_info(running_machine &machine, threadid main_threadid, threadid window_threadid)
	: m_machine(machine)
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

	set_starting_view(index, m_machine.options.view(), m_machine.options.view(index));

	m_init_state = 0;

	m_multithreading_enabled = m_machine.options.multithreading();
}

render_primitive_list &window_info::get_primitives()
{
	m_hal->update_bounds();
	return window->target->get_primitives();
}

void window_info::set_starting_view(int index, const char *defview, const char *view)
{
	// choose non-auto over auto
	if (strcmp(view, "auto") == 0 && strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	int viewindex = m_target->configured_view(view, index, video_config.numscreens);

	// set the view
	m_target->set_view(viewindex);
}

int draw_hal::create_resources()
{
	return 0;
}

int draw_hal::delete_resources()
{
	return 0;
}

int	draw_hal::set_view_size(vec2f& size)
{
}

void draw_hal::begin_frame()
{
}

void draw_hal::end_frame()
{
}

void draw_hal::process_primitives()
{
}

void draw_hal::draw_primitives()
{
}

}; // namespace render
