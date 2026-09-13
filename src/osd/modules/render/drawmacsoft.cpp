// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  drawmacsoft.cpp - software renderer for the Mac OSD
//
//  Mac OSD by R. Belmont
//
//============================================================

#include "render_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_MAC)

#include "window.h"

#include "emu.h"
#include "rendersw.hxx"

#include <memory>

// implemented in windowcontroller.mm
extern void MacBlitVideoFrame(void *wincontroller, const void *bits, int width, int height, int pitch);


namespace osd {

namespace {

class renderer_mac_soft : public osd_renderer
{
public:
	renderer_mac_soft(osd_window &window)
		: osd_renderer(window)
		, m_bmsize(0)
	{
	}

	virtual int create() override
	{
		return 0;
	}

	virtual render_primitive_list *get_primitives() override
	{
		const osd_dim dimensions = window().get_size_pixels();
		if ((dimensions.width() <= 0) || (dimensions.height() <= 0))
		{
			return nullptr;
		}

		window().target()->set_bounds(dimensions.width(), dimensions.height(), window().pixel_aspect());
		return &window().target()->get_primitives();
	}

	virtual int draw(const int update) override;

private:
	std::unique_ptr<uint8_t []> m_bmdata;
	size_t                      m_bmsize;
};

//============================================================
//  renderer_mac_soft::draw
//============================================================

int renderer_mac_soft::draw(const int update)
{
	auto &win = dynamic_cast<mac_window_info &>(window());

	// compute width/height/pitch of target
	const osd_dim dimensions = win.get_size_pixels();
	const int width = dimensions.width();
	const int height = dimensions.height();
	if ((width <= 0) || (height <= 0))
	{
		return 0;
	}

	const int pitch = (width + 3) & ~3;

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

	// blit to the window
	MacBlitVideoFrame(win.platform_window(), m_bmdata.get(), width, height, pitch);

	return 0;
}


//============================================================
//  video_mac_soft - software render module
//============================================================

class video_mac_soft : public osd_module, public render_module
{
public:
	video_mac_soft() : osd_module(OSD_RENDERER_PROVIDER, "soft") { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) override
	{
		return std::make_unique<renderer_mac_soft>(window);
	}

protected:
	virtual unsigned flags() const override { return FLAG_INTERACTIVE; }
};

} // anonymous namespace

} // namespace osd


#else // defined(OSD_MAC)

namespace osd { namespace { MODULE_NOT_SUPPORTED(video_mac_soft, OSD_RENDERER_PROVIDER, "soft") } }

#endif // defined(OSD_MAC)


MODULE_DEFINITION(RENDERER_MACSOFT, osd::video_mac_soft)
