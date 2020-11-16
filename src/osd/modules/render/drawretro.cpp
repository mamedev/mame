
//============================================================
//
//  drawretro.c - basic retro drawing
//
//============================================================

#include "drawretro.h"
#include "rendersw.hxx"

//============================================================
//  destructor
//============================================================

renderer_retro::~renderer_retro()
{
	// free the bitmap memory
	delete m_bmdata;
}

//============================================================
//  renderer_gdi::create
//============================================================

int renderer_retro::create()
{

	return 0;
}

//============================================================
//  renderer_gdi::get_primitives
//============================================================

render_primitive_list *renderer_retro::get_primitives()
{
	auto win = try_getwindow();
	if (win == nullptr)
		return nullptr;

	osd_dim nd = win->get_size();

	win->target()->set_bounds(nd.width(), nd.height(), win->pixel_aspect());
	return &win->target()->get_primitives();
}

int renderer_retro::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	auto win = assert_window();

	osd_dim nd = win->get_size();

	*xt = x;
	*yt = y;
	if (*xt<0 || *xt >= nd.width())
		return 0;
	if (*yt<0 || *yt >= nd.height())
		return 0;
	return 1;
}

//============================================================
//  renderer_gdi::draw
//============================================================

int renderer_retro::draw(const int update)
{
	auto win = assert_window();

	// get the target bounds
	osd_dim nd = win->get_size();

	// compute width/height/pitch of target
	int width = nd.width();
	int height = nd.height();
/*
	int pitch = (width + 3) & ~3;

	// make sure our temporary bitmap is big enough
	if (pitch * height * 4 > m_bmsize)
	{
		m_bmsize = pitch * height * 4 * 2;
		global_free_array(m_bmdata);
		m_bmdata = global_alloc_array(uint8_t, m_bmsize);
	}

*/
        m_bmdata = (uint8_t *)retro_get_fb_ptr();

	//FIXME retro handle 16/32 bits

	// draw the primitives to the bitmap
	win->m_primlist->acquire_lock();
	software_renderer<uint32_t, 0,0,0, 16,8,0>::draw_primitives(*win->m_primlist, m_bmdata, width, height, width/*pitch*/);
	win->m_primlist->release_lock();

	return 0;
}
