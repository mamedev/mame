//============================================================
//
//  video.c - Win32 video handling
//
//============================================================
//
//  Copyright Aaron Giles
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

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Windows 95/NT4 multimonitor stubs
#ifdef WIN95_MULTIMON
#include "multidef.h"
#endif

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "video/vector.h"
#include "render.h"
#include "rendutil.h"
#include "ui.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "video.h"
#include "window.h"
#include "input.h"
#include "debugwin.h"
#include "strconv.h"
#include "config.h"

//============================================================
//  update
//============================================================

void windows_osd_interface::update(bool skip_redraw)
{
	// ping the watchdog on each update
	winmain_watchdog_ping();

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
	{
		m_video->update();
	}

	// poll the joystick values here
	m_video->process_events(TRUE);
	wininput_poll(machine());
	check_osd_inputs();
}



//============================================================
//  check_osd_inputs
//============================================================

void windows_osd_interface::check_osd_inputs()
{
	// check for toggling fullscreen mode
	if (ui_input_pressed(*m_machine, IPT_OSD_1))
		m_video->window_system()->toggle_full_screen();

	// check for taking fullscreen snap
	if (ui_input_pressed(*m_machine, IPT_OSD_2))
		winwindow_take_snap();

	// check for taking fullscreen video
	if (ui_input_pressed(*m_machine, IPT_OSD_3))
		winwindow_take_video();

	// check for taking fullscreen video
	if (ui_input_pressed(*m_machine, IPT_OSD_4))
		winwindow_toggle_fsfx();
}
