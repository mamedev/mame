// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.cpp - SDL window handling
//
//============================================================

#include "emu.h"
#include "osdwindow.h"

// osd/modules
#include "lib/osdobj_common.h"
#include "monitor/monitor_module.h"
#include "render/render_module.h"

// emu
#include "main.h"
#include "render.h"


osd_window::osd_window(
		running_machine &machine,
		render_module &renderprovider,
		int index,
		const std::shared_ptr<osd_monitor_info> &monitor,
		const osd_window_config &config) :
	m_target(nullptr),
	m_primlist(nullptr),
	m_win_config(config),
	m_index(index),
	m_fullscreen(false),
	m_prescale(1),
	m_machine(machine),
	m_renderprovider(renderprovider),
	m_monitor(std::move(monitor)),
	m_renderer(nullptr),
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

osd_window::~osd_window()
{
}

float osd_window::pixel_aspect() const
{
	return monitor()->pixel_aspect();
}

bool osd_window::swap_xy() const
{
	bool orientation_swap_xy =
		(machine().system().flags & ORIENTATION_SWAP_XY) == ORIENTATION_SWAP_XY;
	bool rotation_swap_xy =
		(target()->orientation() & ORIENTATION_SWAP_XY) == ORIENTATION_SWAP_XY;
	return orientation_swap_xy ^ rotation_swap_xy;
};

bool osd_window::keepaspect() const
{
	if (m_target != nullptr)
		return m_target->keepaspect();
	else
		return false;
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
	// load the layout
	m_target = m_machine.render().target_alloc();

	// set the specific view
	osd_options &options = downcast<osd_options &>(m_machine.options());
	set_starting_view(m_index, options.view(), options.view(m_index));
}

bool osd_window::renderer_interactive() const
{
	return m_renderprovider.is_interactive();
}

bool osd_window::renderer_sdl_needs_opengl() const
{
	return m_renderprovider.sdl_needs_opengl();
}

void osd_window::renderer_create()
{
	m_renderer = m_renderprovider.create(*this);
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
	// free the textures etc
	complete_destroy();

	// free the render target, after the textures!
	m_machine.render().target_free(m_target);
	m_target = nullptr;
}
