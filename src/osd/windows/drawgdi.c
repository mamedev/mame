//============================================================
//
//  drawgdi.c - Win32 GDI drawing
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "mamecore.h"

// MAMEOS headers
#include "window.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

/* gdi_info is the information for the current screen */
typedef struct _gdi_info gdi_info;
struct _gdi_info
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
static const render_primitive_list *drawgdi_window_get_primitives(win_window_info *window);
static int drawgdi_window_draw(win_window_info *window, HDC dc, int update);

// rendering
static void drawgdi_rgb888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);



//============================================================
//  drawgdi_init
//============================================================

int drawgdi_init(win_draw_callbacks *callbacks)
{
	// fill in the callbacks
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
	gdi_info *gdi;
	int i;

	// allocate memory for our structures
	gdi = malloc_or_die(sizeof(*gdi));
	memset(gdi, 0, sizeof(*gdi));
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
	gdi_info *gdi = window->drawdata;

	// skip if nothing
	if (gdi == NULL)
		return;

	// free the bitmap memory
	if (gdi->bmdata != NULL)
		free(gdi->bmdata);
	free(gdi);
	window->drawdata = NULL;
}



//============================================================
//  drawgdi_window_get_primitives
//============================================================

static const render_primitive_list *drawgdi_window_get_primitives(win_window_info *window)
{
	RECT client;
	GetClientRect(window->hwnd, &client);
	render_target_set_bounds(window->target, rect_width(&client), rect_height(&client), winvideo_monitor_get_aspect(window->monitor));
	return render_target_get_primitives(window->target);
}



//============================================================
//  drawgdi_window_draw
//============================================================

static int drawgdi_window_draw(win_window_info *window, HDC dc, int update)
{
	gdi_info *gdi = window->drawdata;
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
		gdi->bmdata = realloc(gdi->bmdata, gdi->bmsize);
	}

	// draw the primitives to the bitmap
	osd_lock_acquire(window->primlist->lock);
	drawgdi_rgb888_draw_primitives(window->primlist->head, gdi->bmdata, width, height, pitch);
	osd_lock_release(window->primlist->lock);

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



//============================================================
//  SOFTWARE RENDERING
//============================================================

#define FUNC_PREFIX(x)		drawgdi_rgb888_##x
#define PIXEL_TYPE			UINT32
#define SRCSHIFT_R			0
#define SRCSHIFT_G			0
#define SRCSHIFT_B			0
#define DSTSHIFT_R			16
#define DSTSHIFT_G			8
#define DSTSHIFT_B			0

#include "rendersw.c"
