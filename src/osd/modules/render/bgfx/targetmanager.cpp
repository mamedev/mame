// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetmanager.cpp - BGFX render target manager
//
//  Maintains a per-screen string-to-entry mapping for any
//  registered render targets.
//
//============================================================

#include <bgfx/bgfx.h>

#include <vector>

#include "modules/lib/osdobj_common.h"

#include "targetmanager.h"

#include "target.h"

const int32_t target_manager::MAX_SCREENS = 100;

target_manager::target_manager(texture_manager& textures)
	: m_textures(textures)
	, m_screen_count(0)
{
}

target_manager::~target_manager()
{
	std::vector<bgfx_target*> to_delete;
	for (std::pair<std::string, bgfx_target*> target : m_targets)
	{
		if (target.second != nullptr)
		{
			to_delete.push_back(target.second);
		}
	}
	m_targets.clear();

	for (uint32_t i = 0; i < to_delete.size(); i++)
	{
		delete to_delete[i];
	}
}

bgfx_target* target_manager::create_target(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, uint32_t style, bool double_buffer, bool filter, uint16_t scale, uint32_t screen)
{
	bgfx_target* target = new bgfx_target(name, format, width, height, style, double_buffer, filter, scale, screen);

	m_targets[name + std::to_string(screen)] = target;

	m_textures.add_provider(name + std::to_string(screen), target);

	return target;
}

bgfx_target* target_manager::create_backbuffer(void *handle, uint16_t width, uint16_t height)
{
	bgfx_target* target = new bgfx_target(handle, width, height);
	m_targets["backbuffer"] = target;
	return target;
}

bgfx_target* target_manager::target(uint32_t screen, std::string name)
{
	std::string full_name = name + std::to_string(screen);
	bgfx_target* target = m_targets[full_name];
	if (target == nullptr)
	{
		osd_printf_verbose("Warning: Attempting to retrieve a nonexistent target '%s' for screen %d\n", name.c_str(), screen);
	}
	return target;
}

void target_manager::update_target_sizes(uint32_t screen, uint16_t width, uint16_t height, uint32_t style)
{
	if (style == TARGET_STYLE_CUSTOM) return;

	std::vector<osd_dim>& sizes = style == TARGET_STYLE_GUEST ? m_guest_dims : m_native_dims;

	// Ensure that there's an entry to fill
	while (sizes.size() <= screen)
	{
		sizes.push_back(osd_dim(0, 0));
	}

	if (width != sizes[screen].width() || height != sizes[screen].height())
	{
		sizes[screen] = osd_dim(width, height);
		rebuild_targets(screen, style);
	}
}

void target_manager::rebuild_targets(uint32_t screen, uint32_t style)
{
	if (style == TARGET_STYLE_CUSTOM) return;

	std::vector<bgfx_target*> to_resize;
	for (std::pair<std::string, bgfx_target*> target_pair : m_targets)
	{
		bgfx_target* target = target_pair.second;
		if (target == nullptr || target->style() != style || target->screen_index() != screen)
		{
			continue;
		}
		to_resize.push_back(target);
	}

	std::vector<osd_dim>& sizes = style == TARGET_STYLE_GUEST ? m_guest_dims : m_native_dims;
	for (bgfx_target* target : to_resize)
	{
		std::string name = target->name();
		const bgfx::TextureFormat::Enum format = target->format();
		const bool double_buffered = target->double_buffered();
		const bool filter = target->filter();
		const uint16_t scale = target->scale();
		const uint16_t width(sizes[screen].width());
		const uint16_t height(sizes[screen].height());
		delete target;

		create_target(name, format, width, height, style, double_buffered, filter, scale, screen);
	}
}

void target_manager::update_screen_count(uint32_t count)
{
	// Ensure that there's an entry to fill
	while (count > m_native_dims.size())
	{
		m_native_dims.push_back(osd_dim(0, 0));
	}

	if (count != m_screen_count)
	{
		uint32_t old_count = m_screen_count;
		m_screen_count = count;
		if (m_screen_count > old_count)
		{
			for (uint32_t screen = old_count; screen < m_screen_count; screen++)
			{
				create_target_if_nonexistent(screen, "output", false, false, TARGET_STYLE_NATIVE);
			}
		}
	}
}

void target_manager::create_target_if_nonexistent(uint32_t screen, std::string name, bool double_buffered, bool filter, uint32_t style)
{
	if (style == TARGET_STYLE_CUSTOM) return;

	if (m_targets[name + std::to_string(screen)] != nullptr)
	{
		return;
	}

	std::vector<osd_dim>& sizes = style == TARGET_STYLE_GUEST ? m_guest_dims : m_native_dims;
	uint16_t width(sizes[screen].width());
	uint16_t height(sizes[screen].height());

	create_target(name, bgfx::TextureFormat::RGBA8, width, height, style, double_buffered, filter, 1, screen);
}
