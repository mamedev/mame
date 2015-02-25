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

class renderer_gdi : public osd_renderer
{
public:
	renderer_gdi(osd_window *window)
	: osd_renderer(window, FLAG_NONE), bmdata(NULL), bmsize(0) { }

	virtual ~renderer_gdi() { }

	virtual int create();
	virtual render_primitive_list *get_primitives();
	virtual int draw(const int update);
	virtual void save() {};
	virtual void record() {};
	virtual void toggle_fsfx() {};
	virtual void destroy();

private:
	/* gdi_info is the information for the current screen */
	BITMAPINFO              bminfo;
	UINT8 *                 bmdata;
	size_t                  bmsize;
};


//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawgdi_exit(void);

//============================================================
//  drawnone_create
//============================================================

static osd_renderer *drawgdi_create(osd_window *window)
{
	return global_alloc(renderer_gdi(window));
}


//============================================================
//  drawgdi_init
//============================================================

int drawgdi_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawgdi_exit;
	callbacks->create = drawgdi_create;
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

int renderer_gdi::create()
{
	// fill in the bitmap info header
	bminfo.bmiHeader.biSize            = sizeof(bminfo.bmiHeader);
	bminfo.bmiHeader.biPlanes          = 1;
	bminfo.bmiHeader.biBitCount        = 32;
	bminfo.bmiHeader.biCompression     = BI_RGB;
	bminfo.bmiHeader.biSizeImage       = 0;
	bminfo.bmiHeader.biXPelsPerMeter   = 0;
	bminfo.bmiHeader.biYPelsPerMeter   = 0;
	bminfo.bmiHeader.biClrUsed         = 0;
	bminfo.bmiHeader.biClrImportant    = 0;

	return 0;
}



//============================================================
//  drawgdi_window_destroy
//============================================================

void renderer_gdi::destroy()
{
	// free the bitmap memory
	if (bmdata != NULL)
		global_free_array(bmdata);
}



//============================================================
//  drawgdi_window_get_primitives
//============================================================

render_primitive_list *renderer_gdi::get_primitives()
{
	RECT client;
	GetClientRect(window().m_hwnd, &client);
	window().target()->set_bounds(rect_width(&client), rect_height(&client), window().aspect());
	return &window().target()->get_primitives();
}



//============================================================
//  drawgdi_window_draw
//============================================================

int renderer_gdi::draw(const int update)
{
	int width, height, pitch;
	RECT bounds;

	// we don't have any special resize behaviors
	if (window().m_resize_state == RESIZE_STATE_PENDING)
		window().m_resize_state = RESIZE_STATE_NORMAL;

	// get the target bounds
	GetClientRect(window().m_hwnd, &bounds);

	// compute width/height/pitch of target
	width = rect_width(&bounds);
	height = rect_height(&bounds);
	pitch = (width + 3) & ~3;

	// make sure our temporary bitmap is big enough
	if (pitch * height * 4 > bmsize)
	{
		bmsize = pitch * height * 4 * 2;
		global_free_array(bmdata);
		bmdata = global_alloc_array(UINT8, bmsize);
	}

	// draw the primitives to the bitmap
	window().m_primlist->acquire_lock();
	software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window().m_primlist, bmdata, width, height, pitch);
	window().m_primlist->release_lock();

	// fill in bitmap-specific info
	bminfo.bmiHeader.biWidth = pitch;
	bminfo.bmiHeader.biHeight = -height;

	// blit to the screen
	StretchDIBits(window().m_dc, 0, 0, width, height,
				0, 0, width, height,
				bmdata, &bminfo, DIB_RGB_COLORS, SRCCOPY);
	return 0;
}
