// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetmanager.cpp - BGFX render target manager
//
//  Maintains a string-to-entry mapping for any registered
//  render targets.
//
//============================================================

#include <bgfx/bgfx.h>

#include <vector>

#include "modules/lib/osdobj_common.h"

#include "targetmanager.h"

#include "target.h"

target_manager::~target_manager()
{
	for (std::pair<std::string, bgfx_target*> target : m_targets)
	{
		delete target.second;
	}
	m_targets.clear();
}

bgfx_target* target_manager::create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, uint32_t prescale_x, uint32_t prescale_y, uint32_t style, bool double_buffer, bool filter, bool output)
{
	bgfx_target* target = new bgfx_target(name, format, width, height, prescale_x, prescale_y, style, double_buffer, filter, width > 0 && height > 0, output);
	m_targets[name] = target;

	m_textures.add_provider(name, target);
	return target;
}

bgfx_target* target_manager::create_backbuffer(void *handle, uint32_t width, uint32_t height)
{
	bgfx_target* target = new bgfx_target(handle, width, height);
	m_targets["backbuffer"] = target;
	return target;
}

bgfx_target* target_manager::target(std::string name)
{
	std::map<std::string, bgfx_target*>::iterator iter = m_targets.find(name);
	if (iter != m_targets.end())
	{
		return iter->second;
	}

	return nullptr;
}

void target_manager::update_guest_targets(uint16_t width, uint16_t height)
{
	if (width != m_guest_width || height != m_guest_height)
	{
		m_guest_width = width;
		m_guest_height = height;

		std::vector<bgfx_target*> to_resize;
		for (std::pair<std::string, bgfx_target*> target : m_targets)
		{
			if ((target.second)->style() == TARGET_STYLE_GUEST)
			{
				to_resize.push_back(target.second);
			}
		}

		for (bgfx_target* target : to_resize)
		{
			std::string name = target->name();
			const bgfx::TextureFormat::Enum format = target->format();
			const bool double_buffered = target->double_buffered();
			const bool filter = target->filter();
			const uint32_t prescale_x = target->prescale_x();
			const uint32_t prescale_y = target->prescale_y();
			delete target;

			m_targets[name] = new bgfx_target(name, format, m_guest_width, m_guest_height, prescale_x, prescale_y, TARGET_STYLE_GUEST, double_buffered, filter);
			m_textures.add_provider(name, m_targets[name]);
		}
	}
}

void target_manager::update_window_count(uint32_t count)
{
    if (count != m_window_count)
    {
        m_window_count = count;
        rebuild_outputs();
    }
}

void target_manager::rebuild_outputs()
{
    for (uint32_t i = 0; i < m_window_count; i++)
    {
        rebuild_target("output" + std::to_string(i));
        rebuild_target("previous" + std::to_string(i));
    }
}

void target_manager::rebuild_target(std::string name)
{
    bgfx_target* target = m_targets[name];
    if (target != nullptr) {
        delete target;
    }
    m_targets[name] = new bgfx_target(name, bgfx::TextureFormat::RGBA8, m_guest_width, m_guest_height, m_options.bgfx_prescale_x(), m_options.bgfx_prescale_y(), TARGET_STYLE_GUEST, true, true);
    m_textures.add_provider(name, m_targets[name]);
}