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

bgfx_target* target_manager::create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, uint32_t style, bool double_buffer, bool filter)
{
	bgfx_target* target = new bgfx_target(name, format, width, height, style, double_buffer, filter);
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
			m_targets[target->name()] = new bgfx_target(target->name(), target->format(), width, height, TARGET_STYLE_GUEST, target->double_buffered(), target->filter());
			delete target;
		}
	}
}
