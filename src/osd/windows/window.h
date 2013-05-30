//============================================================
//
//  window.h - Win32 window handling
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

#ifndef __WIN_WINDOW__
#define __WIN_WINDOW__

#include "video.h"
#include "render.h"

#include "render/window.h"

//============================================================
//  PARAMETERS
//============================================================


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct win_draw_callbacks
{
	void (*exit)(void);

	int (*window_init)(render::window_info *window);
	render_primitive_list *(*window_get_primitives)(render::window_info *window);
	int (*window_draw)(render::window_info *window, HDC dc, int update);
	void (*window_save)(render::window_info *window);
	void (*window_record)(render::window_info *window);
	void (*window_toggle_fsfx)(render::window_info *window);
	void (*window_destroy)(render::window_info *window);
};


//============================================================
//  PROTOTYPES
//============================================================

// core initialization
void winwindow_init(running_machine &machine);

void winwindow_process_events_periodic(running_machine &machine);

void winwindow_ui_pause_from_window_thread(running_machine &machine, int pause);
int winwindow_ui_is_paused(running_machine &machine);

void winwindow_ui_exec_on_main_thread(void (*func)(void *), void *param);
void winwindow_dispatch_message(running_machine &machine, MSG *message);

extern int win_create_menu(running_machine &machine, HMENU *menus);


//============================================================
//  rect_width / rect_height
//============================================================

INLINE int rect_width(const RECT *rect)
{
	return rect->right - rect->left;
}


INLINE int rect_height(const RECT *rect)
{
	return rect->bottom - rect->top;
}

#endif
