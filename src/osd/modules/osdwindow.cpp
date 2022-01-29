// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.cpp - SDL window handling
//
//============================================================

#include "emu.h"
#include "osdwindow.h"

#include "render/drawnone.h"
#include "render/drawbgfx.h"
#if (USE_OPENGL)
#include "render/drawogl.h"
#endif
#if defined(OSD_WINDOWS)
#include "render/drawgdi.h"
#include "render/drawd3d.h"
#elif defined(OSD_SDL)
#include "render/draw13.h"
#include "render/drawsdl.h"
#endif

osd_window::osd_window(running_machine &machine, int index, std::shared_ptr<osd_monitor_info> monitor, const osd_window_config &config) :
#ifdef OSD_WINDOWS
	m_dc(nullptr), m_resize_state(0),
#endif
	m_target(nullptr),
	m_primlist(nullptr),
	m_win_config(config),
	m_index(index),
	m_fullscreen(false),
	m_prescale(1),
	m_machine(machine),
	m_monitor(std::move(monitor)),
	m_renderer(nullptr),
	m_main(nullptr),
	m_title(
			util::string_format(
				(video_config.numscreens > 1)
					? "%3$s [%4$s] screen %5$d - %1$s %2$s (%6$s%7$sP%8$d)"
					: "%3$s [%4$s] - %1$s %2$s (%6$s%7$sP%8$d)",
					emulator_info::get_appname(),
					emulator_info::get_bare_build_version(),
					machine.system().type.fullname(),
					machine.system().name,
					index,
					(sizeof(int) == sizeof(void *)) ? "I" : "",
					(sizeof(long) == sizeof(void *)) ? "L" : (sizeof(long long) == sizeof(void *)) ? "LL" : "",
					sizeof(void *) * 8))
{
}

float osd_window::pixel_aspect() const
{
	return monitor()->pixel_aspect();
}

std::unique_ptr<osd_renderer> osd_renderer::make_for_type(int mode, std::shared_ptr<osd_window> window, int extra_flags)
{
	switch(mode)
	{
#if defined(OSD_WINDOWS)
		case VIDEO_MODE_NONE:
			return std::make_unique<renderer_none>(window);
#endif
		case VIDEO_MODE_BGFX:
			return std::make_unique<renderer_bgfx>(window);
#if (USE_OPENGL)
		case VIDEO_MODE_OPENGL:
			return std::make_unique<renderer_ogl>(window);
#endif
#if defined(OSD_WINDOWS)
		case VIDEO_MODE_GDI:
			return std::make_unique<renderer_gdi>(window);
		case VIDEO_MODE_D3D:
			return std::make_unique<renderer_d3d9>(window);
#elif defined(OSD_SDL)
		case VIDEO_MODE_SDL2ACCEL:
			return std::make_unique<renderer_sdl2>(window, extra_flags);
		case VIDEO_MODE_SOFT:
			return std::make_unique<renderer_sdl1>(window, extra_flags);
#endif
		default:
			return nullptr;
	}
}

std::shared_ptr<osd_monitor_info> osd_window::monitor_from_rect(const osd_rect *proposed) const
{
	std::shared_ptr<osd_monitor_info> monitor;

	if (fullscreen() || !m_monitor) // in full screen, just use the configured monitor
		monitor = m_monitor;
	else if (proposed) // in window mode, find the nearest
		monitor = m_monitor->module().monitor_from_rect(*proposed);
	else
		monitor = m_monitor->module().monitor_from_window(*this);

	return monitor;
}

void osd_window::create_target()
{
	// add us to the list
	osd_common_t::s_window_list.push_back(shared_from_this());

	// load the layout
	m_target = m_machine.render().target_alloc();

	// set the specific view
	osd_options &options = downcast<osd_options &>(m_machine.options());
	set_starting_view(m_index, options.view(), options.view(m_index));
}

void osd_window::set_starting_view(int index, const char *defview, const char *view)
{
	// choose non-auto over auto
	if ((!*view || !strcmp(view, "auto")) && (*defview && strcmp(defview, "auto")))
		view = defview;

	// query the video system to help us pick a view
	int viewindex = m_target->configured_view(view, index, video_config.numscreens);

	// set the view
	m_target->set_view(viewindex);
}

void osd_window::destroy()
{
	// remove us from the list
	osd_common_t::s_window_list.remove(shared_from_this());

	// free the textures etc
	complete_destroy();

	// free the render target, after the textures!
	m_machine.render().target_free(m_target);
	m_target = nullptr;
}
