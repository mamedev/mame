//============================================================
//
//  drawnone.c - stub "nothing" drawer
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

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "window.h"



//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawnone_exit(void);
static int drawnone_window_init(win_window_info *window);
static void drawnone_window_destroy(win_window_info *window);
static render_primitive_list *drawnone_window_get_primitives(win_window_info *window);
static int drawnone_window_draw(win_window_info *window, HDC dc, int update);



//============================================================
//  drawnone_init
//============================================================

int drawnone_init(running_machine &machine, win_draw_callbacks *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawnone_exit;
	callbacks->window_init = drawnone_window_init;
	callbacks->window_get_primitives = drawnone_window_get_primitives;
	callbacks->window_draw = drawnone_window_draw;
	callbacks->window_save = NULL;
	callbacks->window_record = NULL;
	callbacks->window_toggle_fsfx = NULL;
	callbacks->window_destroy = drawnone_window_destroy;
	return 0;
}



//============================================================
//  drawnone_exit
//============================================================

static void drawnone_exit(void)
{
}



//============================================================
//  drawnone_window_init
//============================================================

static int drawnone_window_init(win_window_info *window)
{
	return 0;
}



//============================================================
//  drawnone_window_destroy
//============================================================

static void drawnone_window_destroy(win_window_info *window)
{
}



//============================================================
//  drawnone_window_get_primitives
//============================================================

static render_primitive_list *drawnone_window_get_primitives(win_window_info *window)
{
	RECT client;
	GetClientRect(window->hwnd, &client);
	window->target->set_bounds(rect_width(&client), rect_height(&client), winvideo_monitor_get_aspect(window->monitor));
	return &window->target->get_primitives();
}



//============================================================
//  drawnone_window_draw
//============================================================

static int drawnone_window_draw(win_window_info *window, HDC dc, int update)
{
	return 0;
}
