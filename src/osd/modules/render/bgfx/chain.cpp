// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chain.cpp - BGFX screen-space post-effect chain
//
//============================================================

#include "emu.h"

#include <bx/timer.h>

#include "slider.h"
#include "parameter.h"
#include "entryuniform.h"
#include "texturemanager.h"
#include "targetmanager.h"
#include "target.h"
#include "vertex.h"
#include "screen.h"
#include "modules/osdwindow.h"

#include "chain.h"

bgfx_chain::bgfx_chain(std::string name, std::string author, bool transform, target_manager& targets, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries, std::vector<bgfx_target*> target_list, std::uint32_t screen_index)
	: m_name(name)
	, m_author(author)
	, m_transform(transform)
	, m_targets(targets)
	, m_sliders(sliders)
	, m_params(params)
	, m_entries(entries)
	, m_target_list(target_list)
	, m_current_time(0)
	, m_screen_index(screen_index)
{
	for (bgfx_slider* slider : m_sliders)
	{
		m_slider_map[slider->name()] = slider;
	}
}

bgfx_chain::~bgfx_chain()
{
	for (bgfx_slider* slider : m_sliders)
	{
		delete slider;
	}
	for (bgfx_parameter* param : m_params)
	{
		delete param;
	}
	for (bgfx_chain_entry* entry : m_entries)
	{
		delete entry;
	}
	for (bgfx_target* target : m_target_list)
	{
		m_targets.destroy_target(target->name(), m_screen_index);
	}
}

void bgfx_chain::process(render_primitive* prim, int view, int screen, texture_manager& textures, osd_window& window, uint64_t blend)
{
	screen_device_iterator screen_iterator(window.machine().root_device());
	screen_device* screen_device = screen_iterator.byindex(screen);
	render_container &screen_container = screen_device->container();

	int current_view = view;
	uint16_t screen_width(floor((prim->bounds.x1 - prim->bounds.x0) + 0.5f));
	uint16_t screen_height(floor((prim->bounds.y1 - prim->bounds.y0) + 0.5f));
	uint32_t rotation_type =
		(window.target()->orientation() & ROT90)  == ROT90  ? 1 :
		(window.target()->orientation() & ROT180) == ROT180 ? 2 :
		(window.target()->orientation() & ROT270) == ROT270 ? 3 : 0;
	bool orientation_swap_xy = (window.machine().system().flags & ORIENTATION_SWAP_XY) == ORIENTATION_SWAP_XY;
	bool rotation_swap_xy = (window.target()->orientation() & ORIENTATION_SWAP_XY) == ORIENTATION_SWAP_XY;
	bool swap_xy = orientation_swap_xy ^ rotation_swap_xy;
	float screen_scale_x = 1.0f / screen_container.xscale();
	float screen_scale_y = 1.0f / screen_container.yscale();
	float screen_offset_x = -screen_container.xoffset();
	float screen_offset_y = -screen_container.yoffset();

	for (bgfx_chain_entry* entry : m_entries)
	{
		if (!entry->skip())
		{
			entry->submit(current_view, prim, textures, screen_width, screen_height, screen_scale_x, screen_scale_y, screen_offset_x, screen_offset_y, rotation_type, swap_xy, blend, screen);
			current_view++;
		}
	}

	m_current_time = bx::getHPCounter();
	static int64_t last = m_current_time;
	const int64_t frameTime = m_current_time - last;
	last = m_current_time;
	const double freq = double(bx::getHPFrequency());
	const double toMs = 1000.0 / freq;
	const double frameTimeInSeconds = (double)frameTime / 1000000.0;

	for (bgfx_parameter* param : m_params)
	{
		param->tick(frameTimeInSeconds* toMs);
	}
}

uint32_t bgfx_chain::applicable_passes()
{
	int applicable_passes = 0;
	for (bgfx_chain_entry* entry : m_entries)
	{
		if (!entry->skip())
		{
			applicable_passes++;
		}
	}

	return applicable_passes;
}
