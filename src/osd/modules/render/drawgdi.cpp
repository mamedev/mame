// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawgdi.cpp - Win32 GDI drawing
//
//============================================================

#include "render_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

#include "window.h"

// emu
#include "emu.h"
#include "rendersw.hxx"

// standard windows headers
#include <windows.h>


namespace osd {

namespace {

// renderer_gdi is the information for the current screen
class renderer_gdi : public osd_renderer
{
public:
	renderer_gdi(osd_window &window)
		: osd_renderer(window)
		, m_bmdata(nullptr)
		, m_bmsize(0)
	{
	}

	virtual int create() override;
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override;
	virtual void save() override {}
	virtual void record() override {}
	virtual void toggle_fsfx() override {}

private:
	BITMAPINFO                  m_bminfo;
	std::unique_ptr<uint8_t []> m_bmdata;
	size_t                      m_bmsize;
};

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
	osd_dim const dimensions = window().get_size();
	if ((dimensions.width() <= 0) || (dimensions.height() <= 0))
		return nullptr;

	window().target()->set_bounds(dimensions.width(), dimensions.height(), window().pixel_aspect());
	return &window().target()->get_primitives();
}

//============================================================
//  renderer_gdi::draw
//============================================================

int renderer_gdi::draw(const int update)
{
	auto &win = dynamic_cast<win_window_info &>(window());

	// we don't have any special resize behaviors
	if (win.m_resize_state == win_window_info::RESIZE_STATE_PENDING)
		win.m_resize_state = win_window_info::RESIZE_STATE_NORMAL;

	// get the target bounds
	RECT bounds;
	GetClientRect(win.platform_window(), &bounds);

	// compute width/height/pitch of target
	osd_dim const dimensions = win.get_size();
	int const width = dimensions.width();
	int const height = dimensions.height();
	int const pitch = (width + 3) & ~3;

	// make sure our temporary bitmap is big enough
	if ((pitch * height * 4) > m_bmsize)
	{
		m_bmsize = pitch * height * 4 * 2;
		m_bmdata.reset();
		m_bmdata = std::make_unique<uint8_t []>(m_bmsize);
	}

	// draw the primitives to the bitmap
	win.m_primlist->acquire_lock();
	software_renderer<uint32_t, 0,0,0, 16,8,0>::draw_primitives(*win.m_primlist, m_bmdata.get(), width, height, pitch);
	win.m_primlist->release_lock();

	// fill in bitmap-specific info
	m_bminfo.bmiHeader.biWidth = pitch;
	m_bminfo.bmiHeader.biHeight = -height;

	// blit to the screen
	StretchDIBits(
			win.m_dc, 0, 0, width, height,
			0, 0, width, height,
			m_bmdata.get(), &m_bminfo, DIB_RGB_COLORS, SRCCOPY);

	return 0;
}


class video_gdi : public osd_module, public render_module
{
public:
	video_gdi() : osd_module(OSD_RENDERER_PROVIDER, "gdi") { }

	virtual int init(osd_interface &osd, osd_options const &options) override { return 0; }
	virtual void exit() override { }

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) override;

protected:
	virtual unsigned flags() const override { return FLAG_INTERACTIVE; }
};

std::unique_ptr<osd_renderer> video_gdi::create(osd_window &window)
{
	return std::make_unique<renderer_gdi>(window);
}

} // anonymous namespace

} // namespace osd


#else // defined(OSD_WINDOWS)

namespace osd { namespace { MODULE_NOT_SUPPORTED(video_gdi, OSD_RENDERER_PROVIDER, "gdi") } }

#endif // defined(OSD_WINDOWS)


MODULE_DEFINITION(RENDERER_GDI, osd::video_gdi)
