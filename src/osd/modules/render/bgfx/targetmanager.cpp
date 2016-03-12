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

bgfx_target* target_manager::create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, bool filter)
{
	bgfx_target* target = new bgfx_target(name, format, width, height, filter);
	m_targets[name] = target;

	m_textures.add_texture(name, target);
	return target;
}

bgfx_target* target_manager::create_target(std::string name, void *handle, uint32_t width, uint32_t height)
{
	bgfx_target* target = new bgfx_target(name, handle, width, height);
	m_targets[name] = target;

	m_textures.add_texture(name, target);
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
