//============================================================
//
//  drawgdi.c - Win32 GDI drawing
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
#include "rendersw.c"

// MAMEOS headers
#include "window.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

/* gdi_info is the information for the current screen */
struct gdi_info
{
	BITMAPINFO				bminfo;
	RGBQUAD					colors[256];
	UINT8 *					bmdata;
	size_t					bmsize;
};



//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawgdi_exit(void);
static int drawgdi_window_init(win_window_info *window);
static void drawgdi_window_destroy(win_window_info *window);
static render_primitive_list *drawgdi_window_get_primitives(win_window_info *window);
static int drawgdi_window_draw(win_window_info *window, HDC dc, int update);



//============================================================
//  drawgdi_init
//============================================================

int drawgdi_init(running_machine &machine, win_draw_callbacks *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawgdi_exit;
	callbacks->window_init = drawgdi_window_init;
	callbacks->window_get_primitives = drawgdi_window_get_primitives;
	callbacks->window_draw = drawgdi_window_draw;
	callbacks->window_save = NULL;
	callbacks->window_record = NULL;
	callbacks->window_destroy = drawgdi_window_destroy;
	return 0;
}



//============================================================
//  drawgdi_exit
//============================================================

static void drawgdi_exit(void)
{
}



//============================================================
//  drawgdi_window_init
//============================================================

static int drawgdi_window_init(win_window_info *window)
{
	gdi_info *gdi;
	int i;

	// allocate memory for our structures
	gdi = global_alloc_clear(gdi_info);
	window->drawdata = gdi;

	// fill in the bitmap info header
	gdi->bminfo.bmiHeader.biSize			= sizeof(gdi->bminfo.bmiHeader);
	gdi->bminfo.bmiHeader.biPlanes			= 1;
	gdi->bminfo.bmiHeader.biCompression		= BI_RGB;
	gdi->bminfo.bmiHeader.biSizeImage		= 0;
	gdi->bminfo.bmiHeader.biXPelsPerMeter	= 0;
	gdi->bminfo.bmiHeader.biYPelsPerMeter	= 0;
	gdi->bminfo.bmiHeader.biClrUsed			= 0;
	gdi->bminfo.bmiHeader.biClrImportant	= 0;

	// initialize the palette to a gray ramp
	for (i = 0; i < 256; i++)
	{
		gdi->bminfo.bmiColors[i].rgbRed			= i;
		gdi->bminfo.bmiColors[i].rgbGreen		= i;
		gdi->bminfo.bmiColors[i].rgbBlue		= i;
		gdi->bminfo.bmiColors[i].rgbReserved	= i;
	}

	return 0;
}



//============================================================
//  drawgdi_window_destroy
//============================================================

static void drawgdi_window_destroy(win_window_info *window)
{
	gdi_info *gdi = (gdi_info *)window->drawdata;

	// skip if nothing
	if (gdi == NULL)
		return;

	// free the bitmap memory
	if (gdi->bmdata != NULL)
		global_free(gdi->bmdata);
	global_free(gdi);
	window->drawdata = NULL;
}



//============================================================
//  drawgdi_window_get_primitives
//============================================================

static render_primitive_list *drawgdi_window_get_primitives(win_window_info *window)
{
	RECT client;
	GetClientRect(window->hwnd, &client);
	window->target->set_bounds(rect_width(&client), rect_height(&client), winvideo_monitor_get_aspect(window->monitor));
	return &window->target->get_primitives();
}



//============================================================
//  drawgdi_window_draw
//============================================================

static int drawgdi_window_draw(win_window_info *window, HDC dc, int update)
{
	gdi_info *gdi = (gdi_info *)window->drawdata;
	int width, height, pitch;
	RECT bounds;

	// we don't have any special resize behaviors
	if (window->resize_state == RESIZE_STATE_PENDING)
		window->resize_state = RESIZE_STATE_NORMAL;

	// get the target bounds
	GetClientRect(window->hwnd, &bounds);

	// compute width/height/pitch of target
	width = rect_width(&bounds);
	height = rect_height(&bounds);
	pitch = (width + 3) & ~3;

	// make sure our temporary bitmap is big enough
	if (pitch * height * 4 > gdi->bmsize)
	{
		gdi->bmsize = pitch * height * 4 * 2;
		global_free(gdi->bmdata);
		gdi->bmdata = global_alloc_array(UINT8, gdi->bmsize);
	}

	// draw the primitives to the bitmap
	window->primlist->acquire_lock();
	software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window->primlist, gdi->bmdata, width, height, pitch);
	window->primlist->release_lock();

	// fill in bitmap-specific info
	gdi->bminfo.bmiHeader.biWidth = pitch;
	gdi->bminfo.bmiHeader.biHeight = -height;
	gdi->bminfo.bmiHeader.biBitCount = 32;

	// blit to the screen
	StretchDIBits(dc, 0, 0, width, height,
				0, 0, width, height,
				gdi->bmdata, &gdi->bminfo, DIB_RGB_COLORS, SRCCOPY);
	return 0;
}
