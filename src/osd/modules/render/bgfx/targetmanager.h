// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetmanager.h - BGFX render target manager
//
//  Maintains a per-screen  string-to-entry mapping for any
//  registered render targets.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TARGET_MANAGER__
#define __DRAWBGFX_TARGET_MANAGER__

#include <map>
#include <string>

#include <bgfx/bgfx.h>

#include "modules/osdhelper.h"

#include "texturemanager.h"

class bgfx_target;

class target_manager {
public:
	target_manager(texture_manager& textures);
	~target_manager();

	bgfx_target* create_target(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, uint32_t style, bool double_buffer, bool filter, uint16_t scale, uint32_t screen);
	bgfx_target* create_backbuffer(void *handle, uint16_t width, uint16_t height);

	void update_target_sizes(uint32_t screen, uint16_t width, uint16_t height, uint32_t style);
	void update_screen_count(uint32_t count);

	// Getters
	bgfx_target* target(uint32_t screen, std::string name);

private:
	void rebuild_targets(uint32_t screen, uint32_t style);
	void create_target_if_nonexistent(uint32_t screen, std::string name, bool double_buffered, bool filter, uint32_t style);

	std::map<std::string, bgfx_target*> m_targets;
	texture_manager& m_textures;

	std::vector<osd_dim> m_guest_dims;
	std::vector<osd_dim> m_native_dims;
	uint32_t m_screen_count;

	static const int32_t MAX_SCREENS;
};

#endif // __DRAWBGFX_TARGET_MANAGER__
