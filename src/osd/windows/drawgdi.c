// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawgdi.c - Win32 GDI drawing
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "emu.h"
#include "rendersw.inc"

// MAMEOS headers
#include "window.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

/* gdi_info is the information for the current screen */
struct gdi_info
{
	BITMAPINFO              bminfo;
	UINT8 *                 bmdata;
	size_t                  bmsize;
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
	memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawgdi_exit;
	callbacks->window_init = drawgdi_window_init;
	callbacks->window_get_primitives = drawgdi_window_get_primitives;
	callbacks->window_draw = drawgdi_window_draw;
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
	// allocate memory for our structures
	gdi_info *gdi = global_alloc_clear(gdi_info);
	window->drawdata = gdi;

	// fill in the bitmap info header
	gdi->bminfo.bmiHeader.biSize            = sizeof(gdi->bminfo.bmiHeader);
	gdi->bminfo.bmiHeader.biPlanes          = 1;
	gdi->bminfo.bmiHeader.biBitCount        = 32;
	gdi->bminfo.bmiHeader.biCompression     = BI_RGB;
	gdi->bminfo.bmiHeader.biSizeImage       = 0;
	gdi->bminfo.bmiHeader.biXPelsPerMeter   = 0;
	gdi->bminfo.bmiHeader.biYPelsPerMeter   = 0;
	gdi->bminfo.bmiHeader.biClrUsed         = 0;
	gdi->bminfo.bmiHeader.biClrImportant    = 0;

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
		global_free_array(gdi->bmdata);
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
		global_free_array(gdi->bmdata);
		gdi->bmdata = global_alloc_array(UINT8, gdi->bmsize);
	}

	// draw the primitives to the bitmap
	window->primlist->acquire_lock();
	software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window->primlist, gdi->bmdata, width, height, pitch);
	window->primlist->release_lock();

	// fill in bitmap-specific info
	gdi->bminfo.bmiHeader.biWidth = pitch;
	gdi->bminfo.bmiHeader.biHeight = -height;

	// blit to the screen
	StretchDIBits(dc, 0, 0, width, height,
				0, 0, width, height,
				gdi->bmdata, &gdi->bminfo, DIB_RGB_COLORS, SRCCOPY);
	return 0;
}
