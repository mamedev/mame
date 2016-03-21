// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawgdi.c - Win32 GDI drawing
//
//============================================================

#include "drawgdi.h"
#include "rendersw.inc"

//============================================================
//  destructor
//============================================================

renderer_gdi::~renderer_gdi()
{
	// free the bitmap memory
	if (m_bmdata != nullptr)
		global_free_array(m_bmdata);
}

//============================================================
//  renderer_gdi::create
//============================================================

int renderer_gdi::create()
{
	// fill in the bitmap info header
	m_bminfo.bmiHeader.biSize            = sizeof(m_bminfo.bmiHeader);
	m_bminfo.bmiHeader.biPlanes          = 1;
	m_bminfo.bmiHeader.biBitCount        = 32;
	m_bminfo.bmiHeader.biCompression     = BI_RGB;
	m_bminfo.bmiHeader.biSizeImage       = 0;
	m_bminfo.bmiHeader.biXPelsPerMeter   = 0;
	m_bminfo.bmiHeader.biYPelsPerMeter   = 0;
	m_bminfo.bmiHeader.biClrUsed         = 0;
	m_bminfo.bmiHeader.biClrImportant    = 0;
	return 0;
}

//============================================================
//  renderer_gdi::get_primitives
//============================================================

render_primitive_list *renderer_gdi::get_primitives()
{
	RECT client;
	GetClientRect(window().m_hwnd, &client);
	window().target()->set_bounds(rect_width(&client), rect_height(&client), window().pixel_aspect());
	return &window().target()->get_primitives();
}

//============================================================
//  renderer_gdi::draw
//============================================================

int renderer_gdi::draw(const int update)
{
	// we don't have any special resize behaviors
	if (window().m_resize_state == RESIZE_STATE_PENDING)
		window().m_resize_state = RESIZE_STATE_NORMAL;

	// get the target bounds
	RECT bounds;
	GetClientRect(window().m_hwnd, &bounds);

	// compute width/height/pitch of target
	int width = rect_width(&bounds);
	int height = rect_height(&bounds);
	int pitch = (width + 3) & ~3;

	// make sure our temporary bitmap is big enough
	if (pitch * height * 4 > m_bmsize)
	{
		m_bmsize = pitch * height * 4 * 2;
		global_free_array(m_bmdata);
		m_bmdata = global_alloc_array(UINT8, m_bmsize);
	}

	// draw the primitives to the bitmap
	window().m_primlist->acquire_lock();
	software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window().m_primlist, m_bmdata, width, height, pitch);
	window().m_primlist->release_lock();

	// fill in bitmap-specific info
	m_bminfo.bmiHeader.biWidth = pitch;
	m_bminfo.bmiHeader.biHeight = -height;

	// blit to the screen
	StretchDIBits(window().m_dc, 0, 0, width, height,
				0, 0, width, height,
				m_bmdata, &m_bminfo, DIB_RGB_COLORS, SRCCOPY);
	return 0;
}
